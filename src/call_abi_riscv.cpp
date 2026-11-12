#include "call_abi_riscv.hpp"

#include <algorithm>
#include <stdexcept>

// The RISC-V (ilp32d) classifier: a thin layer over ClassifyArgs and
// ClassifyReturn (abi.cpp), which hold the rules of D5, D18(d) and D19(d).
// The old EmitRISC path calls those same functions, so there is one rule set.
namespace {

[[noreturn]] void NotYet() { throw std::runtime_error("not in IR yet: this type in a call"); }

// The only types the IR path passes so far.
bool IsIntOrPointer(const Type& type) {
    return (type.IsInteger() && type.Size() <= 4) || type.IsPointer();
}

Piece ScalarPiece(Piece::Where where, int index, const Type& type) {
    return Piece{where, index, 0, type.Size(), type.IsInteger() && type.IsSigned()};
}

ArgPlace PlaceArg(const ArgLoc& loc, const Type& type) {
    if (!IsIntOrPointer(type)) NotYet();
    ArgPlace place;
    if (loc.kind == ArgLoc::Kind::IntReg) {
        place.pieces.push_back(ScalarPiece(Piece::Where::IntReg, loc.reg, type));
    } else {
        place.pieces.push_back(ScalarPiece(Piece::Where::StackWord, loc.offset, type));
    }
    return place;
}

RetPlace PlaceReturn(const Type& type) {
    RetPlace ret;
    if (type.IsVoid()) return ret;
    if (!IsIntOrPointer(type)) NotYet();
    ret.pieces.push_back(ScalarPiece(Piece::Where::IntReg, 0, type));
    return ret;
}

}  // namespace

CallLayout RiscV32Abi::Classify(const std::vector<TypePtr>& params, const TypePtr& ret) const {
    CallLayout layout;
    layout.ret = PlaceReturn(*ret);
    std::vector<ArgLoc> locs = ClassifyArgs(params, layout.ret.in_memory);
    for (size_t i = 0; i < params.size(); ++i) {
        layout.args.push_back(PlaceArg(locs[i], *params[i]));
        layout.stack_bytes = std::max(layout.stack_bytes, StackExtent(locs[i], *params[i]));
    }
    return layout;
}
