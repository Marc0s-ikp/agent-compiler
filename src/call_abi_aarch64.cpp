#include "call_abi_aarch64.hpp"

#include <stdexcept>
#include <string>

// The AArch64 calling convention (AAPCS64, LP64; D27, 27.2) and nothing else
// knows its rules (D25). Step A3 classifies integers and pointers: x0-x7, then
// 8-byte stack slots in order. Later steps add floats and records (27.2) here,
// behind the same interface.
namespace {

constexpr int kIntArgRegs = 8;     // x0-x7
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

// An integer result comes back in x0 (w0 for up to 4 bytes).
RetPlace ClassifyReturn(const Type& ret) {
    RetPlace place;
    if (ret.IsVoid()) return place;
    if (!IsIntegerOrPointer(ret)) NotYet("return type other than an integer, a pointer or void");
    place.pieces.push_back(ScalarPiece(Piece::Where::IntReg, 0, ret));
    return place;
}

// AAPCS64 stage C for integers: the next x register while NGRN < 8, else the
// next 8-byte stack slot (NSAA, in bytes from the start of the outgoing area).
class ArgAssigner {
public:
    ArgPlace Place(const Type& type) {
        if (!IsIntegerOrPointer(type)) NotYet("parameter type other than an integer or a pointer");
        ArgPlace place;
        if (next_int_reg_ < kIntArgRegs) {
            place.pieces.push_back(ScalarPiece(Piece::Where::IntReg, next_int_reg_++, type));
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
