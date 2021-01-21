/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Specialized Nodes
 */

#pragma once

#include "Instruction.hpp"

namespace kF::Lang
{
    template<InstructionType type>
    struct UnaryInstruction;

    template<InstructionType type>
    struct BinaryInstruction;
}

#include "Instructions.ipp"