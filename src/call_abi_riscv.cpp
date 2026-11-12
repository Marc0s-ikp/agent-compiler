#include "call_abi_riscv.hpp"

#include <algorithm>
#include <stdexcept>

// The RISC-V (ilp32d) calling convention (D5, D18(d), D19(d)) and nothing else
// knows its rules (D25). Everything above RiscV32Abi::Classify is private to
// this file: ClassifyArgs and ClassifyReturn decide where each argument and a
// struct result live, StackExtent sizes the outgoing area, and the Place*
// functions turn that into the target-neutral Pieces of call_abi.hpp.
namespace {

// Where one argument lives under the ilp32d calling convention. The caller's
// and the callee's layouts both come from this one classifier, so the two
// sides cannot disagree (D5).
//   IntReg    a<reg>: an integer, a pointer, or a float whose fa registers
//             ran out (it travels as its 32 bits)
//   FloatReg  fa<reg>: a float or double
//   IntPair   a<reg> (low word) and a<reg+1> (high word): a double whose fa
//             registers ran out
//   Split     a7 (low word) and the stack word at 'offset' (high word): a
//             double when only a7 is left
//   Stack     'offset' bytes above the caller's sp (= s0 in the callee)
//   Fields    a struct in the floating convention: each flattened scalar
//             field has its own register in 'fields' (D19)
// A struct or union uses the same kinds as a double does (IntReg for up to 4
// bytes, IntPair, Split or Stack for 8) when it is passed in integer words.
// A struct over 8 bytes is passed by reference (D19): 'by_reference' is set
// and IntReg or Stack describes the one-word pointer to the caller's copy.
struct FieldLoc {
    bool is_float;
    int reg;     // fa or a register number
    int offset;  // byte offset of the field inside the struct
    TypePtr type;
};

struct ArgLoc {
    enum class Kind { IntReg, FloatReg, IntPair, Split, Stack, Fields };
    ArgLoc(Kind k, int r, int o) : kind(k), reg(r), offset(o) {}
    Kind kind;
    int reg;
    int offset;
    bool by_reference = false;
    std::vector<FieldLoc> fields;  // Kind::Fields only
};

// How a struct is returned: through a hidden pointer, or in 'pieces' (a0, a1,
// fa0, fa1).
struct RetLoc {
    bool hidden_pointer;
    std::vector<Piece> pieces;
};

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

// Floats and doubles use fa0-fa7 first. Everything else, and a float or
// double that finds those full, uses a0-a7, then 4-byte-aligned stack slots
// (8-aligned for a double). 'hidden_return' is true if the callee returns a
// struct through a hidden pointer, which takes a0 and moves every integer
// register up by one.
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

Piece IntWord(int reg, int offset, int bytes) {
    return Piece{Piece::Where::IntReg, reg, offset, bytes, false};
}

Piece FieldPiece(const FieldLoc& f) {
    Piece::Where where = f.is_float ? Piece::Where::FloatReg : Piece::Where::IntReg;
    return Piece{where, f.reg, f.offset, f.type->Size(), f.type->IsSigned()};
}

// The pieces of a struct passed in registers (any kind except Stack and
// by-reference). The caller loads each from the struct; the callee stores
// each into the parameter's home.
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

[[noreturn]] void NotYet() { throw std::runtime_error("not in IR yet: this type in a call"); }

bool IsPassable(const Type& type) {
    return (type.IsInteger() && type.Size() <= 4) || type.IsPointer() || type.IsFloating() ||
           type.IsRecord();
}

Piece ScalarPiece(Piece::Where where, int index, const Type& type) {
    return Piece{where, index, 0, type.Size(), type.IsInteger() && type.IsSigned()};
}

// A float or double: in an fa register as one piece; otherwise what ArgPieces
// says (one IntReg piece for a float, two for a double in a register
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
// ArgPieces gives the integer words (IntReg, IntPair, the a7 Split)
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
