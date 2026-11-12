#include "call_abi_riscv.hpp"

#include <algorithm>
#include <stdexcept>

// The RISC-V (ilp32d) classifier: a thin layer over ClassifyArgs and
// ClassifyReturn (abi.cpp), which hold the rules of D5, D18(d) and D19(d).
namespace {

[[noreturn]] void NotYet() { throw std::runtime_error("not in IR yet: this type in a call"); }

bool IsPassable(const Type& type) {
    return (type.IsInteger() && type.Size() <= 4) || type.IsPointer() || type.IsFloating() ||
           type.IsRecord();
}

Piece ScalarPiece(Piece::Where where, int index, const Type& type) {
    return Piece{where, index, 0, type.Size(), type.IsInteger() && type.IsSigned()};
}

// A float or double: in an fa register as one piece; otherwise what abi.cpp's
// ArgPieces says (one IntReg piece for a float, two for a double in a register
// pair, IntReg plus StackWord for the a7 split), or one stack piece of the
// value's size (8 bytes for a double).
ArgPlace PlaceFloating(const ArgLoc& loc, const Type& type) {
    ArgPlace place;
    switch (loc.kind) {
        case ArgLoc::Kind::FloatReg:
            place.pieces.push_back(ScalarPiece(Piece::Where::FloatReg, loc.reg, type));
            break;
        case ArgLoc::Kind::Stack:
            place.pieces.push_back(ScalarPiece(Piece::Where::StackWord, loc.offset, type));
            break;
        default: place.pieces = ArgPieces(loc, type);
    }
    return place;
}

// The one-word pointer to the caller's copy of a struct over 8 bytes.
Piece ReferencePiece(const ArgLoc& loc) {
    Piece::Where where = loc.kind == ArgLoc::Kind::IntReg ? Piece::Where::IntReg : Piece::Where::StackWord;
    int index = loc.kind == ArgLoc::Kind::IntReg ? loc.reg : loc.offset;
    return Piece{where, index, 0, 4, false};
}

// A struct or union. By reference: the pieces describe the pointer. On the
// stack: one StackWord piece as big as the record (offset 0). Otherwise
// abi.cpp's ArgPieces gives the integer words (IntReg, IntPair, the a7 Split)
// or one piece per field (the floating convention, 'Fields'), each with its
// byte offset inside the record.
ArgPlace PlaceRecord(const ArgLoc& loc, const Type& type) {
    ArgPlace place;
    place.by_reference = loc.by_reference;
    if (loc.by_reference) {
        place.pieces.push_back(ReferencePiece(loc));
    } else if (loc.kind == ArgLoc::Kind::Stack) {
        place.pieces.push_back(Piece{Piece::Where::StackWord, loc.offset, 0, type.Size(), false});
    } else {
        place.pieces = ArgPieces(loc, type);
    }
    return place;
}

ArgPlace PlaceArg(const ArgLoc& loc, const Type& type) {
    if (!IsPassable(type)) NotYet();
    if (type.IsRecord()) return PlaceRecord(loc, type);
    if (type.IsFloating()) return PlaceFloating(loc, type);
    ArgPlace place;
    if (loc.kind == ArgLoc::Kind::IntReg) {
        place.pieces.push_back(ScalarPiece(Piece::Where::IntReg, loc.reg, type));
    } else {
        place.pieces.push_back(ScalarPiece(Piece::Where::StackWord, loc.offset, type));
    }
    return place;
}

// A struct or union comes back in the pieces ClassifyReturn gives (a0, a1,
// fa0, fa1), or through a hidden pointer that the caller passes in a0.
RetPlace PlaceRecordReturn(const Type& type) {
    RetPlace ret;
    RetLoc loc = ClassifyReturn(type);
    ret.in_memory = loc.hidden_pointer;
    if (ret.in_memory) {
        ret.pointer = Piece{Piece::Where::IntReg, 0, 0, 4, false};
    } else {
        ret.pieces = loc.pieces;
    }
    return ret;
}

RetPlace PlaceReturn(const Type& type) {
    RetPlace ret;
    if (type.IsVoid()) return ret;
    if (!IsPassable(type)) NotYet();
    if (type.IsRecord()) return PlaceRecordReturn(type);
    Piece::Where where = type.IsFloating() ? Piece::Where::FloatReg : Piece::Where::IntReg;
    ret.pieces.push_back(ScalarPiece(where, 0, type));
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
