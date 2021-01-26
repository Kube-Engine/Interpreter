/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Unit tests of Lexer
 */

#include <sstream>

#include <gtest/gtest.h>

#include <Kube/Interpreter/Lexer.hpp>

using namespace kF;

static void TestToken(const Lang::TokenStack::Iterator it,
        const Lang::FileIndex file, const Lang::LineIndex line, const Lang::ColumnIndex column, const std::string_view &word)
{
    ASSERT_EQ(it->file, file);
    ASSERT_EQ(it->line, line);
    ASSERT_EQ(it->column, column);
    ASSERT_EQ(it->length, word.size());
    ASSERT_EQ(it.literal(), word);
}

TEST(Lexer, Basics)
{
    std::istringstream iss("Hello\t\n\tworld()");

    Lang::Lexer lexer;
    auto stack = lexer.run(0, iss, "Root");
    auto it = stack.begin();
    TestToken(it++, 0, 1, 1, "Hello");
    TestToken(it++, 0, 2, 2, "world");
    TestToken(it++, 0, 2, 7, "(");
    TestToken(it++, 0, 2, 8, ")");
}

TEST(Lexer, BasicString)
{
    std::istringstream iss("\"\"\n\"Hello\"\n\"4\\n2\"");

    Lang::Lexer lexer;
    auto stack = lexer.run(0, iss, "Root");
    auto it = stack.begin();
    TestToken(it++, 0, 1, 2, "\0");
    TestToken(it++, 0, 2, 2, "Hello");
    TestToken(it++, 0, 3, 2, "4\n2");
}

TEST(Lexer, BasicCharacter)
{
    std::istringstream iss("\'\n\'\n\'4\'");

    Lang::Lexer lexer;
    auto stack = lexer.run(0, iss, "Root");
    auto it = stack.begin();
    TestToken(it++, 0, 1, 2, "\n");
    TestToken(it++, 0, 2, 2, "4");
}

TEST(Lexer, Compact)
{
    std::istringstream iss("Item:item{x:100;y:1'000;Rectangle:child{x_01:0.5;y_01:42.24}}");

    Lang::Lexer lexer;
    auto stack = lexer.run(42, iss, "Root");
    auto it = stack.begin();
    TestToken(it++, 42, 1, 1, "Item"); TestToken(it++, 42, 1, 5, ":"); TestToken(it++, 42, 1, 6, "item");
    TestToken(it++, 42, 1, 10, "{");
        TestToken(it++, 42, 1, 11, "x"); TestToken(it++, 42, 1, 12, ":"); TestToken(it++, 42, 1, 13, "100"); TestToken(it++, 42, 1, 16, ";");
        TestToken(it++, 42, 1, 17, "y"); TestToken(it++, 42, 1, 18, ":"); TestToken(it++, 42, 1, 19, "1'000"); TestToken(it++, 42, 1, 24, ";");
        TestToken(it++, 42, 1, 25, "Rectangle"); TestToken(it++, 42, 1, 34, ":"); TestToken(it++, 42, 1, 35, "child");
        TestToken(it++, 42, 1, 40, "{");
            TestToken(it++, 42, 1, 41, "x_01"); TestToken(it++, 42, 1, 45, ":"); TestToken(it++, 42, 1, 46, "0.5"); TestToken(it++, 42, 1, 49, ";");
            TestToken(it++, 42, 1, 50, "y_01"); TestToken(it++, 42, 1, 54, ":"); TestToken(it++, 42, 1, 55, "42.24");
        TestToken(it++, 42, 1, 60, "}");
    TestToken(it++, 42, 1, 61, "}");

}