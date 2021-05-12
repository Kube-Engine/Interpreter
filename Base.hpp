/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Base
 */

#pragma once

#include <string_view>

#include <Kube/Core/Utils.hpp>
#include <Kube/Core/Hash.hpp>

namespace kF::Lang
{
    /** @brief A directory index */
    using DirectoryIndex = std::uint32_t;

    /** @brief A file index */
    using FileIndex = std::uint16_t;

    /** @brief A file's line index */
    using LineIndex = std::uint16_t;

    /** @brief A file line's column index */
    using ColumnIndex = std::uint16_t;

    /** @brief A token in a file */
    struct alignas_eighth_cacheline Token
    {
        FileIndex file { 0u };
        LineIndex line { 0u };
        ColumnIndex column { 0u };
        std::uint16_t length { 0u };

        /** @brief Comparison operator */
        [[nodiscard]] bool operator==(const Token &other) const noexcept
            { return file == other.file && line == other.line && column == other.column && length == other.length; }

        /** @brief Get the token literal */
        [[nodiscard]] std::string_view literal(void) const noexcept
            { return length ? std::string_view(reinterpret_cast<const char *>(this + 1), length) : std::string_view(); }

        /** @brief Token iterator */
        class Iterator
        {
        public:
            /** @brief STL compatibility */
            using iterator_category = std::forward_iterator_tag;
            using value_type = Token;
            using difference_type = std::size_t;
            using pointer = Token *;
            using reference = Token &;

            /** @brief Constructor */
            Iterator(void) noexcept = default;

            /** @brief Data constructor */
            Iterator(const Token *data) noexcept : _data(data) {}

            /** @brief Copy constructor */
            Iterator(const Iterator &other) noexcept = default;

            /** @brief Destructor */
            ~Iterator(void) noexcept = default;

            /** @brief Copy assignment */
            Iterator &operator=(const Iterator &other) noexcept = default;

            /** @brief Comparison operator */
            [[nodiscard]] bool operator==(const Iterator &other) const noexcept
                { return _data == other._data; }
            [[nodiscard]] bool operator!=(const Iterator &other) const noexcept
                { return _data != other._data; }

            /** @brief Access operators */
            [[nodiscard]] const Token &operator*(void) const noexcept { return *_data; }
            [[nodiscard]] const Token *operator->(void) const noexcept { return _data; }

            /** @brief Retreive token's literal */
            [[nodiscard]] std::string_view literal(void) const noexcept
                { return _data->literal(); }

            /** @brief Prefix Increment operator */
            Iterator &operator++(void) noexcept
            {
                _data = reinterpret_cast<const Token *>(
                    reinterpret_cast<const std::byte *>(_data) + _data->length + sizeof(Token)
                );
                return *this;
            }

            /** @brief Sufix Increment operator */
            Iterator operator++(int) noexcept
            {
                auto copy = *this;
                _data = reinterpret_cast<const Token *>(
                    reinterpret_cast<const std::byte *>(_data) + _data->length + sizeof(Token)
                );
                return copy;
            }

        private:
            const Token *_data { nullptr };
        };
    };

    /** @brief Command set of contexts (class parsing) */
    enum class TokenType : std::uint32_t {
        None,

        // Global scope
        Class,

        // Class scope
        Property,
        Signal,
        Function,
        Event,
        Assignment,

        // Declaration scope
        ParameterList,
        Expression,

        // Expression scope
        List,
        Local,
        Type,
        Statement,
        TemplateType,
        Operator,
        Name,
        Constant,
        LeftParenthesis,
        RightParenthesis
    };

    /** @brief All types of operators */
    enum class OperatorType : std::uint32_t {
        None,

        // Unary
        Not,
        Minus,
        BitReverse,
        Increment,
        Decrement,
        IncrementSuffix,
        DecrementSuffix,

        // Binary
        Addition,
        Substraction,
        Multiplication,
        Division,
        Modulo,
        Equal,
        Different,
        Greater,
        GreaterEqual,
        Lighter,
        LighterEqual,
        And,
        Or,
        BitAnd,
        BitOr,
        BitXor,
        Assign,
        AdditionAssign,
        SubstractionAssign,
        MultiplicationAssign,
        DivisionAssign,
        ModuloAssign,
        BitAndAssign,
        BitOrAssign,
        BitXorAssign,
        Coma,
        Dot,
        Call,

        // Terciary
        TernaryIf,
        TernaryElse
    };

    /** @brief Check if an operator is unary */
    [[nodiscard]] inline bool IsUnary(const OperatorType type) noexcept
    { return static_cast<std::uint32_t>(type) >= static_cast<std::uint32_t>(OperatorType::Not) &&
            static_cast<std::uint32_t>(type) <= static_cast<std::uint32_t>(OperatorType::DecrementSuffix); }

    /** @brief Check if an operator is binary */
    [[nodiscard]] inline bool IsBinary(const OperatorType type) noexcept
    { return static_cast<std::uint32_t>(type) >= static_cast<std::uint32_t>(OperatorType::Addition) &&
            static_cast<std::uint32_t>(type) <= static_cast<std::uint32_t>(OperatorType::Dot); }

    /** @brief Check if an operator is terciary */
    [[nodiscard]] inline bool IsTerciary(const OperatorType type) noexcept
    { return static_cast<std::uint32_t>(type) >= static_cast<std::uint32_t>(OperatorType::TernaryIf) &&
            static_cast<std::uint32_t>(type) <= static_cast<std::uint32_t>(OperatorType::TernaryElse); }


    /** @brief All types of statement operators */
    enum class StatementType : std::uint32_t {
        None,
        If,
        While,
        For,
        Switch,
        Break,
        Continue,
        Return,
        Emit
    };

    /** @brief All types of constants */
    enum class ConstantType : std::uint32_t {
        None,
        Numeric,
        Char,
        Literal
    };

    struct alignas_quarter_cacheline TokenDescriptor
    {
        using Data = union
        {
            OperatorType operatorType;
            StatementType statementType;
        };

        Token::Iterator token {};
        TokenType type { TokenType::None };
        Data data { OperatorType::None };
    };

    /** @brief All types of operator associativity */
    enum class AssociativityType {
        LeftToRight,
        RightToLeft
    };
}