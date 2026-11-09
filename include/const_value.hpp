#pragma once
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

// An integer constant of integer type 'type': 'n' wrapped to its width.
long WrapToType(long n, const Type& type);

}  // namespace ast
