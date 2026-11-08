#pragma once
#include <ostream>
#include <string>

#include "type.hpp"

// The only places that choose load/store instructions, build addresses and
// convert between types (design section 2). Nodes never print lw/sw directly.

// 'op reg, off(base)'. If off does not fit 12 bits, the address is built in
// t6 first (design section 4).
void EmitMem(std::ostream& os, const std::string& op, const std::string& reg,
             const std::string& base, int off);

void EmitLoad(std::ostream& os, const Type& type, const std::string& reg,
              const std::string& base, int off);
void EmitStore(std::ostream& os, const Type& type, const std::string& reg,
               const std::string& base, int off);

// dst = base + off, for any offset.
void EmitAddrOffset(std::ostream& os, const std::string& dst,
                    const std::string& base, int off);

// Branch on the truthiness of a0 (an integer: nonzero is true). Uses the
// inverted short branch over an unconditional 'j', so the target may be any
// distance away (a conditional branch only reaches about 4KB).
void EmitBranchIfZero(std::ostream& os, const std::string& label);
void EmitBranchIfNonZero(std::ostream& os, const std::string& label);
void EmitJump(std::ostream& os, const std::string& label);

// Convert the value in a0 from one type to another.
void EmitConvert(std::ostream& os, const Type& from, const Type& to);
