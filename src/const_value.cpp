#include "const_value.hpp"

#include <cfloat>
#include <cmath>
#include <cstdint>

namespace ast {

long WrapToType(long n, const Type& type) {
    uint32_t bits = static_cast<uint32_t>(n);
    switch (type.Size()) {
        case 1: return type.IsSigned() ? static_cast<int8_t>(bits) : static_cast<uint8_t>(bits);
        case 2: return type.IsSigned() ? static_cast<int16_t>(bits) : static_cast<uint16_t>(bits);
        default: return type.IsSigned() ? static_cast<int32_t>(bits) : static_cast<long>(bits);
    }
}

namespace {

// Rounds to float, going to infinity when the value is too large (a plain
// cast would be undefined behaviour in C++).
double RoundToFloat(double x) {
    if (std::isfinite(x) && std::fabs(x) > FLT_MAX) {
        // Halfway between FLT_MAX and 2^128 still rounds down to FLT_MAX.
        bool rounds_down = std::fabs(x) < 3.4028235677973366e38;
        double limit = rounds_down ? FLT_MAX : HUGE_VAL;
        return x < 0 ? -limit : limit;
    }
    return static_cast<double>(static_cast<float>(x));
}

std::optional<ConstValue> ToReal(const ConstValue& v, const Type& to) {
    if (!v.label.empty()) return std::nullopt;
    double x = v.is_real ? v.real : static_cast<double>(v.offset);
    ConstValue out;
    out.is_real = true;
    out.real = to.Size() == 4 ? RoundToFloat(x) : x;
    return out;
}

std::optional<ConstValue> ToInteger(const ConstValue& v, const Type& to) {
    if (!v.label.empty()) {
        if (to.Size() != 4) return std::nullopt;
        return v;
    }
    long n = v.offset;
    if (v.is_real) {
        // Truncate toward zero; anything outside long's range is not a
        // constant we can fold.
        if (!(std::fabs(v.real) < 9.0e18)) return std::nullopt;
        n = static_cast<long>(v.real);
    }
    return ConstValue{"", WrapToType(n, to)};
}

}  // namespace

std::optional<ConstValue> ConvertConst(const ConstValue& value, const Type& to) {
    if (to.IsFloating()) return ToReal(value, to);
    if (to.IsInteger()) return ToInteger(value, to);
    if (value.is_real) return std::nullopt;  // floating to pointer
    return value;
}

std::optional<ConstValue> ConstOperandAs(Context& ctx, const Expr& expr,
                                         const Type& to) {
    std::optional<ConstValue> value = expr.EvalConstValue(ctx);
    if (!value) return std::nullopt;
    return ConvertConst(*value, to);
}

}  // namespace ast
