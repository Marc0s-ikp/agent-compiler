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

// Copies a struct or union of 'bytes' bytes and alignment 'align' from the
// address in 'src' to the address in 'dst' (D19). Handles any size; a large
// one uses a loop. Clobbers t0, t3, t4 and t5, so 'dst' and 'src' must be
// other registers; both stay unchanged.
void EmitCopyMem(std::ostream& os, const std::string& dst,
                 const std::string& src, int bytes, int align);

// Loads 'bytes' (1 to 4) bytes of an integer piece of a struct, zero- or
// sign-extended, into 'reg' from off(base). A whole aligned word is one 'lw';
// anything else is built byte by byte, so it never reads past the object or
// reads a misaligned word. 'align' is the alignment of the struct. Clobbers
// t0; 'reg' and 'base' must not be t0.
void EmitLoadBytes(std::ostream& os, const std::string& reg, int bytes,
                   bool is_signed, int align, const std::string& base, int off);
// The matching store of the low 'bytes' bytes of 'reg'. Clobbers t0; 'reg'
// and 'base' must not be t0.
void EmitStoreBytes(std::ostream& os, const std::string& reg, int bytes,
                    int align, const std::string& base, int off);
