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
    using FileIndex = std::uint16_t;
    using LineIndex = std::uint16_t;
    using ColumnIndex = std::uint16_t;

    struct Token
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
        // Class level
        Import,
        Class,
        Property,
        Function,
        Event,
        Assignment,

        // Expression level
        Expression,
        UnaryOperator,
        BinaryOperator,
        Statement,
        Local,
        Name,
        Call,
        Emit,
        Constant
    };

    /** @brief All types of unary operators */
    enum class UnaryType : std::uint32_t {
        None,
        Minus
    };

    /** @brief All types of binary operators */
    enum class BinaryType : std::uint32_t {
        None,
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
        LighterEqual
    };

    /** @brief All types of assignment operators */
    enum class AssignmentType : std::uint32_t {
        None,
        Assign,
        Addition,
        Substraction,
        Multiplication,
        Division,
        Modulo
    };

    /** @brief All types of statement operators */
    enum class StatementType : std::uint32_t {
        None,
        If,
        While,
        For,
        Switch
    };

    struct alignas_quarter_cacheline TokenDescriptor
    {
        using Data = union
        {
            UnaryType unaryType;
            BinaryType binaryType;
            AssignmentType assignmentType;
            StatementType statementType;
        };

        Token::Iterator token {};
        TokenType type { TokenType::None };
        Data data { UnaryType::None };
    };

    /** @brief Instruction set of expressions */
    enum class InstructionType : std::uint32_t {
        // Unary operators
        UnaryMinus,

        // Binary operators
        BinaryAddition,
        BinarySubstraction,
        BinaryMultiplication,
        BinaryDivision,
        BinaryModulo,

        // Assignment operators
        AssignmentAddition,
        AssignmentSubstraction,
        AssignmentMultiplication,
        AssignmentDivision,
        AssignmentModulo,

        // Variables
        Local,
        LValue,
        RValue,
        LValueLocal,
        RValueLocal,
        Constant,

        // Variable access
        Access,
        Derefence,
        DereferenceAccess,

        // Functions
        Call,

        // Signals
        Emit,

        // Properties
        Get,
        Set,

        // Statements
        Branch,
        Switch,
        While,
        For,
        ForEach
    };
}