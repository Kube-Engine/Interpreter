/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Instruction
 */

#pragma once

#include "Base.hpp"

namespace kF::Lang
{
    struct Instruction;
}

struct alignas_eighth_cacheline kF::Lang::Instruction
{
    InstructionType type;
    FileIndex file;
    Token token;
};

static_assert_fit_eighth_cacheline(kF::Lang::Instruction);