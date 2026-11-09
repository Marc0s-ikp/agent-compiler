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

// The register an expression of this type leaves its value in (design
// section 5): fa0 for float and double, a0 for everything else.
std::string AccReg(const Type& type);
// The scratch register for a second operand: ft0 for floating types, else t0.
std::string ScratchReg(const Type& type);
// Copies the accumulator to the scratch register.
void EmitMoveToScratch(std::ostream& os, const Type& type);

// For a floating type 'reg' must be a floating register; otherwise an
// integer register.
void EmitLoad(std::ostream& os, const Type& type, const std::string& reg,
              const std::string& base, int off);
void EmitStore(std::ostream& os, const Type& type, const std::string& reg,
               const std::string& base, int off);

// dst = base + off, for any offset.
void EmitAddrOffset(std::ostream& os, const std::string& dst,
                    const std::string& base, int off);

// Sets 'bytes' bytes at off(base) to zero, for any size and offset: a few
// stores for a small object, a loop (clobbering t0 and t1) for a big one.
void EmitZeroMem(std::ostream& os, const std::string& base, int off, int bytes);

// Branch on the truthiness of a0 (an integer: nonzero is true). Uses the
// inverted short branch over an unconditional 'j', so the target may be any
// distance away (a conditional branch only reaches about 4KB).
void EmitBranchIfZero(std::ostream& os, const std::string& label);
void EmitBranchIfNonZero(std::ostream& os, const std::string& label);
void EmitJump(std::ostream& os, const std::string& label);

// Leaves 1 in a0 if the floating value in fa0 is nonzero, else 0. -0.0 is
// false and NaN is true, because the test is "not equal to zero".
void EmitFloatTruth(std::ostream& os, const Type& type);

// Convert the value in the accumulator (AccReg) from one type to another.
// Both must be scalar (a source array must be decayed by the caller first),
// or both void. A pointer cannot convert to or from a floating type.
void EmitConvert(std::ostream& os, const Type& from, const Type& to);
