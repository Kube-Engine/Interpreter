/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Lexer
 */

#include <Kube/Core/StringLiteral.hpp>

#include "Lexer.hpp"

using namespace kF;
using namespace kF::Literal;

void Lang::Lexer::prepare(const FileIndex file, std::istream &istream, const std::string_view &context)
{
    _token.file = file;
    _line = 1u;
    _column = 1u;
    _context = context;
    istream.seekg(0u, std::ios::end);
    const std::size_t fileSize = istream.tellg();
    if (!fileSize)
        throw std::logic_error(FormatStdString("Lang::Lexer::prepare: File '", _context, "' is empty"));
    istream.seekg(0u, std::ios::beg);
    _buffer.resize(fileSize);
    istream.read(_buffer.begin(), fileSize);
    process();
}

void Lang::Lexer::process(void)
{
    for (char current = peek(); current != '\0'; current = peek()) [[likely]] {
        if (std::isspace(current)) {
            consume(current);
        } else {
            switch (processRegularToken(current)) {
            case ProcessState::Success:
                break;
            case ProcessState::Error:
                throw std::logic_error(FormatStdString("Lang::Lexer::process: Error while processing regular token character '",
                        current, "' (line ", _line, " column ", _column, ") in file '", _context));
            case ProcessState::NotRecognized:
                switch (processSpecialToken(current)) {
                case ProcessState::Success:
                    break;
                case ProcessState::NotRecognized:
                    throw std::logic_error(FormatStdString("Lang::Lexer::process: Unrecognized character '",
                            current, "' (line ", _line, " column ", _column, ") in file '", _context));
                case ProcessState::Error:
                    throw std::logic_error(FormatStdString("Lang::Lexer::process: Error while processing special token character '",
                            current, "' (line ", _line, " column ", _column, ") in file '", _context));
                }
            }
        }
    }
}