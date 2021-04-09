/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Base
 */

#pragma once

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

        /** @brief Comparison opeartor */
        [[nodiscard]] bool operator==(const Token &other) const noexcept
            { return file == other.file && line == other.line && column == other.column && length == other.length; }
    };

    /** @brief Command set of contexts (class parsing) */
    enum class TokenType : std::uint16_t {
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
        Constant,
        ErrorToken
    };

    /** @brief Instruction set of expressions */
    enum class InstructionType : std::uint16_t {
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