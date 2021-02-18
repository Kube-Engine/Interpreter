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
    kF::Var *y = new kF::Var(1);

    for (auto _ : state) {
        benchmark::DoNotOptimize(obj->getVar(data) + *y);
    }
}
BENCHMARK(BenchNative);

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

    // std::cout << "Recursive: " << RecursiveProcesser(res).process().as<int>() << std::endl;
    for (auto _ : state) {
        benchmark::DoNotOptimize(RecursiveProcesser(res).process());
    }

    std::free(res);
}
BENCHMARK(BenchRecursive);


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
        // Register 0 + 1 -> Register 2
        new (it) StackAddition {
            code: 2,
            reg: 2,
            lhs: 0,
            rhs: 1
        };
        it += sizeof(StackAddition);
        // Return register 2
        new (it) StackReturn {
            code: 3,
            reg: 2
        };
        it += sizeof(StackReturn);
    }

    auto processer = std::make_unique<StackProcesser>();

    // std::cout << "Stack: " << processer->process(res, 3).as<int>() << std::endl;
    for (auto _ : state) {
        benchmark::DoNotOptimize(processer->process(res, 3));
    }

    std::free(res);
}

BENCHMARK(BenchStack);