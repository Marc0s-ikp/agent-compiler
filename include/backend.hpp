#pragma once
#include <ostream>

#include "ir.hpp"

// The RISC-V backend (D21, 21.6). Takes a checked module and writes assembly:
// each function with its frame, prologue and epilogue, then the data entries.
void EmitRiscv(const ir::Module& module, std::ostream& os);

// The AArch64 backend (D27, 27.3): the same job for AAPCS64 and LP64.
void EmitAArch64(const ir::Module& module, std::ostream& os);
