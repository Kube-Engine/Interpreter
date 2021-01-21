/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Base
 */

#pragma once

#include <Kube/Core/Utils.hpp>

namespace kF::Lang
{
    using ByteIndex = std::uint32_t;
    using FileIndex = std::uint16_t;
    using Token = std::uint32_t;

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
        Constant
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