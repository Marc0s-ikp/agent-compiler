#pragma once
#include <vector>

#include "abi.hpp"
#include "type.hpp"

// The per-target calling-convention classifier (D25, 21.5). A backend calls
// Classify once for each function it writes (entry and parameter homes) and
// once for each IR 'call' it lowers, with the same function type, so caller
// and callee cannot disagree (D5).
//
// Everything is described with Pieces (abi.hpp): a part of a value that
// travels by itself, with its byte offset inside the value, its byte count,
// its C signedness and where it goes:
//   Where::IntReg     integer argument register number 'index'
//   Where::FloatReg   floating argument register number 'index'
//   Where::StackWord  'bytes' bytes at byte 'index' of the outgoing argument
//                     area (the caller's sp; the callee's s0 on RISC-V)
// A narrow integer is one piece of 1 or 2 bytes; the backend extends it to a
// full word by 'is_signed' (the psABI says so for registers and stack slots).
// A register NUMBER is an index into the target's argument registers; only the
// backend turns it into a name.
//
// Today's classifier covers integers and pointers; any other type throws "not
// in IR yet". Steps 5 and 6 lift that throw, and change nothing here: a double
// is one 8-byte FloatReg piece, or two IntReg pieces, or an IntReg piece plus
// a StackWord piece (the split case); a float-convention struct is one piece
// per scalar field; a struct over 8 bytes has by_reference set and its pieces
// describe the pointer.
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
