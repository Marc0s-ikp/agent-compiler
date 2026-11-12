#pragma once
#include <ostream>

#include "init_layout.hpp"
#include "ir_gen.hpp"

namespace ast {

// Runs a laid-out initialiser for the local object at s0 + offset: an array
// is zeroed first, so elements the initialiser leaves out read as zero, then
// each item is evaluated and stored. Unlike a global's, the values need not
// be constants.
void EmitLocalInit(std::ostream& os, Context& ctx, const FlatInit& flat,
                   int offset);

// The --ir twin: the object is the frame object 'object'. An array is zeroed
// first (memzero), then each scalar item is evaluated, converted to its type
// and stored at its byte offset; string bytes are stored one by one.
void EmitLocalInitIR(IRGen& gen, const FlatInit& flat, const ir::Operand& object);

}  // namespace ast
