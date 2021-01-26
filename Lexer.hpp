/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Lexer
 */

#pragma once

#include <istream>

#include <Kube/Core/AllocatedVector.hpp>
#include <Kube/Core/AllocatedSmallString.hpp>

#include "TokenStack.hpp"

namespace kF::Lang
{
    class Lexer;
}

/** @brief The lexer is a processing class that takes an input stream and return a TokenStack */
class alignas_double_cacheline kF::Lang::Lexer
{
public:
    /** @brief State returned by internal processing functions */
    enum class ProcessState {
        Error,
        Success,
        NotRecognized
    };

    /** @brief Process the lexer over a input stream */
    [[nodiscard]] TokenStack run(const FileIndex file, std::istream &istream, const std::string_view &context)
        { prepare(file, istream, context); return TokenStack(std::move(_stack)); }

private:
    Core::AllocatedTinyVector<char, &TokenStack::Allocate, &TokenStack::Deallocate> _buffer {};
    TokenStack _stack {};
    Token _token {};
    LineIndex _line { 0u };
    ColumnIndex _column { 0u };
    std::uint32_t _index { 0u };
    std::string_view _context {};
    Core::AllocatedSmallStringBase<char, Core::CacheLineSize - Core::CacheLineEighthSize - 2 * sizeof(std::uint16_t),
            &TokenStack::Allocate, &TokenStack::Deallocate, std::uint16_t> _cache {};


    /** @brief Prepare the instance for the next process */
    void prepare(const FileIndex file, std::istream &istream, const std::string_view &context);

    /** @brief Process internal buffer into the token stack */
    void process(void);

    /** @brief Process regular tokens such as alphanumerics
     *  The function keep lexer's position after the last character captured
     */
    [[nodiscard]] ProcessState processRegularToken(const char begin) noexcept;

    /** @brief Process special tokens such as operators or comments
     *  The function keep lexer's position after the last character captured
    */
    [[nodiscard]] ProcessState processSpecialToken(const char begin) noexcept;

    /** @brief Helper used to retreive composed special tokens */
    template<char ...Values>
    void processComposedSpecialToken(const char begin) noexcept;

    /** @brief Parse a string, return false on error
     *  Assumes that the next peek is a double quote */
    [[nodiscard]] bool parseString(void) noexcept;

    /** @brief Parse a character, return false on error
     *  Assumes that the next peek is a simple quote */
    [[nodiscard]] bool parseCharacter(void) noexcept;

    /** @brief Skip a line
     *  Assumes that the two next peeks are a single line comment token */
    void skipComment(void) noexcept;

    /** @brief Skip a multiline comment, return false on error
     *  Assumes that the two next peeks are a multiline comment token */
    [[nodiscard]] bool skipMultilineComment(void) noexcept;


    /** @brief Peek the next character */
    [[nodiscard]] char peek(void) const noexcept;

    /** @brief Peek the character after the next character */
    [[nodiscard]] char peekNext(void) const noexcept;

    /** @brief Consume the next character
     *  The character to be consumed must be the exact one lastly peeked */
    void consume(const char consumed) noexcept;

    /** @brief Consume the next character without checking if it is a new-line */
    template<bool IsNewLine>
    void consume(void) noexcept;

    /** @brief Consume the two following character without checking if they are new-lines */
    void consumeNext(void) noexcept;


    /** @brief Begin token recording */
    void beginToken(const char begin) noexcept;

    /** @brief Begin token recording without checking if character is new-line */
    template<bool IsNewLine>
    void beginToken(const char begin) noexcept;

    /** @brief Feed the current token being recorded */
    void feedToken(const char elem) noexcept;

    /** @brief Feed the current token being recorded without checking if character is new-line */
    template<bool IsNewLine>
    void feedToken(const char elem) noexcept;

    /** @brief Push the current token and cache into the token stack */
    void endToken(void) noexcept;

    /** @brief Push a single character token */
    void pushSingleCharToken(const char begin) noexcept;

    /** @brief Push an empty token */
    void pushEmptyToken(void) noexcept;
};

static_assert_fit_double_cacheline(kF::Lang::Lexer);

#include "Lexer.ipp"