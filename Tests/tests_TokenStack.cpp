/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Unit tests of the token stack
 */

#include <gtest/gtest.h>

#include <Kube/Interpreter/TokenStack.hpp>

using namespace kF;

TEST(TokenStack, Basics)
{
    constexpr Lang::Token Token1 { file: 1, line: 2, column: 3, length: 2 };
    constexpr Lang::Token Token2 { file: 3, line: 2, column: 1, length: 3 };

    Lang::TokenStack stack;

    ASSERT_EQ(std::distance(stack.begin(), stack.end()), 0);

    stack.push(Token1, "hello");
    ASSERT_EQ(std::distance(stack.begin(), stack.end()), 1);
    ASSERT_EQ(*stack.begin(), Token1);
    ASSERT_EQ(stack.begin().literal(), "he");

    stack.push(Token2, "world");
    ASSERT_EQ(std::distance(stack.begin(), stack.end()), 2);
    ASSERT_EQ(*++stack.begin(), Token2);
    ASSERT_EQ((++stack.begin()).literal(), "wor");
}