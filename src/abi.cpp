#include "abi.hpp"

namespace {

// The scalar fields of a struct for the floating convention, in memory order,
// or false if it does not qualify. Nested structs and arrays are flattened.
// A union, or any field that is not a float, double or integer of at most 4
// bytes (a pointer included, as GCC has it), disqualifies the struct.
bool Flatten(const Type& type, int base, std::vector<FieldLoc>& out) {
    if (out.size() > 2) return false;
    if (type.IsArray()) {
        for (int i = 0; i < type.Length(); ++i) {
            if (!Flatten(*type.Base(), base + i * type.Base()->Size(), out)) {
                return false;
            }
        }
        return true;
    }
    if (type.IsRecord()) {
        if (type.IsUnion()) return false;
        for (const RecordMember& m : type.Members()) {
            if (!Flatten(*m.type, base + m.offset, out)) return false;
        }
        return true;
    }
    bool real = type.IsFloating();
    if (!real && !(type.IsInteger() && type.Size() <= 4)) return false;
    // The type is a shared pointer in the result; rebuild an equal one.
    TypePtr copy = real ? Type::Floating(type.Size())
                        : Type::Integer(type.Size(), type.IsSigned());
    out.push_back(FieldLoc{real, 0, base, copy});
    return out.size() <= 2;
}

// The fields of a struct that travels in the floating convention: one or two
// scalars, at least one floating. Empty if it does not qualify. The 'reg'
// members are not filled in.
std::vector<FieldLoc> FloatConventionFields(const Type& type) {
    std::vector<FieldLoc> fields;
    if (!type.IsRecord() || !Flatten(type, 0, fields)) return {};
    bool any_float = false;
    for (const FieldLoc& f : fields) any_float = any_float || f.is_float;
    return any_float ? fields : std::vector<FieldLoc>{};
}

const int kIntArgRegs = 8;
const int kFloatArgRegs = 8;

int RoundUp(int value, int multiple) {
    return (value + multiple - 1) / multiple * multiple;
}

// The running state of the classifier: the next free register of each file
// and the next free stack byte.
struct Cursor {
    int int_reg = 0;
    int float_reg = 0;
    int stack = 0;

    // A struct or union (D19). In the floating convention only if every
    // register it needs is free; otherwise the integer convention entirely:
    // up to 8 bytes in a registers, more by reference to the caller's copy.
    ArgLoc TakeRecord(const Type& type) {
        std::vector<FieldLoc> fields = FloatConventionFields(type);
        if (!fields.empty()) {
            ArgLoc loc = TakeFields(fields);
            if (!loc.fields.empty()) return loc;
        }
        if (type.Size() > 8) return TakeReference();
        if (type.Size() <= 4) {
            if (int_reg < kIntArgRegs) return {ArgLoc::Kind::IntReg, int_reg++, 0};
            return TakeStack(type);
        }
        return TakeWordPair(type);
    }

    // Floating fields go in the next fa registers, integer fields in the next
    // a registers. An empty result means a register file is too full.
    ArgLoc TakeFields(std::vector<FieldLoc> fields) {
        int floats = 0;
        for (const FieldLoc& f : fields) floats += f.is_float ? 1 : 0;
        int ints = static_cast<int>(fields.size()) - floats;
        ArgLoc loc{ArgLoc::Kind::Fields, 0, 0};
        if (float_reg + floats > kFloatArgRegs || int_reg + ints > kIntArgRegs) {
            return loc;
        }
        for (FieldLoc& f : fields) f.reg = f.is_float ? float_reg++ : int_reg++;
        loc.fields = std::move(fields);
        return loc;
    }

    // A pointer to the caller's copy: a register, else a stack word.
    ArgLoc TakeReference() {
        ArgLoc loc = int_reg < kIntArgRegs
                         ? ArgLoc{ArgLoc::Kind::IntReg, int_reg++, 0}
                         : TakeStack(*Type::Int());
        loc.by_reference = true;
        return loc;
    }

    // 5 to 8 bytes as two words, like a double.
    ArgLoc TakeWordPair(const Type& type) { return TakeDoubleAsInts(type); }

    ArgLoc TakeStack(const Type& type) {
        stack = RoundUp(stack, type.Align());
        ArgLoc loc{ArgLoc::Kind::Stack, 0, stack};
        stack += RoundUp(type.Size(), 4);
        return loc;
    }

    // A double with no fa register left: two a registers if there are two,
    // the last a register plus a stack word if one, else the stack.
    ArgLoc TakeDoubleAsInts(const Type& type) {
        int left = kIntArgRegs - int_reg;
        if (left >= 2) {
            ArgLoc loc{ArgLoc::Kind::IntPair, int_reg, 0};
            int_reg += 2;
            return loc;
        }
        if (left == 1) {
            ArgLoc loc{ArgLoc::Kind::Split, int_reg++, stack};
            stack += 4;
            return loc;
        }
        return TakeStack(type);
    }

    ArgLoc Take(const Type& type) {
        if (type.IsRecord()) return TakeRecord(type);
        if (type.IsFloating() && float_reg < kFloatArgRegs) {
            return {ArgLoc::Kind::FloatReg, float_reg++, 0};
        }
        if (type.IsFloating() && type.Size() == 8) return TakeDoubleAsInts(type);
        if (int_reg < kIntArgRegs) return {ArgLoc::Kind::IntReg, int_reg++, 0};
        return TakeStack(type);
    }
};
}  // namespace

// Floats and doubles use fa0-fa7 first. Everything else, and a float or
// double that finds those full, uses a0-a7, then 4-byte-aligned stack slots
// (8-aligned for a double).
std::vector<ArgLoc> ClassifyArgs(const std::vector<TypePtr>& param_types,
                                 bool hidden_return) {
    std::vector<ArgLoc> locs;
    Cursor cursor;
    if (hidden_return) cursor.int_reg = 1;
    for (const TypePtr& type : param_types) locs.push_back(cursor.Take(*type));
    return locs;
}

int StackExtent(const ArgLoc& loc, const Type& type) {
    switch (loc.kind) {
        case ArgLoc::Kind::Stack:
            return loc.offset + (loc.by_reference ? 4 : RoundUp(type.Size(), 4));
        case ArgLoc::Kind::Split: return loc.offset + 4;
        default: return 0;
    }
}

namespace {
Piece IntWord(int reg, int offset, int bytes) {
    return Piece{Piece::Where::IntReg, reg, offset, bytes, false};
}

Piece FieldPiece(const FieldLoc& f) {
    Piece::Where where = f.is_float ? Piece::Where::FloatReg : Piece::Where::IntReg;
    return Piece{where, f.reg, f.offset, f.type->Size(), f.type->IsSigned()};
}
}  // namespace

std::vector<Piece> ArgPieces(const ArgLoc& loc, const Type& record) {
    int size = record.Size();
    switch (loc.kind) {
        case ArgLoc::Kind::IntReg: return {IntWord(loc.reg, 0, size)};
        case ArgLoc::Kind::IntPair:
            return {IntWord(loc.reg, 0, 4), IntWord(loc.reg + 1, 4, size - 4)};
        case ArgLoc::Kind::Split:
            return {IntWord(loc.reg, 0, 4),
                    Piece{Piece::Where::StackWord, loc.offset, 4, size - 4, false}};
        case ArgLoc::Kind::Fields: {
            std::vector<Piece> pieces;
            for (const FieldLoc& f : loc.fields) pieces.push_back(FieldPiece(f));
            return pieces;
        }
        default: return {};
    }
}

bool ReturnsThroughPointer(const Type& type) {
    return type.IsRecord() && ClassifyReturn(type).hidden_pointer;
}

// The same flattening as for arguments, with every register file empty: so
// {float,double} is fa0 and fa1, {int,double} is a0 and fa0.
RetLoc ClassifyReturn(const Type& record) {
    std::vector<FieldLoc> fields = FloatConventionFields(record);
    if (!fields.empty()) {
        Cursor cursor;
        std::vector<Piece> pieces;
        for (FieldLoc& f : fields) {
            f.reg = f.is_float ? cursor.float_reg++ : cursor.int_reg++;
            pieces.push_back(FieldPiece(f));
        }
        return RetLoc{false, pieces};
    }
    if (record.Size() > 8) return RetLoc{true, {}};
    if (record.Size() <= 4) return RetLoc{false, {IntWord(0, 0, record.Size())}};
    return RetLoc{false, {IntWord(0, 0, 4), IntWord(1, 4, record.Size() - 4)}};
}
