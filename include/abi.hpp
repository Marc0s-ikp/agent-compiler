#pragma once
#include <vector>

#include "type.hpp"

// Where one argument lives under the ilp32d calling convention.
// Shared by the callee (FunctionDefinition) and, from feature 3, the caller,
// so the two sides cannot disagree (D5). Floating rows are added with floats.
struct ArgLoc {
    enum class Kind { IntReg, Stack };
    Kind kind;
    int reg;     // IntReg: the number n of a<n>
    int offset;  // Stack: bytes above the caller's sp (= s0 in the callee)
};

std::vector<ArgLoc> ClassifyArgs(const std::vector<TypePtr>& param_types);
