/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Benchmark of instructions
 */

#include <iostream>
#include <utility>

#include <benchmark/benchmark.h>

#include <Kube/Interpreter/Instruction.hpp>
#include <Kube/Object/Object.hpp>

using Type = std::uint32_t;

class MyObject : public kF::Object
{
    K_DERIVED(MyObject, kF::Object,
        K_PROPERTY(int, value, 1)
    )
};

// ------------- NATIVE

static void BenchNative(benchmark::State &state)
{
    kF::Meta::Resolver::Clear();
    kF::RegisterMetadata();
    MyObject *obj = new MyObject();
    auto data = kF::Meta::Factory<MyObject>::Resolve().findData(kF::Hash("value"));
    kF::Var *y = new kF::Var(1);

    for (auto _ : state) {
        benchmark::DoNotOptimize(obj->getVar(data) + *y);
    }
}
BENCHMARK(BenchNative);

// ------------- PACKED

#pragma pack(push, 1)
struct PackedAddition
{
    Type type {};
};

struct PackedConstant
{
    Type type {};
    kF::Var *data;
};

struct PackedGet
{
    Type type {};
    kF::Meta::Data data {};
    kF::Object *target {};
};
#pragma pack(pop)

static_assert(sizeof(PackedAddition) == 4);
static_assert(sizeof(PackedConstant) == 12);
static_assert(sizeof(PackedGet) == 20);

std::byte * CreateBuffer(void)
{
    const auto res = kF::Core::Utils::AlignedAlloc<64, std::byte>(sizeof(PackedAddition) + sizeof(PackedGet) + sizeof(PackedConstant));
    auto begin = res;

    auto data = begin;

    new (data) PackedAddition {
        type: 0,
    };
    data += sizeof(PackedAddition);
    new (data) PackedGet {
        type: 2,
        data: kF::Meta::Factory<MyObject>::Resolve().findData(kF::Hash("value")),
        target: new MyObject()
    };
    data += sizeof(PackedGet);
    // new (data) Constant {
    //     type: 2,
    //     data: new kF::Var(kF::Var::Emplace<MyObject>())
    // };
    // data += sizeof(Constant);
    new (data) PackedConstant {
        type: 1,
        data: new kF::Var(1)
    };
    return res;
}

struct PackedProcesser
{
    const std::byte *it;

    PackedProcesser(const std::byte *data) : it(data) {}

    inline kF::Var process(void)
    {
        switch (*reinterpret_cast<const Type *>(it)) {
        case 0:
            it += sizeof(PackedAddition);
            return process() + process();
        case 1:
        {
            const auto constant = reinterpret_cast<const PackedConstant *>(it);
            it += sizeof(PackedConstant);
            return kF::Var::Assign(constant->data);
        }
        case 2:
        {
            const auto get = reinterpret_cast<const PackedGet *>(it);
            it += sizeof(PackedGet);
            return get->target->getVar(get->data);
        }
        default:
            throw std::logic_error("Invalid packed instruction");
        }
    }
};

static void BenchPacked(benchmark::State &state)
{
    kF::Meta::Resolver::Clear();
    kF::RegisterMetadata();
    auto buffer = CreateBuffer();

    for (auto _ : state) {
        benchmark::DoNotOptimize(PackedProcesser(buffer).process());
    }
}
BENCHMARK(BenchPacked);

// ------------- Stack

struct StackInstruction
{
    std::uint32_t code {};
    std::uint32_t addr {};
};

struct StackAddition : public StackInstruction
{
};

struct StackConstant : public StackInstruction
{
    kF::Var *data;
};


// if (a + b > c)
//     return d * a
// else
//     return b * 42

// |(a)
// |(a),(b)
// |(a+b)
// |(a+b)
// |(a+b),(c)
// |(a+b>c)



// struct StackGet
// {
//     Type type {};
//     kF::Meta::Data data {};
//     kF::Object *target {};
// };


// struct Stack
// {
//     kF::Core::TinyVector<std::byte> instructionStack;
//     kF::Core::TinyVector<kF::Var> dataStack;
// };


// static void BenchStack(benchmark::State &state)
// {

// }

// BENCHMARK(BenchStack);