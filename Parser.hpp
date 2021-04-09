/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Parser
 */

#pragma once

#include <istream>

#include <Kube/Core/Vector.hpp>
#include <Kube/Core/AllocatedVector.hpp>
#include <Kube/Core/AllocatedSmallString.hpp>

#include "TokenStack.hpp"
#include "AST.hpp"

namespace kF::Lang
{
    class Parser;
}

/** @brief The Parser is a processing class that takes an input stream and return a TokenStack */
class alignas_double_cacheline kF::Lang::Parser
{
public:
    /** @brief Process the Parser over a input stream */
    [[nodiscard]] AST::Ptr run(const FileIndex file, const TokenStack *stack, const std::string_view &context)
        { prepare(file, stack, context); return std::move(_root); }

private:
    std::string_view _context {};
    Core::TinyVector<AST *> _processStack {};
    const TokenStack *_stack { nullptr };
    TokenStack::Iterator _it { nullptr };
    TokenStack::Iterator _end { nullptr };
    AST::Ptr _root {};
    FileIndex _file {};

    /** @brief Prepare the instance for the next process */
    void prepare(const FileIndex file, const TokenStack *stack, const std::string_view &context);

    /** @brief Process AST from the token stack */
    void process(void);
};

static_assert_fit_double_cacheline(kF::Lang::Parser);

#include "Parser.ipp"