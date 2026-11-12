#pragma once
#include <cstdint>
#include <optional>

#include "ast_node.hpp"

namespace ast {

// The compile-time twin of EmitConvert (D6, D18): converts a constant to type
// 'to' with the same C rules (truncation toward zero, wrapping
// to the width of an integer type, rounding to float). An address keeps its
// label when it stays 4 bytes wide. Returns nullopt if the value cannot be
// represented as a constant of that type (an address as a float, or a
// floating value too large for an integer).
std::optional<ConstValue> ConvertConst(const ConstValue& value, const Type& to);

// Evaluates 'expr' as a constant and converts it to 'to'; nullopt if it is
// not constant or does not convert.
std::optional<ConstValue> ConstOperandAs(Context& ctx, const Expr& expr,
                                         const Type& to);

// The IEEE bit pattern of 'value' as a float (size 4, rounded) or double
// (size 8): what the float pool and data entries store.
uint64_t FloatBits(double value, int size);

// An integer constant of integer type 'type': 'n' wrapped to its width.
long WrapToType(long n, const Type& type);

}  // namespace ast
