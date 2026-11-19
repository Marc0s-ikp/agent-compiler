#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <ostream>
#include <set>
#include <stdexcept>
#include <string>

#include "backend.hpp"
#include "call_abi.hpp"
#include "symbol_names.hpp"
#include "target.hpp"

// The AArch64 backend (D27, 27.3), AAPCS64 with LP64. It has the same shape as
// the RISC-V one (21.4, 21.6) but shares no code with it: every IR temporary
// owns a stack slot, and each instruction loads its operands into the scratch
// registers x9 and x10 (w9, w10), computes, and stores the result to the
// destination's slot. Nothing is held in a register between instructions.
// x16 and x17 hold addresses and large constants. All four are caller-saved,
// so the prologue saves only the frame record (x29 and x30).
//
// This is step A6: integer and floating operators, assignment, locals, control flow, calls
// with scalar and record arguments and results, and memcpy. An instruction
// that belongs to a later step throws "not in AArch64 yet".
namespace {

using backend::LocalLabel;
using backend::SymbolNames;
using ir::Instruction;
using ir::IrType;
using ir::Op;
using ir::Operand;

[[noreturn]] void NotYet(const std::string& what) {
    throw std::runtime_error("not in AArch64 yet: " + what);
}

int RoundUp(int value, int multiple) { return (value + multiple - 1) / multiple * multiple; }

// ---- registers and constants ----

// True for the types that live in an x register (64 bits); the others use w.
bool IsWide(IrType type) { return type == IrType::I64 || type == IrType::Ptr; }

// Register number n for a value of this IR type: w<n> or x<n>, or s<n> / d<n>
// for a float or double (27.3).
std::string Reg(int n, IrType type) {
    if (type == IrType::F32) return "s" + std::to_string(n);
    if (type == IrType::F64) return "d" + std::to_string(n);
    return std::string(IsWide(type) ? "x" : "w") + std::to_string(n);
}

// The scratch register number k (0 or 1) for a value of this type: x9/x10 for
// integers and pointers, d16/d17 for floats and doubles (27.3).
int Scratch(IrType type, int k) { return (ir::IsFloatType(type) ? 16 : 9) + k; }

// 'reg' = a constant: movz of the low 16 bits, then a movk for each further
// non-zero 16-bit chunk (at most 1 + 3). 'bits' is 32 for a w register, else 64.
void EmitConst(std::ostream& os, const std::string& reg, uint64_t value, int bits) {
    if (bits == 32) value &= 0xFFFFFFFFu;
    os << "\tmovz " << reg << ", #" << (value & 0xFFFF) << "\n";
    for (int shift = 16; shift < bits; shift += 16) {
        uint64_t chunk = (value >> shift) & 0xFFFF;
        if (chunk) os << "\tmovk " << reg << ", #" << chunk << ", lsl #" << shift << "\n";
    }
}

// The value of an integer immediate of this type, cut to the type's width.
uint64_t ImmediateBits(long value, IrType type) {
    uint64_t bits = static_cast<uint64_t>(value);
    int width = 8 * ir::SizeOf(type);
    return width >= 64 ? bits : bits & ((uint64_t{1} << width) - 1);
}

// ---- memory access helpers ----

bool FitsImm12(long v) { return v >= 0 && v <= 4095; }

// 'op reg, [base, #off]'. ldr/str take an unsigned 12-bit offset scaled by the
// access size, so 'off' must be a multiple of 'size' and at most 4095 * size.
// Otherwise the offset is built in x16 and added to the base.
void EmitMem(std::ostream& os, const std::string& op, const std::string& reg,
             const std::string& base, long off, int size) {
    if (off >= 0 && off % size == 0 && FitsImm12(off / size)) {
        os << "\t" << op << " " << reg << ", [" << base << ", #" << off << "]\n";
        return;
    }
    EmitConst(os, "x16", static_cast<uint64_t>(off), 64);
    os << "\tadd x16, " << base << ", x16\n";
    os << "\t" << op << " " << reg << ", [x16]\n";
}

// dst = base + off, for any offset (both are x registers, or sp).
void EmitAddrOffset(std::ostream& os, const std::string& dst, const std::string& base, long off) {
    if (off == 0 && dst == base) return;
    if (FitsImm12(off)) {
        os << "\tadd " << dst << ", " << base << ", #" << off << "\n";
    } else if (FitsImm12(-off)) {
        os << "\tsub " << dst << ", " << base << ", #" << -off << "\n";
    } else {
        EmitConst(os, "x16", static_cast<uint64_t>(off), 64);
        os << "\tadd " << dst << ", " << base << ", x16\n";
    }
}

// sp = sp -/+ bytes: an immediate if it fits 12 bits, else through x16.
void EmitAdjustSp(std::ostream& os, const char* op, int bytes) {
    if (FitsImm12(bytes)) {
        os << "\t" << op << " sp, sp, #" << bytes << "\n";
        return;
    }
    EmitConst(os, "x16", static_cast<uint64_t>(bytes), 64);
    os << "\t" << op << " sp, sp, x16\n";
}

const char* LoadOp(IrType type) {
    switch (type) {
        case IrType::I8: return "ldrb";  // a narrow temp holds bits; sext/zext widens
        case IrType::I16: return "ldrh";
        case IrType::I32:
        case IrType::I64:
        case IrType::Ptr:
        case IrType::F32:
        case IrType::F64: return "ldr";
    }
    NotYet("load type");
}

const char* StoreOp(IrType type) {
    switch (type) {
        case IrType::I8: return "strb";
        case IrType::I16: return "strh";
        case IrType::I32:
        case IrType::I64:
        case IrType::Ptr:
        case IrType::F32:
        case IrType::F64: return "str";
    }
    NotYet("store type");
}

// The load and store of an access of 1, 2, 4 or 8 bytes (an integer piece, or a unit of a record).
const char* LoadBytesOp(int bytes) { return bytes == 1 ? "ldrb" : bytes == 2 ? "ldrh" : "ldr"; }
const char* StoreBytesOp(int bytes) { return bytes == 1 ? "strb" : bytes == 2 ? "strh" : "str"; }

// ---- records (27.3) ----
// A record is memory in the IR: its value is the address of an object. Only at a call, an entry and a
// return is it split into pieces (call_abi.hpp), each with its byte offset inside the record. The unit
// of every access to a record is its alignment (at most 8 bytes), so a piece of a char-aligned record
// is read and written a byte at a time and never touches a byte outside the record. A size that is a
// multiple of the alignment is a multiple of the unit; the loops below shrink the unit if it is not.

int AccessUnit(int align, int bytes) {
    int unit = std::min(align, 8);
    while (bytes % unit != 0) unit /= 2;
    return unit;
}

// The w or x register number n for an access of 'unit' bytes.
std::string UnitReg(int n, int unit) { return std::string(unit == 8 ? "x" : "w") + std::to_string(n); }

// Copies 'bytes' bytes of alignment 'align' from the address in 'src' to the one in 'dst'. Up to 16
// units it is straight-line code; beyond that a loop runs x12 (dst) up to x14 (the end) with x13 (src).
// Clobbers x11 to x14 and x16. 'src' must be none of them and stays unchanged. 'dst' must not be x11
// to x14; it may be x16 (read before x16 is written), but x16 is then overwritten by a loop copy whose
// end offset needs more than 12 bits, so do not rely on 'dst' afterwards.
void EmitCopyMem(std::ostream& os, const std::string& dst, const std::string& src, int bytes, int align) {
    const int kUnrolledMax = 16;
    int unit = AccessUnit(align, bytes);
    std::string tmp = UnitReg(11, unit);
    if (bytes / unit <= kUnrolledMax) {
        for (int off = 0; off < bytes; off += unit) {
            EmitMem(os, LoadBytesOp(unit), tmp, src, off, unit);
            EmitMem(os, StoreBytesOp(unit), tmp, dst, off, unit);
        }
        return;
    }
    os << "\tmov x12, " << dst << "\n\tmov x13, " << src << "\n";
    EmitAddrOffset(os, "x14", "x12", bytes);
    os << "1:\n\t" << LoadBytesOp(unit) << " " << tmp << ", [x13], #" << unit << "\n";
    os << "\t" << StoreBytesOp(unit) << " " << tmp << ", [x12], #" << unit << "\n\tcmp x12, x14\n\tb.ne 1b\n";
}

// x<n> = 'bytes' bytes of a record at off(base), zero-extended, a unit at a time (clobbers x11).
void EmitLoadChunks(std::ostream& os, int n, int bytes, int align, const std::string& base, int off) {
    int unit = AccessUnit(align, bytes);
    EmitMem(os, LoadBytesOp(unit), UnitReg(n, unit), base, off, unit);
    for (int done = unit; done < bytes; done += unit) {
        EmitMem(os, LoadBytesOp(unit), UnitReg(11, unit), base, off + done, unit);
        os << "\torr x" << n << ", x" << n << ", x11, lsl #" << 8 * done << "\n";
    }
}

// The low 'bytes' bytes of x<n> to off(base), a unit at a time (clobbers x11).
void EmitStoreChunks(std::ostream& os, int n, int bytes, int align, const std::string& base, int off) {
    int unit = AccessUnit(align, bytes);
    EmitMem(os, StoreBytesOp(unit), UnitReg(n, unit), base, off, unit);
    for (int done = unit; done < bytes; done += unit) {
        os << "\tlsr x11, x" << n << ", #" << 8 * done << "\n";
        EmitMem(os, StoreBytesOp(unit), UnitReg(11, unit), base, off + done, unit);
    }
}

// ---- frame layout (27.3) ----
// Two parts, each a multiple of 16, so sp is always 16-aligned. The upper part
// holds, from x29 upwards: the frame record (saved x29 and x30), the frame
// objects (locals and parameter homes), then one slot per temp. Below it, at
// sp, is the outgoing argument area. Every slot is at a positive offset from x29.

struct Frame {
    std::map<std::string, int> objects;  // name -> offset from x29
    std::vector<int> temps;              // temp id -> offset from x29
    int return_pointer = 0;              // the saved x8 (the caller's result object), if any
    int upper = 0;                       // size of the upper part
    int outgoing = 0;                    // size of the area at sp

    int used = 16;  // bytes handed out from x29 so far (the frame record)

    int Reserve(int bytes, int align) {
        int offset = RoundUp(used, align);
        used = offset + bytes;
        return offset;
    }
};

Frame LayoutFrame(const ir::Function& fn, int outgoing_bytes, bool result_in_memory) {
    Frame frame;
    if (result_in_memory) frame.return_pointer = frame.Reserve(8, 8);
    for (const ir::FrameObject& o : fn.objects) frame.objects[o.name] = frame.Reserve(o.size, o.align);
    for (IrType t : fn.temps) frame.temps.push_back(frame.Reserve(ir::SizeOf(t), ir::SizeOf(t)));
    frame.upper = RoundUp(frame.used, 16);
    frame.outgoing = RoundUp(outgoing_bytes, 16);
    return frame;
}

class FunctionWriter {
public:
    // One Classify call for the function's own entry and one for each call (D5,
    // 21.5); the frame layout needs the calls' stack sizes first.
    FunctionWriter(const ir::Function& fn, const SymbolNames& names, std::ostream& os)
        : fn_(fn), names_(names), os_(os) {
        entry_ = Target().Abi().Classify(ParamTypes(), fn.c_return);
        int outgoing = 0;
        for (size_t i = 0; i < fn.body.size(); ++i) {
            const Instruction& in = fn.body[i];
            if (in.op != Op::Call) continue;
            calls_[i] = Target().Abi().Classify(in.arg_types, in.ret_type);
            outgoing = std::max(outgoing, calls_[i].stack_bytes);
        }
        frame_ = LayoutFrame(fn, outgoing, entry_.ret.in_memory);
    }

    void Write() {
        os_ << "\t.text\n\t.p2align 2\n";
        if (fn_.is_global) os_ << "\t.globl " << fn_.name << "\n";
        os_ << fn_.name << ":\n";
        WritePrologue();
        for (size_t i = 0; i < fn_.body.size(); ++i) WriteInstruction(i, fn_.body[i]);
        os_ << ReturnLabel() << ":\n";
        WriteEpilogue();
    }

private:
    std::vector<TypePtr> ParamTypes() const {
        std::vector<TypePtr> types;
        for (const ir::Param& p : fn_.params) types.push_back(p.type);
        return types;
    }

    std::string ReturnLabel() const { return LocalLabel("ret." + fn_.name); }

    // sub sp, upper; the frame record at the bottom of the upper part; x29 =
    // that record; then the outgoing area below it.
    void WritePrologue() {
        EmitAdjustSp(os_, "sub", frame_.upper);
        os_ << "\tstp x29, x30, [sp]\n\tmov x29, sp\n";
        if (frame_.outgoing) EmitAdjustSp(os_, "sub", frame_.outgoing);
        if (entry_.ret.in_memory) EmitMem(os_, "str", "x8", "x29", frame_.return_pointer, 8);  // for 'ret'
        StoreParameters();
    }

    // ---- parameters (callee side) ----

    // Each incoming piece goes to its parameter's home. A stack piece is in the
    // caller's outgoing area, which starts at x29 + upper.
    void StoreParameters() {
        for (size_t i = 0; i < fn_.params.size(); ++i) {
            int home = frame_.objects.at(fn_.params[i].object);
            if (fn_.params[i].type->IsRecord()) {
                StoreRecordParameter(entry_.args[i], *fn_.params[i].type, home);
                continue;
            }
            for (const Piece& piece : entry_.args[i].pieces) StoreIncomingPiece(piece, home);
        }
    }

    // A record parameter. By reference: copy from the caller's copy into the home. Otherwise each piece
    // is stored at its offset in the home; a whole record on the stack is copied from the caller's
    // outgoing area at x29 + upper + index.
    void StoreRecordParameter(const ArgPlace& place, const Type& type, int home) {
        if (place.by_reference) {
            LoadIncomingPointer(17, place.pieces.at(0));
            EmitAddrOffset(os_, "x16", "x29", home);
            return EmitCopyMem(os_, "x16", "x17", type.Size(), type.Align());
        }
        for (const Piece& piece : place.pieces) StoreIncomingRecordPiece(piece, type, home);
    }

    // The pointer of a by-reference argument, as it arrives, into x<n>.
    void LoadIncomingPointer(int n, const Piece& piece) {
        if (piece.where == Piece::Where::IntReg) {
            os_ << "\tmov x" << n << ", x" << piece.index << "\n";
        } else {
            EmitMem(os_, "ldr", "x" + std::to_string(n), "x29", frame_.upper + piece.index, 8);
        }
    }

    // A piece that is a whole record on the stack, not a part of one.
    static bool IsWholeStackRecord(const Piece& piece, const Type& record) {
        return piece.where == Piece::Where::StackWord && piece.offset == 0 && piece.bytes == record.Size();
    }

    static IrType FloatPieceType(const Piece& piece) { return piece.bytes == 8 ? IrType::F64 : IrType::F32; }

    // A record piece held in a register (v<n> as its float type, or x<n> a unit at a time) to off(base).
    // Used for an incoming parameter and for a returned record. Clobbers x11 (and x16 for a far offset).
    void StoreRecordRegisterPiece(const Piece& piece, const Type& type, const std::string& base, int off) {
        switch (piece.where) {
            case Piece::Where::FloatReg:
                return EmitMem(os_, "str", Reg(piece.index, FloatPieceType(piece)), base, off, piece.bytes);
            case Piece::Where::IntReg:
                return EmitStoreChunks(os_, piece.index, piece.bytes, type.Align(), base, off);
            case Piece::Where::StackWord:
                NotYet("a stack piece where a register piece is expected");
        }
    }

    void StoreIncomingRecordPiece(const Piece& piece, const Type& type, int home) {
        if (piece.where != Piece::Where::StackWord) {
            return StoreRecordRegisterPiece(piece, type, "x29", home + piece.offset);
        }
        if (!IsWholeStackRecord(piece, type)) NotYet("a record split between registers and the stack");
        EmitAddrOffset(os_, "x17", "x29", frame_.upper + piece.index);
        EmitAddrOffset(os_, "x16", "x29", home);
        EmitCopyMem(os_, "x16", "x17", piece.bytes, type.Align());
    }

    // Only the piece's bytes are stored: the upper bits of a narrow argument are
    // unspecified (27.2), and the IR extends the loaded home again.
    void StoreIncomingPiece(const Piece& piece, int home) {
        std::string reg = PieceReg(piece);
        if (piece.where == Piece::Where::StackWord) {
            EmitMem(os_, LoadBytesOp(piece.bytes), reg, "x29", frame_.upper + piece.index, piece.bytes);
        }
        EmitMem(os_, StoreBytesOp(piece.bytes), reg, "x29", home + piece.offset, piece.bytes);
    }

    // The register that holds the piece: x<n> or w<n> for an integer piece; s<n>
    // or d<n> for a floating one. A stack piece goes through x9 or w9 (a float
    // or double is only copied, as bits).
    static std::string PieceReg(const Piece& piece) {
        bool wide = piece.bytes == 8;
        if (piece.where == Piece::Where::FloatReg) return std::string(wide ? "d" : "s") + std::to_string(piece.index);
        return std::string(wide ? "x" : "w") + std::to_string(piece.where == Piece::Where::IntReg ? piece.index : 9);
    }

    void WriteEpilogue() {
        os_ << "\tmov sp, x29\n\tldp x29, x30, [sp]\n";
        EmitAdjustSp(os_, "add", frame_.upper);
        os_ << "\tret\n";
    }

    // ---- operands ----

    // Operand o into register number n (w or x by its type; an object or
    // symbol is an address, so x).
    void Load(int n, const Operand& o) {
        switch (o.kind) {
            case Operand::Kind::Temp:
                return EmitMem(os_, LoadOp(o.type), Reg(n, o.type), "x29", frame_.temps[o.temp], ir::SizeOf(o.type));
            case Operand::Kind::Int:
                return EmitConst(os_, Reg(n, o.type), ImmediateBits(o.int_value, o.type), IsWide(o.type) ? 64 : 32);
            case Operand::Kind::Object:
                return EmitAddrOffset(os_, "x" + std::to_string(n), "x29", frame_.objects.at(o.name));
            case Operand::Kind::Symbol: return EmitSymbolAddress(n, names_.Spell(o.name));
            case Operand::Kind::Float: NotYet("floating constant");
            case Operand::Kind::None: break;
        }
        NotYet("operand");
    }

    // x<n> = &sym, for any distance within +-4 GB (static linking, no GOT).
    void EmitSymbolAddress(int n, const std::string& sym) {
        std::string reg = "x" + std::to_string(n);
        os_ << "\tadrp " << reg << ", " << sym << "\n\tadd " << reg << ", " << reg << ", :lo12:" << sym << "\n";
    }

    void StoreResult(int n, const Operand& dst) {
        EmitMem(os_, StoreOp(dst.type), Reg(n, dst.type), "x29", frame_.temps[dst.temp], ir::SizeOf(dst.type));
    }

    // The value comes back in x0 or w0 (27.2); the upper bits of a narrow
    // value are unspecified, so nothing is extended.
    void LoadReturnValue(const Operand& value) {
        if (fn_.c_return->IsRecord()) return LoadReturnRecord(value);
        Load(entry_.ret.pieces.at(0).index, value);  // x0, w0, s0 or d0, by the value's type
    }

    // 'ret' of a record: the operand is the address of the value. Through the saved x8 it is copied to
    // the caller's result object; otherwise its pieces are loaded into x0, x1 or v0 to v3.
    void LoadReturnRecord(const Operand& value) {
        const Type& type = *fn_.c_return;
        Load(17, value);
        if (entry_.ret.in_memory) {
            EmitMem(os_, "ldr", "x16", "x29", frame_.return_pointer, 8);
            return EmitCopyMem(os_, "x16", "x17", type.Size(), type.Align());
        }
        for (const Piece& piece : entry_.ret.pieces) PlaceOutgoingRecordPiece(piece, type);
    }

    // ---- calls (caller side) ----

    // Each argument piece goes to its x register or its word of the outgoing
    // area at sp. Only the piece's bytes are written: the upper bits of a narrow
    // argument are unspecified (27.2). Then 'bl'; an integer or pointer result
    // is in x0 (w0), and its slot keeps the bytes of the result's type.
    void WriteCall(size_t index, const Instruction& in) {
        const CallLayout& layout = calls_.at(index);
        if (layout.ret.in_memory) Load(layout.ret.pointer.index, in.result_object);  // x8
        for (size_t i = 0; i < in.args.size(); ++i) {
            if (in.arg_types[i]->IsRecord()) {
                PlaceOutgoingRecord(in.args[i], layout.args[i], *in.arg_types[i]);
                continue;
            }
            for (const Piece& piece : layout.args[i].pieces) PlaceOutgoingPiece(in.args[i], piece);
        }
        os_ << "\tbl " << in.callee << "\n";
        if (in.dst.kind == Operand::Kind::Temp) {
            StoreResult(0, in.dst);
        } else if (in.ret_type->IsRecord()) {
            SaveReturnedRecord(layout.ret, in);
        }
    }

    // The caller of a record argument: the operand is the address of the argument's copy. By reference
    // it is passed as it is; otherwise the pieces are loaded from it (its address is in x17).
    void PlaceOutgoingRecord(const Operand& o, const ArgPlace& place, const Type& type) {
        if (place.by_reference) return PlaceOutgoingPointer(o, place.pieces.at(0));
        Load(17, o);
        for (const Piece& piece : place.pieces) PlaceOutgoingRecordPiece(piece, type);
    }

    void PlaceOutgoingPointer(const Operand& o, const Piece& piece) {
        if (piece.where == Piece::Where::IntReg) return Load(piece.index, o);
        Load(9, o);
        EmitMem(os_, "str", "x9", "sp", piece.index, 8);
    }

    // One piece of the record whose address is in x17, to its register or its place in the outgoing
    // area. Also the pieces of a returned record, which go to x0, x1 or v0 to v3.
    void PlaceOutgoingRecordPiece(const Piece& piece, const Type& type) {
        switch (piece.where) {
            case Piece::Where::FloatReg:
                return EmitMem(os_, "ldr", Reg(piece.index, FloatPieceType(piece)), "x17", piece.offset, piece.bytes);
            case Piece::Where::IntReg:
                return EmitLoadChunks(os_, piece.index, piece.bytes, type.Align(), "x17", piece.offset);
            case Piece::Where::StackWord:
                if (!IsWholeStackRecord(piece, type)) NotYet("a record split between registers and the stack");
                EmitAddrOffset(os_, "x16", "sp", piece.index);
                return EmitCopyMem(os_, "x16", "x17", piece.bytes, type.Align());
        }
    }

    // After the call: the returned pieces (x0, x1, v0 to v3) are stored into the call's result object. A
    // result in memory was written by the callee through x8, so there is nothing to do.
    void SaveReturnedRecord(const RetPlace& ret, const Instruction& in) {
        if (ret.in_memory) return;
        const Type& type = *in.ret_type;
        Load(17, in.result_object);
        for (const Piece& piece : ret.pieces) StoreRecordRegisterPiece(piece, type, "x17", piece.offset);
    }

    void PlaceOutgoingPiece(const Operand& o, const Piece& piece) {
        switch (piece.where) {
            case Piece::Where::IntReg: return Load(piece.index, o);
            case Piece::Where::StackWord:
                Load(Scratch(o.type, 0), o);
                return EmitMem(os_, StoreOp(o.type), Reg(Scratch(o.type, 0), o.type), "sp", piece.index, ir::SizeOf(o.type));
            case Piece::Where::FloatReg: return Load(piece.index, o);  // s<n> or d<n> by the type
        }
    }

    // ---- instructions ----

    // Regs of an operation on 'type': dst/lhs/rhs all in scratch 9 and 10.
    void Emit3(const char* mnemonic, IrType type) {
        os_ << "\t" << mnemonic << " " << Reg(9, type) << ", " << Reg(9, type) << ", " << Reg(10, type) << "\n";
    }

    static const char* BinaryMnemonic(Op op) {
        switch (op) {
            case Op::Add: return "add";
            case Op::Sub: return "sub";
            case Op::Mul: return "mul";
            case Op::Div: return "sdiv";
            case Op::DivU: return "udiv";
            case Op::And: return "and";
            case Op::Or: return "orr";
            case Op::Xor: return "eor";
            case Op::Shl: return "lsl";
            case Op::Shr: return "lsr";
            case Op::Sar: return "asr";
            default: return nullptr;
        }
    }

    // fadd, fsub, fmul or fdiv on d16 and d17 (or s16, s17).
    void WriteFloatBinary(const Instruction& in) {
        const char* name = in.op == Op::Add ? "fadd" : in.op == Op::Sub ? "fsub" : in.op == Op::Mul ? "fmul"
                         : in.op == Op::Div ? "fdiv" : nullptr;
        if (!name) NotYet(std::string("floating ") + ir::OpName(in.op));
        Load(16, in.args[0]);
        Load(17, in.args[1]);
        std::string a = Reg(16, in.type);
        os_ << "\t" << name << " " << a << ", " << a << ", " << Reg(17, in.type) << "\n";
        StoreResult(16, in.dst);
    }

    void WriteBinary(const Instruction& in) {
        if (ir::IsFloatType(in.type)) return WriteFloatBinary(in);
        Load(9, in.args[0]);
        Load(10, in.args[1]);
        if (in.op == Op::Rem || in.op == Op::RemU) {
            // a - (a / b) * b, with the quotient in x17 or w17
            EmitRemainder(in);
        } else {
            Emit3(BinaryMnemonic(in.op), in.type);
        }
        StoreResult(9, in.dst);
    }

    void EmitRemainder(const Instruction& in) {
        std::string q = Reg(17, in.type), a = Reg(9, in.type), b = Reg(10, in.type);
        os_ << "\t" << (in.op == Op::Rem ? "sdiv " : "udiv ") << q << ", " << a << ", " << b << "\n";
        os_ << "\tmsub " << a << ", " << q << ", " << b << ", " << a << "\n";
    }

    void WriteFloatNeg(const Instruction& in) {
        Load(16, in.args[0]);
        os_ << "\tfneg " << Reg(16, in.type) << ", " << Reg(16, in.type) << "\n";
        StoreResult(16, in.dst);
    }

    static const char* Condition(Op op) {
        switch (op) {
            case Op::Eq: return "eq";
            case Op::Ne: return "ne";
            case Op::Lt: return "lt";
            case Op::LtU: return "lo";
            case Op::Le: return "le";
            case Op::LeU: return "ls";
            default: NotYet(ir::OpName(op));
        }
    }

    // fcmp sets N, Z, C, V as 0b0011 (C, V) for an unordered pair, so 'lt' and
    // 'le' would be true for NaN. 'mi' (N set) is "less than" and 'ls' (C clear
    // or Z set) is "less or equal", both false when unordered; 'eq' is false too.
    // != is built from FEq by the frontend.
    void WriteFloatCompare(const Instruction& in) {
        const char* cond = in.op == Op::FEq ? "eq" : in.op == Op::FLt ? "mi" : "ls";
        Load(16, in.args[0]);
        Load(17, in.args[1]);
        os_ << "\tfcmp " << Reg(16, in.type) << ", " << Reg(17, in.type) << "\n";
        os_ << "\tcset w9, " << cond << "\n";
        StoreResult(9, in.dst);
    }

    void WriteCompare(const Instruction& in) {
        if (ir::IsFloatType(in.type)) return WriteFloatCompare(in);
        Load(9, in.args[0]);
        Load(10, in.args[1]);
        os_ << "\tcmp " << Reg(9, in.type) << ", " << Reg(10, in.type) << "\n";
        os_ << "\tcset w9, " << Condition(in.op) << "\n";
        StoreResult(9, in.dst);
    }

    // Integer widening and truncation. The source is loaded by its own type;
    // a trunc stores the low bytes of the same register number.
    void WriteConversion(const Instruction& in) {
        IrType from = in.args[0].type;
        if (in.op == Op::PtrToInt || in.op == Op::IntToPtr) return WritePointerCast(in);
        if (in.op >= Op::IToF && in.op <= Op::FTrunc) return WriteFloatConversion(in);
        if (in.op != Op::SExt && in.op != Op::ZExt && in.op != Op::Trunc) NotYet(ir::OpName(in.op));
        Load(9, in.args[0]);
        std::string src = Reg(9, from), dst = Reg(9, in.type);
        int bytes = ir::SizeOf(from);
        if (in.op == Op::SExt) {
            os_ << "\t" << (bytes == 1 ? "sxtb " : bytes == 2 ? "sxth " : "sxtw ") << dst << ", " << src << "\n";
        } else if (in.op == Op::ZExt) {  // a w-register write clears bits 32 to 63
            os_ << "\t" << (bytes == 1 ? "uxtb " : bytes == 2 ? "uxth " : "mov ") << "w9, w9\n";
        }
        StoreResult(9, in.dst);
    }

    // scvtf/ucvtf from a w or x register, fcvtzs/fcvtzu (round toward zero, as C
    // needs) to one, fcvt between s and d. Integer sources and results are I32
    // or I64 (the frontend widens narrow ones).
    void WriteFloatConversion(const Instruction& in) {
        IrType from = in.args[0].type;
        if (in.op == Op::FExt || in.op == Op::FTrunc) {
            Load(16, in.args[0]);
            os_ << "\tfcvt " << Reg(16, in.type) << ", " << Reg(16, from) << "\n";
            return StoreResult(16, in.dst);
        }
        if (in.op == Op::IToF || in.op == Op::UToF) {
            Load(9, in.args[0]);
            os_ << "\t" << (in.op == Op::IToF ? "scvtf " : "ucvtf ") << Reg(16, in.type) << ", " << Reg(9, from) << "\n";
            return StoreResult(16, in.dst);
        }
        Load(16, in.args[0]);
        os_ << "\t" << (in.op == Op::FToI ? "fcvtzs " : "fcvtzu ") << Reg(9, in.type) << ", " << Reg(16, from) << "\n";
        StoreResult(9, in.dst);
    }

    // A pointer and an I64 are both 64 bits in an x register, so the cast is a copy.
    void WritePointerCast(const Instruction& in) {
        Load(9, in.args[0]);
        StoreResult(9, in.dst);
    }

    // 'cbz' reaches only +-1 MB, so it skips one 'b' and every jump to a label
    // is a 'b' (+-128 MB). The condition is in w9, or x9 for an I64 or Ptr.
    void WriteBranch(const Instruction& in) {
        Load(9, in.args[0]);
        os_ << "\tcbz " << Reg(9, in.args[0].type) << ", 1f\n\tb " << LocalLabel(in.label)
            << "\n1:\n\tb " << LocalLabel(in.label2) << "\n";
    }

    // Zeroes in the widest stores the alignment and size allow. Up to 64 bytes
    // it is unrolled (the widest store that fits at each offset); beyond that a
    // loop runs x9 up to x10 = the end, one store of 'step' bytes at a time.
    void WriteMemZero(const Instruction& in) {
        Load(9, in.args[0]);
        int step = in.align >= 8 && in.size % 8 == 0 ? 8 : in.align >= 4 && in.size % 4 == 0 ? 4 : 1;
        if (in.size <= 64) {
            for (long off = 0; off < in.size;) {
                int width = ZeroWidth(in.align, in.size - off, off);
                EmitMem(os_, StoreBytesOp(width), width == 8 ? "xzr" : "wzr", "x9", off, width);
                off += width;
            }
            return;
        }
        EmitAddrOffset(os_, "x10", "x9", in.size);
        os_ << "1:\n\t" << StoreBytesOp(step) << " " << (step == 8 ? "xzr" : "wzr") << ", [x9], #" << step
            << "\n\tcmp x9, x10\n\tb.ne 1b\n";
    }

    // dst in x9, src in x10; EmitCopyMem uses x11 to x14 (and x16) as its own.
    void WriteMemCopy(const Instruction& in) {
        Load(9, in.args[0]);
        Load(10, in.args[1]);
        EmitCopyMem(os_, "x9", "x10", static_cast<int>(in.size), static_cast<int>(in.align));
    }

    // The widest of 8, 4, 2, 1 that the base alignment, the offset and the bytes left allow.
    static int ZeroWidth(long align, long left, long off) {
        for (int width : {8, 4, 2}) {
            if (align >= width && off % width == 0 && left >= width) return width;
        }
        return 1;
    }

    void WriteInstruction(size_t index, const Instruction& in) {
        if (ir::IsBinaryArith(in.op)) return WriteBinary(in);
        if (ir::IsCompare(in.op)) return WriteCompare(in);
        if (ir::IsConversion(in.op)) return WriteConversion(in);
        switch (in.op) {
            case Op::Copy:
            case Op::Const: {
                int reg = Scratch(in.args[0].type, 0);
                Load(reg, in.args[0]);
                return StoreResult(reg, in.dst);
            }
            case Op::PtrAdd:  // pointer + byte offset, both 64-bit (the frontend scaled it)
                Load(9, in.args[0]);
                Load(10, in.args[1]);
                os_ << "\tadd x9, x9, x10\n";
                return StoreResult(9, in.dst);
            case Op::Neg:
            case Op::Not:
                if (ir::IsFloatType(in.type)) return WriteFloatNeg(in);
                Load(9, in.args[0]);
                os_ << "\t" << (in.op == Op::Neg ? "neg " : "mvn ") << Reg(9, in.type) << ", " << Reg(9, in.type) << "\n";
                return StoreResult(9, in.dst);
            case Op::Addr:
                Load(9, in.args[0]);
                EmitAddrOffset(os_, "x9", "x9", in.offset);
                return StoreResult(9, in.dst);
            case Op::Load:
                Load(9, in.args[0]);
                EmitMem(os_, LoadOp(in.type), Reg(Scratch(in.type, 0), in.type), "x9", 0, ir::SizeOf(in.type));
                return StoreResult(Scratch(in.type, 0), in.dst);
            case Op::Store:
                Load(Scratch(in.type, 0), in.args[0]);
                Load(10, in.args[1]);
                return EmitMem(os_, StoreOp(in.type), Reg(Scratch(in.type, 0), in.type), "x10", 0, ir::SizeOf(in.type));
            case Op::MemZero: return WriteMemZero(in);
            case Op::MemCpy: return WriteMemCopy(in);
            case Op::Label: os_ << LocalLabel(in.label) << ":\n"; return;
            case Op::Jump: os_ << "\tb " << LocalLabel(in.label) << "\n"; return;
            case Op::Branch: return WriteBranch(in);
            case Op::Call: return WriteCall(index, in);
            case Op::Ret:
                if (!in.args.empty()) LoadReturnValue(in.args[0]);
                os_ << "\tb " << ReturnLabel() << "\n";
                return;
            default: NotYet(ir::OpName(in.op));
        }
    }

    const ir::Function& fn_;
    const SymbolNames& names_;
    std::ostream& os_;
    CallLayout entry_;
    std::map<size_t, CallLayout> calls_;  // by instruction index
    Frame frame_;
};

// ---- data (21.2a): the only place that writes data directives ----

void WriteItem(const ir::DataItem& item, const SymbolNames& names, std::ostream& os) {
    using Kind = ir::DataItem::Kind;
    switch (item.kind) {
        case Kind::Bytes:
            for (unsigned char c : item.bytes) os << "\t.byte " << static_cast<int>(c) << "\n";
            break;
        case Kind::Int: {
            const char* directive = item.size == 1 ? ".byte" : item.size == 2 ? ".hword" : item.size == 4 ? ".word" : ".xword";
            if (item.size != 1 && item.size != 2 && item.size != 4 && item.size != 8) NotYet("integer data item size");
            os << "\t" << directive << " " << item.value << "\n";
            break;
        }
        case Kind::Float:  // the IEEE bits, as an integer of the same size
            os << (item.size == 8 ? "\t.xword " : "\t.word ") << (item.size == 8 ? item.value : (item.value & 0xFFFFFFFFLL)) << "\n";
            break;
        case Kind::Address:  // a pointer is 8 bytes
            os << "\t.xword " << names.Spell(item.bytes);
            if (item.offset) os << (item.offset < 0 ? "-" : "+") << std::labs(item.offset);
            os << "\n";
            break;
        case Kind::Zero: os << "\t.zero " << item.size << "\n"; break;
    }
}

void WriteData(const ir::Module& module, const SymbolNames& names, std::ostream& os) {
    for (const ir::DataEntry& d : module.data) {
        os << (d.read_only ? "\t.section .rodata\n" : "\t.data\n");
        os << "\t.balign " << d.align << "\n";
        if (!d.internal) os << "\t.globl " << d.label << "\n";
        os << names.Spell(d.label) << ":\n";
        if (d.items.empty()) os << "\t.zero " << d.size << "\n";
        for (const ir::DataItem& item : d.items) WriteItem(item, names, os);
    }
}

}  // namespace

void EmitAArch64(const ir::Module& module, std::ostream& os) {
    SymbolNames names(module);
    for (const ir::Function& fn : module.functions) FunctionWriter(fn, names, os).Write();
    WriteData(module, names, os);
}
