#include "call_abi_aarch64.hpp"

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>

// The AArch64 calling convention (AAPCS64, LP64; D27, 27.2) and nothing else
// knows its rules (D25). Steps A3 and A5 classify scalars: integers and
// pointers in x0-x7, floats and doubles in v0-v7 (s and d), the two counts
// independent, and then 8-byte stack slots in order for both. Step A6 adds
// records: a Homogeneous Floating-point Aggregate in consecutive v registers,
// another record of up to 16 bytes in one or two x registers, a larger one by
// reference, and the same placement for a record result (27.2).
namespace {

constexpr int kIntArgRegs = 8;     // x0-x7
constexpr int kFloatArgRegs = 8;   // v0-v7
constexpr int kStackSlotBytes = 8;
constexpr int kResultPointerReg = 8;  // IntReg index 8 is x8, not an argument register (27.8.1)

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

int RoundUp(int value, int multiple) { return (value + multiple - 1) / multiple * multiple; }

// ---- Homogeneous Floating-point Aggregates (AAPCS64 "Homogeneous Aggregates") ----

// A record is an HFA if, after layout, every Fundamental Data Type in it is the
// same floating type (float or double) and it has 1 to 4 uniquely addressable
// members. A struct's members add up; the members of a union overlap, so only
// its largest member counts (GCC passes union { float a; float b; } in s0, and
// union { float a[2]; float b[3]; } in s0 to s2). An array of n elements has n
// times its element's members.
struct Hfa {
    int base_size;  // 4 (float) or 8 (double)
    int members;
};

// Counts the members of 'type' into 'count' and agrees the base size with
// 'base_size' (0 until the first floating type is found). False if some leaf
// is not floating or the leaves differ.
bool CountFloatMembers(const Type& type, int& base_size, int& count) {
    if (type.IsFloating()) {
        if (base_size != 0 && base_size != type.Size()) return false;
        base_size = type.Size();
        count = 1;
        return true;
    }
    if (type.IsArray()) {
        if (!CountFloatMembers(*type.Base(), base_size, count)) return false;
        count *= type.Length();
        return true;
    }
    if (!type.IsRecord()) return false;
    int total = 0;
    for (const RecordMember& member : type.Members()) {
        int member_count = 0;
        if (!CountFloatMembers(*member.type, base_size, member_count)) return false;
        total = type.IsUnion() ? std::max(total, member_count) : total + member_count;
    }
    count = total;
    return true;
}

std::optional<Hfa> HomogeneousFloatAggregate(const Type& record) {
    int base_size = 0, count = 0;
    if (!CountFloatMembers(record, base_size, count)) return std::nullopt;
    bool no_padding = count * base_size == record.Size();
    if (count < 1 || count > 4 || !no_padding) return std::nullopt;
    return Hfa{base_size, count};
}

// ---- the argument assigner: AAPCS64 stages B and C ----
// NGRN, NSRN and NSAA (in bytes from the start of the outgoing area) are the
// standard's counters. NSAA is always a multiple of 8 here: every stack
// argument takes whole 8-byte slots (C.5, C.16, B.5, C.3) and nothing needs
// 16-byte alignment, so C.4 and C.14 never move it.
class ArgAssigner {
public:
    ArgPlace Place(const Type& type) {
        if (type.IsRecord()) return PlaceRecord(type);
        if (!IsScalar(type)) NotYet("parameter type other than a scalar");
        return type.IsFloating() ? PlaceFloating(type) : PlaceIntegral(type);
    }

    // The outgoing area, rounded up to the 16-byte stack alignment.
    int StackBytes() const { return RoundUp(nsaa_, 16); }

private:
    // C.1: a float or double goes in the next v register if NSRN < 8; else
    // (C.5, C.6) in the low bytes of the next 8-byte stack slot.
    ArgPlace PlaceFloating(const Type& type) {
        ArgPlace place;
        if (nsrn_ < kFloatArgRegs) {
            place.pieces.push_back(ScalarPiece(Piece::Where::FloatReg, nsrn_++, type));
        } else {
            place.pieces.push_back(ScalarPiece(Piece::Where::StackWord, TakeStack(kStackSlotBytes), type));
        }
        return place;
    }

    // C.9: an integer or pointer goes in the next x register if NGRN < 8; else
    // (C.16, C.17) in the next 8-byte stack slot.
    ArgPlace PlaceIntegral(const Type& type) {
        ArgPlace place;
        place.pieces.push_back(ScalarPiece(Piece::Where::IntReg, 0, type));
        PlaceWord(place.pieces.back());
        return place;
    }

    // Puts a one-word piece in the next x register, or else in the next stack slot.
    void PlaceWord(Piece& piece) {
        if (ngrn_ < kIntArgRegs) {
            piece.where = Piece::Where::IntReg;
            piece.index = ngrn_++;
        } else {
            piece.where = Piece::Where::StackWord;
            piece.index = TakeStack(kStackSlotBytes);
        }
    }

    // The byte offset of 'bytes' bytes taken from the stack, rounded to whole slots.
    int TakeStack(int bytes) {
        int offset = nsaa_;
        nsaa_ += RoundUp(bytes, kStackSlotBytes);
        return offset;
    }

    ArgPlace PlaceRecord(const Type& record) {
        if (std::optional<Hfa> hfa = HomogeneousFloatAggregate(record)) return PlaceHfa(record, *hfa);
        if (record.Size() > 16) return PlaceByReference();
        return PlaceInIntRegs(record);
    }

    // B.3, C.2: one v register per member if NSRN + members <= 8. C.3, C.4, C.6:
    // otherwise NSRN = 8 and the whole HFA goes on the stack, one piece as big
    // as the record (offset 0), taking its size rounded up to 8.
    ArgPlace PlaceHfa(const Type& record, const Hfa& hfa) {
        ArgPlace place;
        if (nsrn_ + hfa.members <= kFloatArgRegs) {
            for (int i = 0; i < hfa.members; ++i) {
                place.pieces.push_back(Piece{Piece::Where::FloatReg, nsrn_++, i * hfa.base_size, hfa.base_size, false});
            }
            return place;
        }
        nsrn_ = kFloatArgRegs;
        place.pieces.push_back(Piece{Piece::Where::StackWord, TakeStack(record.Size()), 0, record.Size(), false});
        return place;
    }

    // B.4: a record over 16 bytes (and not an HFA) is copied by the caller and
    // replaced by a pointer to the copy, which is placed as an integer (C.9).
    ArgPlace PlaceByReference() {
        ArgPlace place;
        place.by_reference = true;
        place.pieces.push_back(Piece{Piece::Where::IntReg, 0, 0, 8, false});
        PlaceWord(place.pieces.back());
        return place;
    }

    // B.5, C.12: a record of up to 16 bytes, its size rounded up to whole
    // double-words, goes in that many consecutive x registers if they are left;
    // the last piece may be shorter (a 12-byte record is 8 + 4). C.13, C.15:
    // otherwise NGRN = 8 (it is never split between registers and the stack)
    // and the whole record goes on the stack, one piece as big as the record.
    ArgPlace PlaceInIntRegs(const Type& record) {
        ArgPlace place;
        int words = RoundUp(record.Size(), 8) / 8;
        if (words <= kIntArgRegs - ngrn_) {
            for (int i = 0; i < words; ++i) {
                int bytes = std::min(8, record.Size() - 8 * i);
                place.pieces.push_back(Piece{Piece::Where::IntReg, ngrn_++, 8 * i, bytes, false});
            }
            return place;
        }
        ngrn_ = kIntArgRegs;
        place.pieces.push_back(Piece{Piece::Where::StackWord, TakeStack(record.Size()), 0, record.Size(), false});
        return place;
    }

    int ngrn_ = 0;
    int nsrn_ = 0;
    int nsaa_ = 0;
};

// 6.9: a result is returned in the registers in which `void f(T)` would take
// its argument as a value (stage A, B, C on a fresh assigner); if that argument
// would be on the stack or replaced by a pointer (B.4), the caller passes the
// address of the result object in x8, which is IntReg index 8 (27.8.1).
RetPlace ClassifyReturn(const Type& ret) {
    RetPlace place;
    if (ret.IsVoid()) return place;
    if (!IsScalar(ret) && !ret.IsRecord()) NotYet("return type other than a scalar, a record or void");
    ArgPlace as_argument = ArgAssigner().Place(ret);
    if (as_argument.by_reference || as_argument.pieces.at(0).where == Piece::Where::StackWord) {
        place.in_memory = true;
        place.pointer = Piece{Piece::Where::IntReg, kResultPointerReg, 0, 8, false};
    } else {
        place.pieces = as_argument.pieces;
    }
    return place;
}

}  // namespace

CallLayout AArch64Abi::Classify(const std::vector<TypePtr>& params, const TypePtr& ret) const {
    CallLayout layout;
    ArgAssigner assigner;
    for (const TypePtr& param : params) layout.args.push_back(assigner.Place(*param));
    layout.ret = ClassifyReturn(*ret);
    layout.stack_bytes = assigner.StackBytes();
    return layout;
}
