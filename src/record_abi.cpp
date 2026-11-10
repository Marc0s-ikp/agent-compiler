#include "record_abi.hpp"

#include "emit.hpp"

namespace {

std::string IntReg(int n) { return "a" + std::to_string(n); }
std::string FloatReg(int n) { return "fa" + std::to_string(n); }

// The scalar type of a piece that travels in a floating register.
TypePtr FloatPieceType(const Piece& p) { return Type::Floating(p.bytes); }

// Loads one register piece of the struct at base + off into its register.
void LoadPiece(std::ostream& os, const Piece& p, const Type& record,
               const std::string& base, int off) {
    int at = off + p.offset;
    if (p.where == Piece::Where::FloatReg) {
        return EmitLoad(os, *FloatPieceType(p), FloatReg(p.index), base, at);
    }
    EmitLoadBytes(os, IntReg(p.index), p.bytes, p.is_signed, record.Align(), base, at);
}

// Stores one register piece into the struct at base + off.
void StorePiece(std::ostream& os, const Piece& p, const Type& record,
                const std::string& base, int off) {
    int at = off + p.offset;
    if (p.where == Piece::Where::FloatReg) {
        return EmitStore(os, *FloatPieceType(p), FloatReg(p.index), base, at);
    }
    EmitStoreBytes(os, IntReg(p.index), p.bytes, record.Align(), base, at);
}

}  // namespace

void EmitPlaceRecordArg(std::ostream& os, const ArgLoc& loc, const Type& record,
                        int slot) {
    TypePtr word = Type::Int();
    if (loc.by_reference) {
        if (loc.kind == ArgLoc::Kind::IntReg) {
            return EmitAddrOffset(os, IntReg(loc.reg), "s0", slot);
        }
        EmitAddrOffset(os, "t1", "s0", slot);
        return EmitStore(os, *word, "t1", "sp", loc.offset);
    }
    if (loc.kind == ArgLoc::Kind::Stack) {
        EmitAddrOffset(os, "t1", "sp", loc.offset);
        EmitAddrOffset(os, "t2", "s0", slot);
        return EmitCopyMem(os, "t1", "t2", record.Size(), record.Align());
    }
    for (const Piece& p : ArgPieces(loc, record)) {
        if (p.where != Piece::Where::StackWord) {
            LoadPiece(os, p, record, "s0", slot);
            continue;
        }
        EmitLoadBytes(os, "t1", p.bytes, false, record.Align(), "s0", slot + p.offset);
        EmitStore(os, *word, "t1", "sp", p.index);
    }
}

void EmitStoreRecordParam(std::ostream& os, const ArgLoc& loc,
                          const Type& record, int home) {
    TypePtr word = Type::Int();
    if (loc.by_reference) {
        if (loc.kind == ArgLoc::Kind::IntReg) {
            os << "\tmv t1, " << IntReg(loc.reg) << "\n";
        } else {
            EmitLoad(os, *word, "t1", "s0", loc.offset);
        }
        EmitAddrOffset(os, "t2", "s0", home);
        return EmitCopyMem(os, "t2", "t1", record.Size(), record.Align());
    }
    for (const Piece& p : ArgPieces(loc, record)) {
        if (p.where != Piece::Where::StackWord) {
            StorePiece(os, p, record, "s0", home);
            continue;
        }
        EmitLoad(os, *word, "t1", "s0", p.index);
        EmitStoreBytes(os, "t1", p.bytes, record.Align(), "s0", home + p.offset);
    }
}

void EmitReturnRecord(std::ostream& os, const Context& ctx, const Type& record) {
    RetLoc ret = ClassifyReturn(record);
    if (ret.hidden_pointer) {
        EmitLoad(os, *Type::Int(), "t1", "s0", ctx.ReturnPointerSlot());
        EmitCopyMem(os, "t1", "a0", record.Size(), record.Align());
        os << "\tmv a0, t1\n";
        return;
    }
    os << "\tmv t1, a0\n";
    for (const Piece& p : ret.pieces) LoadPiece(os, p, record, "t1", 0);
}

void EmitSaveReturnedRecord(std::ostream& os, const Type& record, int result) {
    RetLoc ret = ClassifyReturn(record);
    if (!ret.hidden_pointer) {
        for (const Piece& p : ret.pieces) StorePiece(os, p, record, "s0", result);
    }
    EmitAddrOffset(os, "a0", "s0", result);
}
