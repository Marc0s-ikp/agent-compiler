#include "call_abi_aarch64.hpp"

#include <stdexcept>

// The AArch64 calling convention (AAPCS64, LP64; D27, 27.2) and nothing else
// knows its rules (D25). Step A1 classifies only what A1 can generate: a
// function with no parameters that returns an integer or nothing. Later steps
// add the rest of 27.2 here, behind the same interface.
namespace {

[[noreturn]] void NotYet(const std::string& what) {
    throw std::runtime_error("not in AArch64 yet: " + what);
}

// An integer result comes back in x0 (w0 for up to 4 bytes). The upper bits of
// a narrow result are unspecified, so the piece only names the byte count.
RetPlace ClassifyReturn(const Type& ret) {
    RetPlace place;
    if (ret.IsVoid()) return place;
    if (!ret.IsInteger()) NotYet("return type other than an integer or void");
    place.pieces.push_back(Piece{Piece::Where::IntReg, 0, 0, ret.Size(), ret.IsSigned()});
    return place;
}

}  // namespace

CallLayout AArch64Abi::Classify(const std::vector<TypePtr>& params, const TypePtr& ret) const {
    if (!params.empty()) NotYet("function parameters");
    CallLayout layout;
    layout.ret = ClassifyReturn(*ret);
    return layout;
}
