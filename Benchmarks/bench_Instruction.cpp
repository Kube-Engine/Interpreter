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

static void BenchNativeAdvanced(benchmark::State &state)
{
    kF::Meta::Resolver::Clear();
    kF::RegisterMetadata();
    kF::Var *x = new kF::Var(1);

    if ((*x + *x + *x + *x + *x + *x + *x + *x + *x + *x).as<int>() != 10)
        throw std::logic_error("Native advanced: code is broken");
    for (auto _ : state) {
        benchmark::DoNotOptimize(*x + *x + *x + *x + *x + *x + *x + *x + *x + *x);
    }
}
BENCHMARK(BenchNativeAdvanced);

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
    }

    //

    if (RecursiveProcesser(res).process().as<int>() != 3)
        throw std::logic_error("Recursive: code is broken");
    for (auto _ : state) {
        benchmark::DoNotOptimize(RecursiveProcesser(res).process());
    }

    std::free(res);
}
BENCHMARK(BenchRecursive);

static void BenchRecursiveAdvanced(benchmark::State &state)
{
    kF::Meta::Resolver::Clear();
    kF::RegisterMetadata();

    const auto res = kF::Core::Utils::AlignedAlloc<64, std::byte>(sizeof(RecursiveAddition) * 9 + sizeof(RecursiveConstant) * 10);
    auto cst = std::make_unique<kF::Var>(1);

    { // Create an addition: 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1
        auto data = res;
        // +
        new (data) RecursiveAddition {
            type: 0,
        };
        data += sizeof(RecursiveAddition);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
        // +
        new (data) RecursiveAddition {
            type: 0,
        };
        data += sizeof(RecursiveAddition);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
        // +
        new (data) RecursiveAddition {
            type: 0,
        };
        data += sizeof(RecursiveAddition);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
        // +
        new (data) RecursiveAddition {
            type: 0,
        };
        data += sizeof(RecursiveAddition);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
        // +
        new (data) RecursiveAddition {
            type: 0,
        };
        data += sizeof(RecursiveAddition);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
        // +
        new (data) RecursiveAddition {
            type: 0,
        };
        data += sizeof(RecursiveAddition);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
        // +
        new (data) RecursiveAddition {
            type: 0,
        };
        data += sizeof(RecursiveAddition);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
        // +
        new (data) RecursiveAddition {
            type: 0,
        };
        data += sizeof(RecursiveAddition);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
        // +
        new (data) RecursiveAddition {
            type: 0,
        };
        data += sizeof(RecursiveAddition);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
    }

    //

    if (RecursiveProcesser(res).process().as<int>() != 10)
        throw std::logic_error("Recursive advanced: code is broken");
    for (auto _ : state) {
        benchmark::DoNotOptimize(RecursiveProcesser(res).process());
    }

    std::free(res);
}
BENCHMARK(BenchRecursiveAdvanced);

static void BenchRecursiveAdvancedAlt(benchmark::State &state)
{
    kF::Meta::Resolver::Clear();
    kF::RegisterMetadata();

    const auto res = kF::Core::Utils::AlignedAlloc<64, std::byte>(sizeof(RecursiveAddition) * 9 + sizeof(RecursiveConstant) * 10);
    auto cst = std::make_unique<kF::Var>(1);

    { // Create an addition: 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1
        auto data = res;
        // +
        new (data) RecursiveAddition {
            type: 0,
        };
        data += sizeof(RecursiveAddition);
        // +
        new (data) RecursiveAddition {
            type: 0,
        };
        data += sizeof(RecursiveAddition);
        // +
        new (data) RecursiveAddition {
            type: 0,
        };
        data += sizeof(RecursiveAddition);
        // +
        new (data) RecursiveAddition {
            type: 0,
        };
        data += sizeof(RecursiveAddition);
        // +
        new (data) RecursiveAddition {
            type: 0,
        };
        data += sizeof(RecursiveAddition);
        // +
        new (data) RecursiveAddition {
            type: 0,
        };
        data += sizeof(RecursiveAddition);
        // +
        new (data) RecursiveAddition {
            type: 0,
        };
        data += sizeof(RecursiveAddition);
        // +
        new (data) RecursiveAddition {
            type: 0,
        };
        data += sizeof(RecursiveAddition);
        // +
        new (data) RecursiveAddition {
            type: 0,
        };
        data += sizeof(RecursiveAddition);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
        // 1
        new (data) RecursiveConstant {
            type: 1,
            data: cst.get()
        };
        data += sizeof(RecursiveConstant);
    }

    //

    if (RecursiveProcesser(res).process().as<int>() != 10)
        throw std::logic_error("Recursive advanced (alt): code is broken");
    for (auto _ : state) {
        benchmark::DoNotOptimize(RecursiveProcesser(res).process());
    }

    std::free(res);
}
BENCHMARK(BenchRecursiveAdvancedAlt);


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


static void BenchStackAdvanced(benchmark::State &state)
{
    kF::Meta::Resolver::Clear();
    kF::RegisterMetadata();

    auto res = kF::Core::Utils::AlignedAlloc<64, std::byte>(
        sizeof(StackLoadConstant) * 10 + sizeof(StackAddition) * 9 + sizeof(StackReturn)
    );
    auto cst = std::make_unique<kF::Var>(1);

    { // Create an addition: 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1
        auto it = reinterpret_cast<std::byte *>(res);

        // 1 -> Register 0
        new (it) StackLoadConstant {
            code: 1,
            reg: 0,
            data: cst.get()
        };
        it += sizeof(StackLoadConstant);
        // 1 -> Register 1
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
        // 1 -> Register 0
        new (it) StackLoadConstant {
            code: 1,
            reg: 0,
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
        // 1 -> Register 0
        new (it) StackLoadConstant {
            code: 1,
            reg: 0,
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
        // 1 -> Register 0
        new (it) StackLoadConstant {
            code: 1,
            reg: 0,
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
        // 1 -> Register 0
        new (it) StackLoadConstant {
            code: 1,
            reg: 0,
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
        // 1 -> Register 0
        new (it) StackLoadConstant {
            code: 1,
            reg: 0,
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
        // 1 -> Register 0
        new (it) StackLoadConstant {
            code: 1,
            reg: 0,
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
        // 1 -> Register 0
        new (it) StackLoadConstant {
            code: 1,
            reg: 0,
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
        // 1 -> Register 0
        new (it) StackLoadConstant {
            code: 1,
            reg: 0,
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

    if (processer->process(res, 10).as<int>() != 10)
        throw std::logic_error("Stack advanced: code is broken");
    for (auto _ : state) {
        benchmark::DoNotOptimize(processer->process(res, 3));
    }

    std::free(res);
}

BENCHMARK(BenchStackAdvanced);


static void BenchStackAdvancedAlt(benchmark::State &state)
{
    kF::Meta::Resolver::Clear();
    kF::RegisterMetadata();

    auto res = kF::Core::Utils::AlignedAlloc<64, std::byte>(
        sizeof(StackLoadConstant) * 10 + sizeof(StackAddition) * 9 + sizeof(StackReturn)
    );
    auto cst = std::make_unique<kF::Var>(1);

    { // Create an addition: 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1
        auto it = reinterpret_cast<std::byte *>(res);

        // 1 -> Register 0
        new (it) StackLoadConstant {
            code: 1,
            reg: 0,
            data: cst.get()
        };
        it += sizeof(StackLoadConstant);
        // 1 -> Register 1
        new (it) StackLoadConstant {
            code: 1,
            reg: 1,
            data: cst.get()
        };
        it += sizeof(StackLoadConstant);
        // 1 -> Register 2
        new (it) StackLoadConstant {
            code: 1,
            reg: 2,
            data: cst.get()
        };
        it += sizeof(StackLoadConstant);
        // 1 -> Register 3
        new (it) StackLoadConstant {
            code: 1,
            reg: 3,
            data: cst.get()
        };
        it += sizeof(StackLoadConstant);
        // 1 -> Register 4
        new (it) StackLoadConstant {
            code: 1,
            reg: 4,
            data: cst.get()
        };
        it += sizeof(StackLoadConstant);
        // 1 -> Register 5
        new (it) StackLoadConstant {
            code: 1,
            reg: 5,
            data: cst.get()
        };
        it += sizeof(StackLoadConstant);
        // 1 -> Register 6
        new (it) StackLoadConstant {
            code: 1,
            reg: 6,
            data: cst.get()
        };
        it += sizeof(StackLoadConstant);
        // 1 -> Register 7
        new (it) StackLoadConstant {
            code: 1,
            reg: 7,
            data: cst.get()
        };
        it += sizeof(StackLoadConstant);
        // 1 -> Register 8
        new (it) StackLoadConstant {
            code: 1,
            reg: 8,
            data: cst.get()
        };
        it += sizeof(StackLoadConstant);
        // 1 -> Register 9
        new (it) StackLoadConstant {
            code: 1,
            reg: 9,
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
        // Register 1 + 2 -> Register 2
        new (it) StackAddition {
            code: 2,
            reg: 2,
            lhs: 1,
            rhs: 2
        };
        it += sizeof(StackAddition);
        // Register 2 + 3 -> Register 3
        new (it) StackAddition {
            code: 2,
            reg: 3,
            lhs: 2,
            rhs: 3
        };
        it += sizeof(StackAddition);
        // Register 3 + 4 -> Register 4
        new (it) StackAddition {
            code: 2,
            reg: 4,
            lhs: 3,
            rhs: 4
        };
        it += sizeof(StackAddition);
        // Register 4 + 5 -> Register 5
        new (it) StackAddition {
            code: 2,
            reg: 5,
            lhs: 4,
            rhs: 5
        };
        it += sizeof(StackAddition);
        // Register 5 + 6 -> Register 6
        new (it) StackAddition {
            code: 2,
            reg: 6,
            lhs: 5,
            rhs: 6
        };
        it += sizeof(StackAddition);
        // Register 6 + 7 -> Register 7
        new (it) StackAddition {
            code: 2,
            reg: 7,
            lhs: 6,
            rhs: 7
        };
        it += sizeof(StackAddition);
        // Register 7 + 8 -> Register 8
        new (it) StackAddition {
            code: 2,
            reg: 8,
            lhs: 7,
            rhs: 8
        };
        it += sizeof(StackAddition);
        // Register 8 + 9 -> Register 9
        new (it) StackAddition {
            code: 2,
            reg: 9,
            lhs: 8,
            rhs: 9
        };
        it += sizeof(StackAddition);
        // Return register 2
        new (it) StackReturn {
            code: 3,
            reg: 9
        };
        it += sizeof(StackReturn);
    }

    auto processer = std::make_unique<StackProcesser>();

    if (processer->process(res, 10).as<int>() != 10)
        throw std::logic_error("Stack advanced (alt): code is broken");
    for (auto _ : state) {
        benchmark::DoNotOptimize(processer->process(res, 10));
    }

    std::free(res);
}

BENCHMARK(BenchStackAdvancedAlt);