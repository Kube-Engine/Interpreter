/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Lexer
 */

inline kF::Lang::Lexer::ProcessState kF::Lang::Lexer::processRegularToken(const char begin) noexcept
{
    if (std::isalpha(begin)) {
        beginToken<false>(begin);
        for (char elem = peek(); elem; elem = peek()) {
            if (std::isalnum(elem) || elem == '_') [[likely]]
                feedToken<false>(elem);
            else [[unlikely]]
                break;
        }
        endToken();
        return ProcessState::Success;
    } else if (std::isdigit(begin)) {
        return processNumeric(begin);
    } else
        return ProcessState::NotRecognized;
}

inline kF::Lang::Lexer::ProcessState kF::Lang::Lexer::processNumeric(const char begin) noexcept
{
    bool dot = false;
    char elem;

    // Process numeric
    beginToken<false>(begin);
    for (elem = peek(); elem; elem = peek()) {
        if (elem == '.') [[unlikely]] {
            if (dot)
                return ProcessState::NotRecognized;
            dot = true;
        } else if (!std::isdigit(elem))
            break;
        feedToken<false>(elem);
    }

    // Process suffix
    switch (elem) {
    case 'u':
        if (peekNext() == 'l') {
            feedToken<false>(elem);
            elem = peek();
        }
        break;
    case 'l':
        if (const char next = peekNext(); next == 'l' || next == 'd') {
            feedToken<false>(elem);
            elem = peek();
        }
        break;
    case 's':
    case 'd':
        break;
    default:
        endToken();
        return ProcessState::Success;
    }
    feedToken<false>(elem);
    endToken();
    return ProcessState::Success;
}

inline kF::Lang::Lexer::ProcessState kF::Lang::Lexer::processSpecialToken(const char begin) noexcept
{
    switch (begin) {
    // Single char operators
    case '(':
    case ')':
    case '{':
    case '}':
    case '[':
    case ']':
    case '?':
    case ':':
    case ',':
    case ';':
    case '.':
    case '~':
        pushSingleCharToken(begin);
        return ProcessState::Success;
    // Composed operators with '='
    case '=':
    case '<':
    case '>':
    case '!':
    case '*':
    case '%':
    case '^':
        processComposedSpecialToken<'='>(begin);
        return ProcessState::Success;
    // Custom composed operators
    case '|':
        processComposedSpecialToken<'|', '='>(begin);
        return ProcessState::Success;
    case '&':
        processComposedSpecialToken<'&', '='>(begin);
        return ProcessState::Success;
    case '+':
        processComposedSpecialToken<'+', '='>(begin);
        return ProcessState::Success;
    case '-':
        processComposedSpecialToken<'-', '='>(begin);
        return ProcessState::Success;
    // Custom cases
    case '/': // Can be either division or comment
        switch (peekNext()) {
        case '/': // It's a line comment
            skipComment();
            return ProcessState::Success;
        case '*': // It's a multiline comment
            if (skipMultilineComment()) [[likely]]
                return ProcessState::Success;
            else [[unlikely]]
                return ProcessState::Error;
        default: // It's a division
            processComposedSpecialToken<'='>(begin);
            return ProcessState::Success;
        }
        break;
    case '"':
        if (parseString()) [[likely]]
            return ProcessState::Success;
        else [[unlikely]]
            return ProcessState::Error;
    case '\'':
        if (parseCharacter()) [[likely]]
            return ProcessState::Success;
        else [[unlikely]]
            return ProcessState::Error;
    default:
        return ProcessState::NotRecognized;
    }
}

template<char ...Values>
inline void kF::Lang::Lexer::processComposedSpecialToken(const char begin) noexcept
{
    beginToken<false>(begin);
    const char elem = peek();
    if (((elem == Values) || ...))
        feedToken<false>(elem);
    endToken();
}

inline bool kF::Lang::Lexer::parseString(void) noexcept
{
    consume<false>();
    char elem = peek();
    if (elem == '"') [[unlikely]] {
        beginToken('"');
        feedToken('"');
        endToken();
        return true;
    }
    beginToken('"');
    feedToken(elem);
    for (elem = peek(); elem; elem = peek()) {
        if (elem != '"') [[likely]] {
            if (elem != '\\') [[likely]]
                feedToken(elem);
            else [[unlikely]] {
                consume<false>();
                switch (elem = peek()) {
                case '\\':
                    feedToken<false>('\\');
                    break;
                case '"':
                    feedToken<false>('"');
                    break;
                case '\'':
                    feedToken<false>('\'');
                    break;
                case 't':
                    feedToken<false>('\t');
                    break;
                case 'n':
                    feedToken<false>('\n');
                    break;
                case 'v':
                    feedToken<false>('\v');
                    break;
                case 'f':
                    feedToken<false>('\f');
                    break;
                case 'r':
                    feedToken<false>('\r');
                    break;
                case '0':
                    feedToken<false>('\0');
                    break;
                default:
                    consume(elem);
                    break;
                }
            }
        } else [[unlikely]] {
            feedToken('"');
            consume<false>();
            endToken();
            return true;
        }
    }
    return false;
}

inline bool kF::Lang::Lexer::parseCharacter(void) noexcept
{
    consume<false>();
    char elem = peek();
    if (elem != '\\') [[likely]] {
        if (peekNext() == '\'') [[likely]] {
            pushSingleCharToken(elem);
            consume<false>();
            return true;
        } else [[unlikely]]
            return false;
    } else [[unlikely]] {
        consume<false>();
        elem = peek();
        if (peekNext() != '\'') [[unlikely]]
            return false;
        switch (elem) {
        case '\\':
            feedToken<false>('\\');
            break;
        case '"':
            feedToken<false>('"');
            break;
        case '\'':
            feedToken<false>('\'');
            break;
        case 't':
            feedToken<false>('\t');
            break;
        case 'n':
            feedToken<false>('\n');
            break;
        case 'v':
            feedToken<false>('\v');
            break;
        case 'f':
            feedToken<false>('\f');
            break;
        case 'r':
            feedToken<false>('\r');
            break;
        case '0':
            feedToken<false>('\0');
            break;
        default:
            return false;
        }
        consume<false>();
        return true;
    }
}

inline void kF::Lang::Lexer::skipComment(void) noexcept
{
    consumeNext();
    for (char current = peek(); current; current = peek()) [[likely]] {
        if (current != '\n') [[likely]]
            consume<false>();
        else [[unlikely]] {
            consume<true>();
            break;
        }
    }
}

inline bool kF::Lang::Lexer::skipMultilineComment(void) noexcept
{
    consumeNext();
    char current = peek();
    while (current) [[likely]] {
        if (current != '*') [[likely]] {
            consume(current);
            current = peek();
        } else [[unlikely]] {
            consume<false>();
            current = peek();
            if (current == '/') {
                consume<false>();
                break;
            }
        }
    }
    return current != '\0';
}

inline char kF::Lang::Lexer::peek(void) const noexcept
{
    if (_index < _buffer.size()) [[likely]]
        return _buffer[_index];
    else [[unlikely]]
        return '\0';
}

inline char kF::Lang::Lexer::peekNext(void) const noexcept
{
    if (_index + 1 < _buffer.size()) [[likely]]
        return _buffer[_index + 1];
    else [[unlikely]]
        return '\0';
}

inline void kF::Lang::Lexer::consume(const char current) noexcept
{
    if (current != '\n') [[likely]]
        ++_column;
    else [[unlikely]] {
        ++_line;
        _column = 1u;
    }
    ++_index;
}

template<bool IsNewLine>
inline void kF::Lang::Lexer::consume(void) noexcept
{
    if constexpr (IsNewLine) {
        ++_line;
        _column = 1u;
    } else
        ++_column;
    ++_index;
}

inline void kF::Lang::Lexer::consumeNext(void) noexcept
{
    _column += 2;
    _index += 2;
}

inline void kF::Lang::Lexer::beginToken(const char begin) noexcept
{
    _token.line = _line;
    _token.column = _column;
    _cache.push(begin);
    consume(begin);
}

template<bool IsNewLine>
inline void kF::Lang::Lexer::beginToken(const char begin) noexcept
{
    _token.line = _line;
    _token.column = _column;
    _cache.push(begin);
    consume<IsNewLine>();
}

inline void kF::Lang::Lexer::feedToken(const char elem) noexcept
{
    _cache.push(elem);
    consume(elem);
}

template<bool IsNewLine>
inline void kF::Lang::Lexer::feedToken(const char elem) noexcept
{
    _cache.push(elem);
    consume<IsNewLine>();
}

inline void kF::Lang::Lexer::endToken(void) noexcept
{
    _token.length = _cache.size();
    _stack.push(_token, _cache.data());
    _cache.clearUnsafe();
}

inline void kF::Lang::Lexer::pushSingleCharToken(const char begin) noexcept
{
    _token.line = _line;
    _token.column = _column;
    _token.length = 1;
    consume<false>();
    _stack.push(_token, &begin);
}

inline void kF::Lang::Lexer::pushEmptyToken(void) noexcept
{
    _token.line = _line;
    _token.column = _column;
    _token.length = 0;
    consume<false>();
    _stack.push(_token, nullptr);
}