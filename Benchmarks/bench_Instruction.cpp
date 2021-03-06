/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Benchmark of instructions
 */

#include <iostream>
#include <cstddef>

#include <benchmark/benchmark.h>

#include <Kube/Interpreter/Instruction.hpp>
#include <Kube/Object/Object.hpp>
#include <Kube/Core/Vector.hpp>

/* Common types used accross all benchmarks */

using Type = std::uint32_t;

class MyObject : public kF::Object
{
    K_DERIVED(MyObject, kF::Object,
        K_PROPERTY(int, value, 1)
    )
};

/* Reference
    A reference benchmark that cannot be outperformed
*/

static void BenchNative(benchmark::State &state)
{
    kF::Meta::Resolver::Clear();
    kF::RegisterMetadata();
    auto obj = std::make_unique<MyObject>();
    auto data = kF::Meta::Factory<MyObject>::Resolve().findData(kF::Hash("value"));
    kF::Var *y = new kF::Var(2);

    if ((obj->getVar(data) + *y).as<int>() != 3)
        throw std::logic_error("Native: code is broken");
    for (auto _ : state) {
        benchmark::DoNotOptimize(obj->getVar(data) + *y);
    }
}
BENCHMARK(BenchNative);

// template<int Value, int Count>
// static void BenchNative10(benchmark::State &state)
// {
//     kF::Meta::Resolver::Clear();
//     kF::RegisterMetadata();
//     kF::Var *x = new kF::Var(1);

//     if ((*x + *x + *x + *x + *x + *x + *x + *x + *x + *x).as<int>() != 10)
//         throw std::logic_error("Native advanced: code is broken");
//     for (auto _ : state) {
//         benchmark::DoNotOptimize(*x + *x + *x + *x + *x + *x + *x + *x + *x + *x);
//     }
// }
// BENCHMARK(BenchNativeAdvanced);

/* Recursive
    Naive approach to code execution using the C++ stack and recursion
*/

struct alignas(8) RecursiveAddition
{
    Type type {};
};

struct alignas(8) RecursiveConstant
{
    alignas(8) Type type {};
    kF::Var *data;
};

struct alignas(8) RecursiveGet
{
    alignas(8) Type type {};
    kF::Meta::Data data {};
    kF::Object *target {};
};

struct alignas(8) RecursiveReturn
{
    alignas(8) Type type {};
};

static_assert(sizeof(RecursiveAddition) == 8);
static_assert(sizeof(RecursiveConstant) == 16);
static_assert(sizeof(RecursiveGet) == 24);

class RecursiveProcesser
{
public:
    RecursiveProcesser(const std::byte *data) : _it(data) {}

    inline kF::Var process(void)
    {
        switch (*reinterpret_cast<const Type *>(_it)) {
        case 0:
        {
            _it += sizeof(RecursiveAddition);
            return process() + process();
        }
        case 1:
        {
            const auto constant = reinterpret_cast<const RecursiveConstant *>(_it);
            _it += sizeof(RecursiveConstant);
            return kF::Var::Assign(*constant->data);
        }
        case 2:
        {
            const auto get = reinterpret_cast<const RecursiveGet *>(_it);
            _it += sizeof(RecursiveGet);
            return get->target->getVar(get->data);
        }
        case 3:
        {
            _it += sizeof(RecursiveReturn);
            return process();
        }
        default:
            throw std::logic_error("Invalid Recursive instruction");
        }
    }

private:
    const std::byte *_it;
};

static void BenchRecursive(benchmark::State &state)
{
    kF::Meta::Resolver::Clear();
    kF::RegisterMetadata();

    const auto res = kF::Core::Utils::AlignedAlloc<64, std::byte>(sizeof(RecursiveAddition) + sizeof(RecursiveGet) + sizeof(RecursiveConstant));
    auto obj = std::make_unique<MyObject>();
    auto cst = std::make_unique<kF::Var>(2);

    { // Create an addition: myObject.value + 2
        auto data = res;
        // +
        new (data) RecursiveAddition {
            type: 0,
        };
        data += sizeof(RecursiveAddition);
        // myObject.value
        new (data) RecursiveGet {
            type: 2,
            data: kF::Meta::Factory<MyObject>::Resolve().findData(kF::Hash("value")),
            target: obj.get()
        };
        data += sizeof(RecursiveGet);
        // 2
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
    }

    if (RecursiveProcesser(res).process().as<int>() != 3)
        throw std::logic_error("Recursive: code is broken");
    for (auto _ : state) {
        benchmark::DoNotOptimize(RecursiveProcesser(res).process());
    }

    std::free(res);
}
BENCHMARK(BenchRecursive);

static void RecursiveAdditionBenchmarkAdditionGenerator(benchmark::State &state, const int value, const int count)
{

    kF::Meta::Resolver::Clear();
    kF::RegisterMetadata();

    const auto addCount = count - 1;
    const auto res = kF::Core::Utils::AlignedAlloc<64, std::byte>(sizeof(RecursiveAddition) * addCount + sizeof(RecursiveConstant) * count);
    auto cst = std::make_unique<kF::Var>(value);

    {
        auto data = res;
        for (auto i = 0; i < addCount; ++i) {
            new (data) RecursiveAddition {
                type: 0,
            };
            data += sizeof(RecursiveAddition);

        }
        for (auto i = 0; i < count; ++i) {
            new (data) RecursiveConstant {
                type: 1,
                data: cst.get()
            };
            data += sizeof(RecursiveConstant);
        }
    }

    if (RecursiveProcesser(res).process().as<int>() != value * count)
        throw std::logic_error("Recursive: code is broken");
    for (auto _ : state) {
        benchmark::DoNotOptimize(RecursiveProcesser(res).process());
    }

    std::free(res);
}

#define REGISTER_RECURSIVE_TEST(count) \
static void BenchRecursive##count(benchmark::State &state) \
{ \
    RecursiveAdditionBenchmarkAdditionGenerator(state, 1, count); \
} \
BENCHMARK(BenchRecursive##count);

REGISTER_RECURSIVE_TEST(2)
REGISTER_RECURSIVE_TEST(4)
REGISTER_RECURSIVE_TEST(8)
REGISTER_RECURSIVE_TEST(16)
REGISTER_RECURSIVE_TEST(32)
REGISTER_RECURSIVE_TEST(64)
REGISTER_RECURSIVE_TEST(128)
REGISTER_RECURSIVE_TEST(256)

/* Stack
    A more difficult approach to code execution using a stack and registers to manipulate memory
    No recursion nor C++ stack abuse
*/

struct alignas(8) StackAddition
{
    Type code;
    std::uint32_t reg;
    std::uint32_t lhs;
    std::uint32_t rhs;
};

struct alignas(8) StackLoadConstant
{
    Type code;
    std::uint32_t reg;
    const kF::Var *data;
};

struct alignas(8) StackLoadObjectMetaGet
{
    Type code;
    std::uint32_t reg;
    kF::Object *object;
    kF::Meta::Data data;
};

struct alignas(8) StackReturn
{
    Type code;
    std::uint32_t reg;
};

class alignas(512) StackProcesser
{
public:
    static constexpr std::size_t RegisterCount = 3;

    [[nodiscard]] inline kF::Var process(const std::byte *data, const std::uint32_t usedRegisters)
    {
        clear();
        _it = data;
        _usedRegisters = usedRegisters;

        while (true) {
            switch (*reinterpret_cast<const Type *>(_it)) {
            case 0:
                registerAt(as<StackLoadObjectMetaGet>().reg) = as<StackLoadObjectMetaGet>().object->getVar(as<StackLoadObjectMetaGet>().data);
                increment<StackLoadObjectMetaGet>();
                break;
            case 1:
                registerAt(as<StackLoadConstant>().reg).assign(*as<StackLoadConstant>().data);
                increment<StackLoadConstant>();
                break;
            case 2:
                registerAt(as<StackAddition>().reg) = registerAt(as<StackAddition>().lhs) + registerAt(as<StackAddition>().rhs);
                increment<StackAddition>();
                break;
            case 3:
                return registerAt(as<StackReturn>().reg);
            default:
                throw std::logic_error("Invalid Stack instruction");
            }
        }
    }

private:
    const std::byte *_it { nullptr };
    std::size_t _usedRegisters {};
    kF::Core::TinyVector<kF::Var> _stack {};
    std::array<kF::Var, RegisterCount> _registers {};

    [[nodiscard]] inline kF::Var &registerAt(const std::uint32_t index) noexcept { return _registers[index]; }
    [[nodiscard]] inline kF::Var &stackAt(const std::uint32_t index) noexcept { return *(_stack.rbegin() + index); }

    template<typename Type>
    inline void increment(void) noexcept { _it += sizeof(Type); }

    template<typename Type>
    [[nodiscard]] inline const Type &as(void) { return *reinterpret_cast<const Type *>(_it); }

    void clear(void)
    {
        _stack.clear();
        for (auto i = 0u; i < _usedRegisters; ++i)
            _registers[i].destruct();
    }
};

static void BenchStack(benchmark::State &state)
{
    kF::Meta::Resolver::Clear();
    kF::RegisterMetadata();

    auto res = kF::Core::Utils::AlignedAlloc<64, std::byte>(
        sizeof(StackLoadObjectMetaGet) + sizeof(StackLoadConstant) + sizeof(StackAddition) + sizeof(StackReturn)
    );
    auto obj = std::make_unique<MyObject>();
    auto cst = std::make_unique<kF::Var>(2);

    { // Create an addition: myObject.value + 2
        auto it = reinterpret_cast<std::byte *>(res);

        // myObject.value -> Register 0
        new (it) StackLoadObjectMetaGet {
            code: 0,
            reg: 0,
            object: obj.get(),
            data: kF::Meta::Factory<MyObject>::Resolve().findData(kF::Hash("value"))
        };
        it += sizeof(StackLoadObjectMetaGet);
        // 2 -> Register 1
        new (it) StackLoadConstant {
            code: 1,
            reg: 1,
            data: cst.get()
        };
        it += sizeof(StackLoadConstant);
        // Register 0 + 1 -> Register 1
        new (it) StackAddition {
            code: 2,
            reg: 1,
            lhs: 0,
            rhs: 1
        };
        it += sizeof(StackAddition);
        // Return register 2
        new (it) StackReturn {
            code: 3,
            reg: 1
        };
        it += sizeof(StackReturn);
    }

    auto processer = std::make_unique<StackProcesser>();


    if (processer->process(res, 3).as<int>() != 3)
        throw std::logic_error("Stack: code is broken");
    for (auto _ : state) {
        benchmark::DoNotOptimize(processer->process(res, 3));
    }

    std::free(res);
}

BENCHMARK(BenchStack);


static void StackAdditionBenchmarkAdditionGenerator(benchmark::State &state, const int value, const int count)
{
    kF::Meta::Resolver::Clear();
    kF::RegisterMetadata();

    const auto addCount = count - 1;

    auto res = kF::Core::Utils::AlignedAlloc<64, std::byte>(
        sizeof(StackLoadConstant) * count + sizeof(StackAddition) * addCount + sizeof(StackReturn)
    );
    auto cst = std::make_unique<kF::Var>(value);

    {
        auto it = reinterpret_cast<std::byte *>(res);

        new (it) StackLoadConstant {
            code: 1,
            reg: 0,
            data: cst.get()
        };
        it += sizeof(StackLoadConstant);
        for (auto i = 0; i < addCount; ++i) {
            new (it) StackLoadConstant {
                code: 1,
                reg: 1,
                data: cst.get()
            };
            it += sizeof(StackLoadConstant);
            new (it) StackAddition {
                code: 2,
                reg: 0,
                lhs: 0,
                rhs: 1
            };
            it += sizeof(StackAddition);
        }
        new (it) StackReturn {
            code: 3,
            reg: 0
        };
        it += sizeof(StackReturn);
    }

    auto processer = std::make_unique<StackProcesser>();

    if (processer->process(res, 10).as<int>() != value * count)
        throw std::logic_error("Stack advanced: code is broken");
    for (auto _ : state) {
        benchmark::DoNotOptimize(processer->process(res, 3));
    }

    std::free(res);
}

#define REGISTER_STACK_TEST(count) \
static void BenchStack##count(benchmark::State &state) \
{ \
    StackAdditionBenchmarkAdditionGenerator(state, 1, count); \
} \
BENCHMARK(BenchStack##count);

REGISTER_STACK_TEST(2)
REGISTER_STACK_TEST(4)
REGISTER_STACK_TEST(8)
REGISTER_STACK_TEST(16)
REGISTER_STACK_TEST(32)
REGISTER_STACK_TEST(64)
REGISTER_STACK_TEST(128)
REGISTER_STACK_TEST(256)