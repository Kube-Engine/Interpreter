/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Unit tests of Object
 */

#include <gtest/gtest.h>

#include <Kube/Interpreter/Object.hpp>

using namespace kF;
using namespace kF::Literal;

class BasicFoo : public Object
{
    KUBE_MAKE_PROPERTY_COPY(int, data, 0)
    KUBE_MAKE_SIGNAL(signal, int)

    KUBE_REGISTER(BasicFoo,
        KUBE_REGISTER_PROPERTY_COPY(int, data)
        KUBE_REGISTER_SIGNAL(signal)
    )
};

TEST(Object, Basics)
{
    Meta::Resolver::Clear();
    RegisterMetadata();
    BasicFoo::RegisterMetadata();

    BasicFoo foo, foo2;
    int x = 0;
    foo.connect<&BasicFoo::dataChanged>([&x] {
        ++x;
    });
    foo2.connect<&BasicFoo::signal>([&foo](int x) {
        foo.data(x);
    });
    foo2.emit<&BasicFoo::signal>(42);
    ASSERT_EQ(foo.data(), 42);
    ASSERT_EQ(x, 1);
    foo2.emit("signal"_hash, 24);
    ASSERT_EQ(foo.data(), 24);
    ASSERT_EQ(x, 2);
    foo2.emit<&BasicFoo::signal>(24);
    ASSERT_EQ(x, 2);
}
