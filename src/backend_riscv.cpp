#include "backend.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <ostream>
#include <set>
#include <stdexcept>

#include "call_abi.hpp"
#include "target.hpp"

// Immediates (21.10): any IR integer is legal. Every constant operand is
// loaded with 'li', which the assembler expands to lui+addi when it does not
// fit 12 bits, so no instruction here takes a variable immediate. The fixed
// ones (shift counts, 'xori 1', 0 offsets) always fit; frame offsets and Addr
// offsets go through EmitMem / EmitAddrOffset, which fall back to a register.
//
// Every IR temporary owns a stack slot (21.4). Each instruction loads its
// operands into t0 and t1, computes, and stores the result to the
// destination's slot, so no register holds a value between instructions.
namespace {

using ir::Instruction;
using ir::IrType;
using ir::Op;
using ir::Operand;

[[noreturn]] void NotYet(const std::string& what) {
    throw std::runtime_error("not in the RISC-V backend yet: " + what);
}

// ---- names (the only place that spells IR names as assembler) ----
// An IR code label ("else3") and a compiler-made data symbol ("str.0") become
// local assembler labels with the ".L" prefix, which stay out of the symbol
// table. A C-named global or function keeps its name.

std::string LocalLabel(const std::string& name) { return ".L" + name; }

class SymbolNames {
public:
    explicit SymbolNames(const ir::Module& module) {
        for (const ir::DataEntry& d : module.data) if (d.compiler_made) local_.insert(d.label);
    }
    std::string Spell(const std::string& name) const {
        return local_.count(name) ? LocalLabel(name) : name;
    }

private:
    std::set<std::string> local_;
};

int RoundUp(int value, int multiple) { return (value + multiple - 1) / multiple * multiple; }

// ---- memory access helpers ----

bool FitsImm12(int v) { return v >= -2048 && v <= 2047; }

// 'op reg, off(base)'. If off does not fit 12 bits, the address is built in
// t6 first.
void EmitMem(std::ostream& os, const std::string& op, const std::string& reg,
             const std::string& base, int off) {
    if (FitsImm12(off)) {
        os << "\t" << op << " " << reg << ", " << off << "(" << base << ")\n";
        return;
    }
    os << "\tli t6, " << off << "\n";
    os << "\tadd t6, " << base << ", t6\n";
    os << "\t" << op << " " << reg << ", 0(t6)\n";
}

// dst = base + off, for any offset.
void EmitAddrOffset(std::ostream& os, const std::string& dst,
                    const std::string& base, int off) {
    if (off == 0 && dst == base) return;
    if (FitsImm12(off)) {
        os << "\taddi " << dst << ", " << base << ", " << off << "\n";
        return;
    }
    os << "\tli t6, " << off << "\n";
    os << "\tadd " << dst << ", " << base << ", t6\n";
}

// A struct whose alignment is 4 or more is copied a word at a time (its size
// is a multiple of its alignment), any other a byte at a time.
int CopyUnit(int align) { return align >= 4 ? 4 : 1; }

// Copies a struct or union of 'bytes' bytes and alignment 'align' from the
// address in 'src' to the address in 'dst' (D19). Handles any size; a large
// one uses a loop. Clobbers t0, t3, t4 and t5, so 'dst' and 'src' must be
// other registers; both stay unchanged.
void EmitCopyMem(std::ostream& os, const std::string& dst,
                 const std::string& src, int bytes, int align) {
    const int kUnrolledMax = 16;  // units copied with straight-line code
    int unit = CopyUnit(align);
    const char* load = unit == 4 ? "lw" : "lbu";
    const char* store = unit == 4 ? "sw" : "sb";
    int count = bytes / unit;
    if (count <= kUnrolledMax) {
        for (int i = 0; i < count; ++i) {
            os << "\t" << load << " t0, " << i * unit << "(" << src << ")\n";
            os << "\t" << store << " t0, " << i * unit << "(" << dst << ")\n";
        }
        return;
    }
    os << "\tmv t3, " << dst << "\n\tmv t4, " << src << "\n";
    os << "\tli t5, " << bytes << "\n\tadd t5, t3, t5\n";
    os << "1:\n\t" << load << " t0, 0(t4)\n\t" << store << " t0, 0(t3)\n";
    os << "\taddi t3, t3, " << unit << "\n\taddi t4, t4, " << unit << "\n";
    os << "\tbne t3, t5, 1b\n";
}

// Loads 'bytes' (1 to 4) bytes of an integer piece of a struct, zero- or
// sign-extended, into 'reg' from off(base). A whole aligned word is one 'lw';
// anything else is built byte by byte, so it never reads past the object or
// reads a misaligned word. 'align' is the alignment of the struct. Clobbers
// t0; 'reg' and 'base' must not be t0.
void EmitLoadBytes(std::ostream& os, const std::string& reg, int bytes,
                   bool is_signed, int align, const std::string& base, int off) {
    if (bytes == 4 && align >= 4) return EmitMem(os, "lw", reg, base, off);
    if (bytes == 1) return EmitMem(os, is_signed ? "lb" : "lbu", reg, base, off);
    EmitMem(os, "lbu", reg, base, off);
    for (int i = 1; i < bytes; ++i) {
        // The top byte of a signed 2-byte value is loaded signed, which
        // extends the sign through the register.
        bool top_signed = is_signed && bytes == 2 && i == 1;
        EmitMem(os, top_signed ? "lb" : "lbu", "t0", base, off + i);
        os << "\tslli t0, t0, " << 8 * i << "\n\tor " << reg << ", " << reg
           << ", t0\n";
    }
}

// The matching store of the low 'bytes' bytes of 'reg'. Clobbers t0; 'reg'
// and 'base' must not be t0.
void EmitStoreBytes(std::ostream& os, const std::string& reg, int bytes,
                    int align, const std::string& base, int off) {
    if (bytes == 4 && align >= 4) return EmitMem(os, "sw", reg, base, off);
    EmitMem(os, "sb", reg, base, off);
    for (int i = 1; i < bytes; ++i) {
        os << "\tsrli t0, " << reg << ", " << 8 * i << "\n";
        EmitMem(os, "sb", "t0", base, off + i);
    }
}

// ---- frame layout (21.6) ----
// Below s0: saved ra and s0, then the frame objects (locals and parameter
// homes), then one slot per temp, then, at sp, the outgoing argument area.

struct Frame {
    std::map<std::string, int> objects;  // name -> offset from s0
    std::vector<int> temps;              // temp id -> offset from s0
    int return_pointer = 0;              // the saved hidden result pointer, if any
    int size = 0;                        // multiple of the stack alignment

    int used = 8;  // bytes below s0 handed out so far (ra and s0)

    int Reserve(int bytes, int align) {
        used = RoundUp(used + bytes, align);
        return -used;
    }
};

Frame LayoutFrame(const ir::Function& fn, int outgoing_bytes, bool hidden_pointer) {
    Frame frame;
    if (hidden_pointer) frame.return_pointer = frame.Reserve(Target().pointer_size, Target().pointer_size);
    for (const ir::FrameObject& o : fn.objects) frame.objects[o.name] = frame.Reserve(o.size, o.align);
    for (ir::IrType t : fn.temps) frame.temps.push_back(frame.Reserve(ir::SizeOf(t), ir::SizeOf(t)));
    frame.size = RoundUp(frame.used + outgoing_bytes, Target().stack_align);
    return frame;
}

// ---- operands ----

const char* LoadOp(IrType type) {
    switch (type) {
        case IrType::I8: return "lbu";  // a narrow temp holds bits; sext/zext widens
        case IrType::I16: return "lhu";
        case IrType::I32:
        case IrType::Ptr: return "lw";
        case IrType::F32: return "flw";
        case IrType::F64: return "fld";
        default: NotYet("64-bit operand");
    }
}

const char* StoreOp(IrType type) {
    switch (type) {
        case IrType::I8: return "sb";
        case IrType::I16: return "sh";
        case IrType::I32:
        case IrType::Ptr: return "sw";
        case IrType::F32: return "fsw";
        case IrType::F64: return "fsd";
        default: NotYet("64-bit result");
    }
}

// The scratch register n (0 or 1) for a value of this type: t0/t1 or ft0/ft1.
std::string Scratch(IrType type, int n) {
    return std::string(ir::IsFloatType(type) ? "ft" : "t") + std::to_string(n);
}

// The instruction suffix of a floating type: .s (single) or .d (double).
const char* FSuffix(IrType type) { return type == IrType::F32 ? ".s" : ".d"; }

class FunctionWriter {
public:
    // One Classify call for the function's own entry and one for each call
    // (D5, 21.5); the frame layout needs the calls' stack sizes first.
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
        os_ << "\t.text\n";
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

    void WritePrologue() {
        int size = frame_.size;
        EmitAddrOffset(os_, "sp", "sp", -size);
        EmitMem(os_, "sw", "ra", "sp", size - 4);
        EmitMem(os_, "sw", "s0", "sp", size - 8);
        EmitAddrOffset(os_, "s0", "sp", size);
        if (entry_.ret.in_memory) {  // the caller's result object, kept for 'ret'
            EmitMem(os_, "sw", IntRegName(entry_.ret.pointer), "s0", frame_.return_pointer);
        }
        StoreParameters();
    }

    static std::string IntRegName(const Piece& piece) { return "a" + std::to_string(piece.index); }
    static std::string FloatRegName(const Piece& piece) { return "fa" + std::to_string(piece.index); }
    // The register of a scalar's single piece (an argument or a return value).
    static std::string RegName(const Piece& piece) {
        return piece.where == Piece::Where::FloatReg ? FloatRegName(piece) : IntRegName(piece);
    }

    // Each incoming piece goes to its parameter's home. A stack piece is in
    // the caller's outgoing area, which starts at s0.
    void StoreParameters() {
        for (size_t i = 0; i < fn_.params.size(); ++i) {
            int home = frame_.objects.at(fn_.params[i].object);
            const Type& type = *fn_.params[i].type;
            if (type.IsRecord()) {
                StoreRecordParameter(entry_.args[i], type, home);
                continue;
            }
            for (const Piece& piece : entry_.args[i].pieces) StoreIncomingPiece(piece, home);
        }
    }

    // ---- structs and unions (D19, 21.5) ----
    // A struct is always memory in the IR: its value is the address of an
    // object. Only at a call, an entry and a return is it split into pieces,
    // each with its byte offset inside the struct.

    // A piece that is a whole record on the stack, not a part of one.
    static bool IsWholeStackRecord(const Piece& piece, const Type& record) {
        return piece.where == Piece::Where::StackWord && piece.offset == 0 && piece.bytes == record.Size();
    }

    // The pointer of a by-reference argument, as it arrives.
    void LoadIncomingPointer(const std::string& reg, const Piece& piece) {
        if (piece.where == Piece::Where::IntReg) {
            os_ << "\tmv " << reg << ", " << IntRegName(piece) << "\n";
        } else {
            EmitMem(os_, "lw", reg, "s0", piece.index);
        }
    }

    // By reference: copy from the caller's copy into the home. Otherwise each
    // piece is stored at its offset in the home (a stack word is read from the
    // caller's outgoing area at s0 + index).
    void StoreRecordParameter(const ArgPlace& place, const Type& type, int home) {
        if (place.by_reference) {
            LoadIncomingPointer("t1", place.pieces.at(0));
            EmitAddrOffset(os_, "t2", "s0", home);
            return EmitCopyMem(os_, "t2", "t1", type.Size(), type.Align());
        }
        for (const Piece& piece : place.pieces) StoreIncomingRecordPiece(piece, type, home);
    }

    void StoreIncomingRecordPiece(const Piece& piece, const Type& type, int home) {
        int dst = home + piece.offset;
        switch (piece.where) {
            case Piece::Where::FloatReg:
                return EmitMem(os_, piece.bytes == 8 ? "fsd" : "fsw", FloatRegName(piece), "s0", dst);
            case Piece::Where::IntReg:
                return EmitStoreBytes(os_, IntRegName(piece), piece.bytes, type.Align(), "s0", dst);
            case Piece::Where::StackWord:
                if (IsWholeStackRecord(piece, type)) {
                    EmitAddrOffset(os_, "t1", "s0", piece.index);
                    EmitAddrOffset(os_, "t2", "s0", home);
                    return EmitCopyMem(os_, "t2", "t1", piece.bytes, type.Align());
                }
                EmitMem(os_, "lw", "t1", "s0", piece.index);  // the high part after an a7 split
                return EmitStoreBytes(os_, "t1", piece.bytes, type.Align(), "s0", dst);
        }
    }

    // The caller: the operand is the address of the argument's copy. By
    // reference it is passed as it is; otherwise the pieces are loaded from it.
    void PlaceOutgoingRecord(const Operand& o, const ArgPlace& place, const Type& type) {
        if (place.by_reference) return PlaceOutgoingPointer(o, place.pieces.at(0));
        Load("t1", o);
        for (const Piece& piece : place.pieces) PlaceOutgoingRecordPiece(piece, type);
    }

    void PlaceOutgoingPointer(const Operand& o, const Piece& piece) {
        if (piece.where == Piece::Where::IntReg) return Load(IntRegName(piece), o);
        Load("t0", o);
        EmitMem(os_, "sw", "t0", "sp", piece.index);
    }

    // The struct's address is in t1.
    void PlaceOutgoingRecordPiece(const Piece& piece, const Type& type) {
        switch (piece.where) {
            case Piece::Where::FloatReg:
                return EmitMem(os_, piece.bytes == 8 ? "fld" : "flw", FloatRegName(piece), "t1", piece.offset);
            case Piece::Where::IntReg:
                return EmitLoadBytes(os_, IntRegName(piece), piece.bytes, piece.is_signed, type.Align(), "t1", piece.offset);
            case Piece::Where::StackWord:
                if (IsWholeStackRecord(piece, type)) {
                    EmitAddrOffset(os_, "t2", "sp", piece.index);
                    return EmitCopyMem(os_, "t2", "t1", piece.bytes, type.Align());
                }
                EmitLoadBytes(os_, "t2", piece.bytes, false, type.Align(), "t1", piece.offset);
                return EmitMem(os_, "sw", "t2", "sp", piece.index);
        }
    }

    // After the call: the returned pieces are in a0, a1, fa0, fa1; store them
    // into the call's result object. A hidden pointer needs nothing.
    void SaveReturnedRecord(const RetPlace& ret, const Instruction& in) {
        if (ret.in_memory) return;
        const Type& type = *in.ret_type;
        Load("t1", in.result_object);
        for (const Piece& piece : ret.pieces) {
            if (piece.where == Piece::Where::FloatReg) {
                EmitMem(os_, piece.bytes == 8 ? "fsd" : "fsw", FloatRegName(piece), "t1", piece.offset);
            } else {
                EmitStoreBytes(os_, IntRegName(piece), piece.bytes, type.Align(), "t1", piece.offset);
            }
        }
    }

    // 'ret' of a struct: the operand is the address of the value. Through the
    // hidden pointer it is copied there (and the pointer comes back in a0);
    // otherwise the pieces are loaded into the return registers.
    void LoadReturnRecord(const Operand& value) {
        const Type& type = *fn_.c_return;
        if (entry_.ret.in_memory) {
            EmitMem(os_, "lw", "t1", "s0", frame_.return_pointer);
            Load("t2", value);
            EmitCopyMem(os_, "t1", "t2", type.Size(), type.Align());
            os_ << "\tmv " << IntRegName(entry_.ret.pointer) << ", t1\n";
            return;
        }
        Load("t1", value);
        for (const Piece& piece : entry_.ret.pieces) PlaceOutgoingRecordPiece(piece, type);
    }

    // An fa register is stored as a float or double; an a register as a word
    // (a double in a register pair arrives as two pieces); a stack word is
    // copied through t0, and an 8-byte stack piece (a double) through ft0.
    void StoreIncomingPiece(const Piece& piece, int home) {
        int dst = home + piece.offset;
        switch (piece.where) {
            case Piece::Where::FloatReg:
                EmitMem(os_, piece.bytes == 8 ? "fsd" : "fsw", "fa" + std::to_string(piece.index), "s0", dst);
                return;
            case Piece::Where::IntReg:
                EmitMem(os_, WordStore(piece.bytes), "a" + std::to_string(piece.index), "s0", dst);
                return;
            case Piece::Where::StackWord:
                if (piece.bytes == 8) {
                    EmitMem(os_, "fld", "ft0", "s0", piece.index);
                    EmitMem(os_, "fsd", "ft0", "s0", dst);
                } else {
                    EmitMem(os_, "lw", "t0", "s0", piece.index);
                    EmitMem(os_, WordStore(piece.bytes), "t0", "s0", dst);
                }
        }
    }

    static const char* WordStore(int bytes) {
        if (bytes == 1) return "sb";
        if (bytes == 2) return "sh";
        if (bytes == 4) return "sw";
        NotYet("a piece of " + std::to_string(bytes) + " bytes");
    }

    void WriteEpilogue() {
        EmitMem(os_, "lw", "ra", "s0", -4);
        EmitMem(os_, "lw", "t0", "s0", -8);
        os_ << "\tmv sp, s0\n\tmv s0, t0\n\tret\n";
    }

    void Load(const std::string& reg, const Operand& o) {
        switch (o.kind) {
            case Operand::Kind::Temp: return EmitMem(os_, LoadOp(o.type), reg, "s0", frame_.temps[o.temp]);
            case Operand::Kind::Int:
                if (ir::SizeOf(o.type) > 4) NotYet("64-bit constant");
                os_ << "\tli " << reg << ", " << static_cast<int32_t>(o.int_value) << "\n";
                return;
            case Operand::Kind::Object: return EmitAddrOffset(os_, reg, "s0", frame_.objects.at(o.name));
            case Operand::Kind::Symbol: os_ << "\tla " << reg << ", " << names_.Spell(o.name) << "\n"; return;
            default: NotYet("floating constant");
        }
    }

    // Loads an operand as the psABI wants a value of this piece in a register
    // or stack word: a narrow integer extended to 32 bits by its C signedness
    // (which the IR type does not know).
    void LoadForPiece(const std::string& reg, const Operand& o, const Piece& piece) {
        if (ir::IsFloatType(o.type)) return LoadFloatWord(reg, o, piece);
        int bytes = piece.bytes;
        if (bytes >= 4) return Load(reg, o);
        if (o.kind == Operand::Kind::Int) {
            long mask = (1L << (8 * bytes)) - 1, v = o.int_value & mask;
            if (piece.is_signed && (v >> (8 * bytes - 1)) & 1) v -= mask + 1;
            os_ << "\tli " << reg << ", " << v << "\n";
            return;
        }
        if (o.kind != Operand::Kind::Temp) NotYet("narrow operand that is not a temporary");
        const char* op = bytes == 1 ? (piece.is_signed ? "lb" : "lbu") : (piece.is_signed ? "lh" : "lhu");
        EmitMem(os_, op, reg, "s0", frame_.temps[o.temp]);
    }

    // A float or double travelling in an integer register or stack word: the
    // word of its slot at the piece's byte offset (4 bytes, so a float is one
    // word and a double is two pieces).
    void LoadFloatWord(const std::string& reg, const Operand& o, const Piece& piece) {
        if (o.kind != Operand::Kind::Temp) NotYet("floating operand that is not a temporary");
        EmitMem(os_, "lw", reg, "s0", frame_.temps[o.temp] + piece.offset);
    }

    // One piece of an argument goes to its register or outgoing stack word.
    void PlaceOutgoingPiece(const Operand& o, const Piece& piece) {
        switch (piece.where) {
            case Piece::Where::FloatReg: return Load("fa" + std::to_string(piece.index), o);
            case Piece::Where::IntReg: return LoadForPiece("a" + std::to_string(piece.index), o, piece);
            case Piece::Where::StackWord:
                if (piece.bytes == 8) {
                    Load("ft0", o);
                    return EmitMem(os_, "fsd", "ft0", "sp", piece.index);
                }
                LoadForPiece("t0", o, piece);
                return EmitMem(os_, "sw", "t0", "sp", piece.index);
        }
    }

    // Arguments go to fa and a registers and outgoing stack words, then the
    // call; a scalar result comes back in the register the classifier names for
    // it (a0, already extended, and its slot keeps the bits; or fa0).
    void WriteCall(size_t index, const Instruction& in) {
        const CallLayout& layout = calls_.at(index);
        if (layout.ret.in_memory) Load(IntRegName(layout.ret.pointer), in.result_object);
        for (size_t i = 0; i < in.args.size(); ++i) {
            if (in.arg_types[i]->IsRecord()) {
                PlaceOutgoingRecord(in.args[i], layout.args[i], *in.arg_types[i]);
                continue;
            }
            for (const Piece& piece : layout.args[i].pieces) PlaceOutgoingPiece(in.args[i], piece);
        }
        os_ << "\tcall " << in.callee << "\n";
        if (in.dst.kind == Operand::Kind::Temp) {
            StoreResult(RegName(layout.ret.pieces.at(0)), in.dst);
        } else if (in.ret_type->IsRecord()) {
            SaveReturnedRecord(layout.ret, in);
        }
    }

    void LoadReturnValue(const Operand& value) {
        if (fn_.c_return->IsRecord()) return LoadReturnRecord(value);
        const Piece& piece = entry_.ret.pieces.at(0);
        if (piece.where == Piece::Where::FloatReg) return Load(FloatRegName(piece), value);
        LoadForPiece(IntRegName(piece), value, piece);
    }

    void StoreResult(const std::string& reg, const Operand& dst) {
        EmitMem(os_, StoreOp(dst.type), reg, "s0", frame_.temps[dst.temp]);
    }

    // ---- instructions ----

    void Emit3(const char* mnemonic) { os_ << "\t" << mnemonic << " t0, t0, t1\n"; }

    void WriteFloatBinary(const Instruction& in) {
        const char* name = nullptr;
        switch (in.op) {
            case Op::Add: name = "fadd"; break;
            case Op::Sub: name = "fsub"; break;
            case Op::Mul: name = "fmul"; break;
            case Op::Div: name = "fdiv"; break;
            default: NotYet(std::string("floating ") + ir::OpName(in.op));
        }
        Load("ft0", in.args[0]);
        Load("ft1", in.args[1]);
        os_ << "\t" << name << FSuffix(in.type) << " ft0, ft0, ft1\n";
        StoreResult("ft0", in.dst);
    }

    void WriteBinary(const Instruction& in) {
        if (ir::IsFloatType(in.type)) return WriteFloatBinary(in);
        Load("t0", in.args[0]);
        Load("t1", in.args[1]);
        switch (in.op) {
            case Op::Add: Emit3("add"); break;
            case Op::Sub: Emit3("sub"); break;
            case Op::Mul: Emit3("mul"); break;
            case Op::Div: Emit3("div"); break;
            case Op::DivU: Emit3("divu"); break;
            case Op::Rem: Emit3("rem"); break;
            case Op::RemU: Emit3("remu"); break;
            case Op::And: Emit3("and"); break;
            case Op::Or: Emit3("or"); break;
            case Op::Xor: Emit3("xor"); break;
            case Op::Shl: Emit3("sll"); break;
            case Op::Shr: Emit3("srl"); break;
            case Op::Sar: Emit3("sra"); break;
            default: break;
        }
        StoreResult("t0", in.dst);
    }

    // a <= b is not (b < a).
    // feq, flt and fle give 0 for NaN, as C needs; != is built from feq by the frontend.
    void WriteFloatCompare(const Instruction& in) {
        const char* name = in.op == Op::FEq ? "feq" : in.op == Op::FLt ? "flt" : "fle";
        Load("ft0", in.args[0]);
        Load("ft1", in.args[1]);
        os_ << "\t" << name << FSuffix(in.type) << " t0, ft0, ft1\n";
        StoreResult("t0", in.dst);
    }

    void WriteCompare(const Instruction& in) {
        if (ir::IsFloatType(in.type)) return WriteFloatCompare(in);
        Load("t0", in.args[0]);
        Load("t1", in.args[1]);
        switch (in.op) {
            case Op::Eq: os_ << "\txor t0, t0, t1\n\tseqz t0, t0\n"; break;
            case Op::Ne: os_ << "\txor t0, t0, t1\n\tsnez t0, t0\n"; break;
            case Op::Lt: os_ << "\tslt t0, t0, t1\n"; break;
            case Op::LtU: os_ << "\tsltu t0, t0, t1\n"; break;
            case Op::Le: os_ << "\tslt t0, t1, t0\n\txori t0, t0, 1\n"; break;
            case Op::LeU: os_ << "\tsltu t0, t1, t0\n\txori t0, t0, 1\n"; break;
            default: NotYet(ir::OpName(in.op));
        }
        StoreResult("t0", in.dst);
    }

    // Integer sources and results are 32-bit (the frontend widens narrow ones);
    // float to int rounds toward zero, as C requires.
    void WriteFloatConversion(const Instruction& in) {
        IrType from = in.args[0].type;
        if (ir::IsIntType(from) && ir::SizeOf(from) != 4) NotYet("itof of a non-32-bit integer");
        if (ir::IsIntType(in.type) && ir::SizeOf(in.type) != 4) NotYet("ftoi to a non-32-bit integer");
        switch (in.op) {
            case Op::IToF:
            case Op::UToF:
                Load("t0", in.args[0]);
                os_ << "\tfcvt" << FSuffix(in.type) << (in.op == Op::IToF ? ".w" : ".wu") << " ft0, t0\n";
                return StoreResult("ft0", in.dst);
            case Op::FToI:
            case Op::FToU:
                Load("ft0", in.args[0]);
                os_ << "\tfcvt" << (in.op == Op::FToI ? ".w" : ".wu") << FSuffix(from) << " t0, ft0, rtz\n";
                return StoreResult("t0", in.dst);
            default:  // FExt, FTrunc
                Load("ft0", in.args[0]);
                os_ << "\tfcvt" << FSuffix(in.type) << FSuffix(from) << " ft0, ft0\n";
                return StoreResult("ft0", in.dst);
        }
    }

    void WriteConversion(const Instruction& in) {
        if (in.op >= Op::IToF && in.op <= Op::FTrunc) return WriteFloatConversion(in);
        Load("t0", in.args[0]);
        int shift = 32 - 8 * ir::SizeOf(in.args[0].type);
        switch (in.op) {
            case Op::SExt: os_ << "\tslli t0, t0, " << shift << "\n\tsrai t0, t0, " << shift << "\n"; break;
            case Op::ZExt: os_ << "\tslli t0, t0, " << shift << "\n\tsrli t0, t0, " << shift << "\n"; break;
            case Op::Trunc: break;  // the store keeps the low bytes
            case Op::PtrToInt:
            case Op::IntToPtr: break;  // same width on this target
            default: NotYet(ir::OpName(in.op));
        }
        StoreResult("t0", in.dst);
    }

    void WriteAddr(const Instruction& in) {
        Load("t0", in.args[0]);
        if (in.offset) EmitAddrOffset(os_, "t0", "t0", static_cast<int>(in.offset));
        StoreResult("t0", in.dst);
    }

    // Zeroes in words when the address and the size allow it, else in bytes:
    // unrolled when short, otherwise a loop from t0 up to t1 (a local label).
    void WriteMemZero(const Instruction& in) {
        Load("t0", in.args[0]);
        bool word = in.align >= 4 && in.size % 4 == 0;
        const char* store = word ? "sw" : "sb";
        int step = word ? 4 : 1;
        if (in.size <= 64) {
            for (long off = 0; off < in.size; off += step) EmitMem(os_, store, "zero", "t0", static_cast<int>(off));
            return;
        }
        os_ << "\tli t1, " << in.size << "\n\tadd t1, t0, t1\n";
        os_ << "1:\n\t" << store << " zero, 0(t0)\n\taddi t0, t0, " << step << "\n\tbne t0, t1, 1b\n";
    }

    // dst in t1 and src in t2; EmitCopyMem uses t0, t3, t4 and t5 as its own.
    void WriteMemCopy(const Instruction& in) {
        Load("t1", in.args[0]);
        Load("t2", in.args[1]);
        EmitCopyMem(os_, "t1", "t2", static_cast<int>(in.size), static_cast<int>(in.align));
    }

    void WriteInstruction(size_t index, const Instruction& in) {
        if (ir::IsBinaryArith(in.op)) return WriteBinary(in);
        if (ir::IsCompare(in.op)) return WriteCompare(in);
        if (ir::IsConversion(in.op)) return WriteConversion(in);
        switch (in.op) {
            case Op::Copy:
            case Op::Const:
                Load(Scratch(in.type, 0), in.args[0]);
                return StoreResult(Scratch(in.type, 0), in.dst);
            case Op::PtrAdd:
                Load("t0", in.args[0]);
                Load("t1", in.args[1]);
                Emit3("add");
                return StoreResult("t0", in.dst);
            case Op::Neg:
                Load(Scratch(in.type, 0), in.args[0]);
                if (ir::IsFloatType(in.type)) {
                    os_ << "\tfneg" << FSuffix(in.type) << " ft0, ft0\n";
                    return StoreResult("ft0", in.dst);
                }
                os_ << "\tneg t0, t0\n";
                return StoreResult("t0", in.dst);
            case Op::Not:
                Load("t0", in.args[0]);
                os_ << "\tnot t0, t0\n";
                return StoreResult("t0", in.dst);
            case Op::Addr: return WriteAddr(in);
            case Op::Load:
                Load("t0", in.args[0]);
                EmitMem(os_, LoadOp(in.type), Scratch(in.type, 0), "t0", 0);
                return StoreResult(Scratch(in.type, 0), in.dst);
            case Op::Store:
                Load(Scratch(in.type, 0), in.args[0]);
                Load("t1", in.args[1]);
                return EmitMem(os_, StoreOp(in.type), Scratch(in.type, 0), "t1", 0);
            case Op::MemZero: return WriteMemZero(in);
            case Op::MemCpy: return WriteMemCopy(in);
            case Op::Label: os_ << LocalLabel(in.label) << ":\n"; return;
            case Op::Jump: os_ << "\tj " << LocalLabel(in.label) << "\n"; return;
            case Op::Branch:
                Load("t0", in.args[0]);
                // 'j' reaches the whole function; a conditional branch only +-4 KiB.
                os_ << "\tbeqz t0, 1f\n\tj " << LocalLabel(in.label) << "\n1:\n\tj "
                    << LocalLabel(in.label2) << "\n";
                return;
            case Op::Call: return WriteCall(index, in);
            case Op::Ret:
                if (!in.args.empty()) LoadReturnValue(in.args[0]);
                os_ << "\tj " << ReturnLabel() << "\n";
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
        case Kind::Int:
            if (item.size == 8) {  // two words, low first (little endian)
                os << "\t.word " << (item.value & 0xFFFFFFFFLL) << "\n";
                os << "\t.word " << ((item.value >> 32) & 0xFFFFFFFFLL) << "\n";
                break;
            }
            if (item.size != 1 && item.size != 2 && item.size != 4) NotYet("integer data item size");
            os << (item.size == 1 ? "\t.byte " : item.size == 2 ? "\t.half " : "\t.word ")
               << item.value << "\n";
            break;
        case Kind::Float:
            os << "\t.word " << (item.value & 0xFFFFFFFFULL) << "\n";
            if (item.size == 8) os << "\t.word " << ((item.value >> 32) & 0xFFFFFFFFULL) << "\n";
            break;
        case Kind::Address:
            os << "\t.word " << names.Spell(item.bytes) << (item.offset ? (item.offset < 0 ? "-" : "+") + std::to_string(std::labs(item.offset)) : "") << "\n";
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

void EmitRiscv(const ir::Module& module, std::ostream& os) {
    SymbolNames names(module);
    for (const ir::Function& fn : module.functions) FunctionWriter(fn, names, os).Write();
    WriteData(module, names, os);
}
