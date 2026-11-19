#include "call_abi_aarch64.hpp"

#include <stdexcept>
#include <string>

// The AArch64 calling convention (AAPCS64, LP64; D27, 27.2) and nothing else
// knows its rules (D25). Steps A3 and A5 classify scalars: integers and
// pointers in x0-x7, floats and doubles in v0-v7 (s and d), the two counts
// independent, and then 8-byte stack slots in order for both. Step A6 adds
// records (27.2) here, behind the same interface.
namespace {

constexpr int kIntArgRegs = 8;     // x0-x7
constexpr int kFloatArgRegs = 8;   // v0-v7
constexpr int kStackSlotBytes = 8;

[[noreturn]] void NotYet(const std::string& what) {
    throw std::runtime_error("not in AArch64 yet: " + what);
}

bool IsIntegerOrPointer(const Type& type) { return type.IsInteger() || type.IsPointer(); }

// An integer or pointer value is one piece of its own size. The upper bits of
// a narrow one are unspecified (27.2), so the piece only names the byte count;
// is_signed is the C type's, for the backend's use.
Piece ScalarPiece(Piece::Where where, int index, const Type& type) {
    return Piece{where, index, 0, type.Size(), type.IsInteger() && type.IsSigned()};
}

bool IsScalar(const Type& type) { return IsIntegerOrPointer(type) || type.IsFloating(); }

// An integer result comes back in x0 (w0 for up to 4 bytes), a float in s0 and
// a double in d0.
RetPlace ClassifyReturn(const Type& ret) {
    RetPlace place;
    if (ret.IsVoid()) return place;
    if (!IsScalar(ret)) NotYet("return type other than a scalar or void");
    Piece::Where where = ret.IsFloating() ? Piece::Where::FloatReg : Piece::Where::IntReg;
    place.pieces.push_back(ScalarPiece(where, 0, ret));
    return place;
}

// AAPCS64 stage C for scalars. An integer or pointer: the next x register while
// NGRN < 8. A float or double: the next v register while NSRN < 8 (C.1). Else
// (C.5, C.6, C.14 to C.17) the next 8-byte stack slot, a float in its low 4
// bytes (NSAA, in bytes from the start of the outgoing area), shared by both
// classes in argument order.
class ArgAssigner {
public:
    ArgPlace Place(const Type& type) {
        if (!IsScalar(type)) NotYet("parameter type other than a scalar");
        bool floating = type.IsFloating();
        int& next_reg = floating ? next_float_reg_ : next_int_reg_;
        ArgPlace place;
        if (next_reg < (floating ? kFloatArgRegs : kIntArgRegs)) {
            Piece::Where where = floating ? Piece::Where::FloatReg : Piece::Where::IntReg;
            place.pieces.push_back(ScalarPiece(where, next_reg++, type));
        } else {
            place.pieces.push_back(ScalarPiece(Piece::Where::StackWord, stack_offset_, type));
            stack_offset_ += kStackSlotBytes;
        }
        return place;
    }

    // The outgoing area, rounded up to the 16-byte stack alignment.
    int StackBytes() const { return (stack_offset_ + 15) / 16 * 16; }

private:
    int next_int_reg_ = 0;
    int next_float_reg_ = 0;
    int stack_offset_ = 0;
};

}  // namespace

CallLayout AArch64Abi::Classify(const std::vector<TypePtr>& params, const TypePtr& ret) const {
    CallLayout layout;
    ArgAssigner assigner;
    for (const TypePtr& param : params) layout.args.push_back(assigner.Place(*param));
    layout.ret = ClassifyReturn(*ret);
    layout.stack_bytes = assigner.StackBytes();
    return layout;
}
