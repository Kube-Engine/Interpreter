/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Parser
 */

#include <iostream>

#include "Parser.hpp"

using namespace kF;

void Lang::Parser::prepare(const FileIndex file, const TokenStack *stack, const std::string_view &context)
{
    _context = context;
    _stack = stack;
    _file = file;
    _it = _stack->begin();
    _end = _stack->end();
    _processStack.clear();
    process();
}

void Lang::Parser::process(void)
{
    while (_it != _end) {
        auto literal = _it.literal();
        std::cout << literal << std::endl;
        ++_it;
    }
}
