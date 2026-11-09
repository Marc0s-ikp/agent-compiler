#pragma once
#include <ostream>

#include "init_layout.hpp"

namespace ast {

// Runs a laid-out initialiser for the local object at s0 + offset: an array
// is zeroed first, so elements the initialiser leaves out read as zero, then
// each item is evaluated and stored. Unlike a global's, the values need not
// be constants.
void EmitLocalInit(std::ostream& os, Context& ctx, const FlatInit& flat,
                   int offset);

}  // namespace ast
