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
struct ArgLoc {
    enum class Kind { IntReg, FloatReg, IntPair, Split, Stack };
    Kind kind;
    int reg;
    int offset;
};

std::vector<ArgLoc> ClassifyArgs(const std::vector<TypePtr>& param_types);

// Bytes of outgoing stack area that argument 'type' at 'loc' occupies,
// counted from the start of the area (0 if it is wholly in registers).
int StackExtent(const ArgLoc& loc, const Type& type);
