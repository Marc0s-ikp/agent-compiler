#pragma once
#include <vector>

#include "type.hpp"

// Where one argument lives under the ilp32d calling convention.
// Shared by the callee (FunctionDefinition) and the caller (FunctionCall),
// so the two sides cannot disagree (D5).
//   IntReg    a<reg>: an integer, a pointer, or a float whose fa registers
//             ran out (it travels as its 32 bits)
//   FloatReg  fa<reg>: a float or double
//   IntPair   a<reg> (low word) and a<reg+1> (high word): a double whose fa
//             registers ran out
//   Split     a7 (low word) and the stack word at 'offset' (high word): a
//             double when only a7 is left
//   Stack     'offset' bytes above the caller's sp (= s0 in the callee)
//   Fields    a struct in the floating convention: each flattened scalar
//             field has its own register in 'fields' (D19)
// A struct or union uses the same kinds as a double does (IntReg for up to 4
// bytes, IntPair, Split or Stack for 8) when it is passed in integer words.
// A struct over 8 bytes is passed by reference (D19): 'by_reference' is set
// and IntReg or Stack describes the one-word pointer to the caller's copy.
struct FieldLoc {
    bool is_float;
    int reg;     // fa or a register number
    int offset;  // byte offset of the field inside the struct
    TypePtr type;
};

struct ArgLoc {
    enum class Kind { IntReg, FloatReg, IntPair, Split, Stack, Fields };
    ArgLoc(Kind k, int r, int o) : kind(k), reg(r), offset(o) {}
    Kind kind;
    int reg;
    int offset;
    bool by_reference = false;
    std::vector<FieldLoc> fields;  // Kind::Fields only
};

// 'hidden_return' is true if the callee returns a struct through a hidden
// pointer, which takes a0 and moves every integer register up by one.
std::vector<ArgLoc> ClassifyArgs(const std::vector<TypePtr>& param_types,
                                 bool hidden_return = false);

// One part of a struct value that travels by itself: in an integer register,
// a floating register, or (the second half of a Split) one stack word.
struct Piece {
    enum class Where { IntReg, FloatReg, StackWord };
    Where where;
    int index;   // register number, or the stack offset for StackWord
    int offset;  // byte offset inside the struct
    int bytes;   // 1 to 4 for an integer piece, 4 or 8 for a floating one
    bool is_signed;
};

// The pieces of a struct passed in registers (any kind except Stack and
// by-reference). The caller loads each from the struct; the callee stores
// each into the parameter's home.
std::vector<Piece> ArgPieces(const ArgLoc& loc, const Type& record);

// How a struct is returned: through a hidden pointer, or in 'pieces' (a0, a1,
// fa0, fa1).
struct RetLoc {
    bool hidden_pointer;
    std::vector<Piece> pieces;
};
RetLoc ClassifyReturn(const Type& record);
// True if a function returning 'type' takes the hidden result pointer.
bool ReturnsThroughPointer(const Type& type);

// Bytes of outgoing stack area that argument 'type' at 'loc' occupies,
// counted from the start of the area (0 if it is wholly in registers).
int StackExtent(const ArgLoc& loc, const Type& type);
