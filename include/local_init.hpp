#pragma once

#include "init_layout.hpp"
#include "ir_gen.hpp"

namespace ast {

// Initialises the frame object 'object' from a laid-out initialiser. An array
// is zeroed first (memzero), then each scalar item is evaluated, converted to its type
// and stored at its byte offset; string bytes are stored one by one.
void EmitLocalInit(IRGen& gen, const FlatInit& flat, const ir::Operand& object);

}  // namespace ast
