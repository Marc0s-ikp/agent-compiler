#pragma once
#include <vector>

#include "type.hpp"

// One part of a value that travels by itself: in an integer register, a
// floating register, or one stack word.
struct Piece {
    enum class Where { IntReg, FloatReg, StackWord };
    Where where;
    int index;   // register number, or the stack offset for StackWord
    int offset;  // byte offset inside the struct
    int bytes;   // 1 to 8 for an integer piece (8 for a pointer or word on LP64), 4 or 8 for a floating one
    bool is_signed;
};

// The per-target calling-convention classifier (D25, 21.5). A backend calls
// Classify once for each function it writes (entry and parameter homes) and
// once for each IR 'call' it lowers, with the same function type, so caller
// and callee cannot disagree (D5).
//
// Everything is described with Pieces: a part of a value that
// travels by itself, with its byte offset inside the value, its byte count,
// its C signedness and where it goes:
//   Where::IntReg     the target's integer register number 'index'
//   Where::FloatReg   the target's floating register number 'index'
//   Where::StackWord  'bytes' bytes at byte 'index' of the outgoing argument
//                     area (the caller's sp; the callee finds it at a fixed
//                     offset from its frame, which only the backend knows)
// A narrow integer is one piece of 1 or 2 bytes with 'is_signed' from its C
// type. Whether the backend extends it, and who does, is the target's rule
// (ilp32d: the caller extends to a full word; AAPCS64: the upper bits are
// unspecified, so each side uses only the piece's bytes).
// 'index' of an IntReg or FloatReg piece is the target's n-th integer (or
// floating) argument register, not an architectural register number (on
// RISC-V, index 0 is a0, which is x10). A target may define indices beyond its
// argument registers (AArch64: IntReg 8 is x8, the result pointer). Only the
// backend turns an index into a name.
//
// A float or double is one FloatReg piece, or one IntReg piece (float) / two
// IntReg pieces (double), or IntReg plus StackWord (the a7 split), or one
// StackWord piece of its size. A struct or union (step 6) is: one piece per
// scalar field in the floating convention (FloatReg or IntReg, each with the
// field's offset); up to two integer words (IntReg, IntReg+IntReg, or IntReg
// plus a StackWord of the rest, offset 4); a single StackWord piece as big as
// the whole struct (offset 0) when the a registers are used up; or, over 8
// bytes, by_reference set and one pointer-sized piece (IntReg or StackWord).
// A struct return is the pieces of 'ret' (a0, a1, fa0, fa1) or in_memory.
struct ArgPlace {
    bool by_reference = false;   // pieces hold a pointer to the caller's copy
    std::vector<Piece> pieces;
};

struct RetPlace {
    bool in_memory = false;      // returned through a hidden pointer
    Piece pointer{};             // where the caller passes that pointer (in_memory only)
    std::vector<Piece> pieces;   // where the value comes back; empty for void
};

struct CallLayout {
    std::vector<ArgPlace> args;
    RetPlace ret;
    int stack_bytes = 0;         // size of the outgoing stack argument area
};

// The calling convention of one target. Callers (frontend and backend) reach
// it as Target().Abi().Classify(...); SetTarget picks the object, so several
// targets' ABIs can live in one binary. When a hidden result pointer is used,
// it is part of 'ret', and the argument pieces already account for the
// registers it takes.
class TargetAbi {
public:
    virtual ~TargetAbi() = default;
    virtual CallLayout Classify(const std::vector<TypePtr>& params, const TypePtr& ret) const = 0;
};
