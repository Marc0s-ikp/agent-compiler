#include "backend.hpp"

#include <algorithm>
#include <cstdint>
#include <map>
#include <set>
#include <stdexcept>

#include "call_abi.hpp"
#include "emit.hpp"
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

// ---- frame layout (21.6) ----
// Below s0: saved ra and s0, then the frame objects (locals and parameter
// homes), then one slot per temp, then, at sp, the outgoing argument area.

struct Frame {
    std::map<std::string, int> objects;  // name -> offset from s0
    std::vector<int> temps;              // temp id -> offset from s0
    int size = 0;                        // multiple of the stack alignment

    int used = 8;  // bytes below s0 handed out so far (ra and s0)

    int Reserve(int bytes, int align) {
        used = RoundUp(used + bytes, align);
        return -used;
    }
};

Frame LayoutFrame(const ir::Function& fn, int outgoing_bytes) {
    Frame frame;
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
        default: NotYet("64-bit or floating operand");
    }
}

const char* StoreOp(IrType type) {
    switch (type) {
        case IrType::I8: return "sb";
        case IrType::I16: return "sh";
        case IrType::I32:
        case IrType::Ptr: return "sw";
        default: NotYet("64-bit or floating result");
    }
}

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
        frame_ = LayoutFrame(fn, outgoing);
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
        StoreParameters();
    }

    // Each incoming piece goes to its parameter's home. A stack piece is in
    // the caller's outgoing area, which starts at s0.
    void StoreParameters() {
        for (size_t i = 0; i < fn_.params.size(); ++i) {
            int home = frame_.objects.at(fn_.params[i].object);
            for (const Piece& piece : entry_.args[i].pieces) {
                std::string reg = "t0";
                if (piece.where == Piece::Where::IntReg) {
                    reg = "a" + std::to_string(piece.index);
                } else {
                    EmitMem(os_, "lw", "t0", "s0", piece.index);
                }
                EmitMem(os_, WordStore(piece.bytes), reg, "s0", home + piece.offset);
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

    // Arguments go to a registers and outgoing stack words, then the call; the
    // result comes back in a0, already extended, and its slot keeps the bits.
    void WriteCall(size_t index, const Instruction& in) {
        const CallLayout& layout = calls_.at(index);
        for (size_t i = 0; i < in.args.size(); ++i) {
            for (const Piece& piece : layout.args[i].pieces) {
                if (piece.where == Piece::Where::IntReg) {
                    LoadForPiece("a" + std::to_string(piece.index), in.args[i], piece);
                } else {
                    LoadForPiece("t0", in.args[i], piece);
                    EmitMem(os_, "sw", "t0", "sp", piece.index);
                }
            }
        }
        os_ << "\tcall " << in.callee << "\n";
        if (in.dst.kind == Operand::Kind::Temp) StoreResult("a0", in.dst);
    }

    void StoreResult(const std::string& reg, const Operand& dst) {
        EmitMem(os_, StoreOp(dst.type), reg, "s0", frame_.temps[dst.temp]);
    }

    // ---- instructions ----

    void Emit3(const char* mnemonic) { os_ << "\t" << mnemonic << " t0, t0, t1\n"; }

    void WriteBinary(const Instruction& in) {
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
    void WriteCompare(const Instruction& in) {
        Load("t0", in.args[0]);
        Load("t1", in.args[1]);
        switch (in.op) {
            case Op::Eq: os_ << "\txor t0, t0, t1\n\tseqz t0, t0\n"; break;
            case Op::Ne: os_ << "\txor t0, t0, t1\n\tsnez t0, t0\n"; break;
            case Op::Lt: os_ << "\tslt t0, t0, t1\n"; break;
            case Op::LtU: os_ << "\tsltu t0, t0, t1\n"; break;
            case Op::Le: os_ << "\tslt t0, t1, t0\n\txori t0, t0, 1\n"; break;
            case Op::LeU: os_ << "\tsltu t0, t1, t0\n\txori t0, t0, 1\n"; break;
            default: NotYet("floating compare");
        }
        StoreResult("t0", in.dst);
    }

    void WriteConversion(const Instruction& in) {
        Load("t0", in.args[0]);
        int shift = 32 - 8 * ir::SizeOf(in.args[0].type);
        switch (in.op) {
            case Op::SExt: os_ << "\tslli t0, t0, " << shift << "\n\tsrai t0, t0, " << shift << "\n"; break;
            case Op::ZExt: os_ << "\tslli t0, t0, " << shift << "\n\tsrli t0, t0, " << shift << "\n"; break;
            case Op::Trunc: break;  // the store keeps the low bytes
            default: NotYet("floating conversion");
        }
        StoreResult("t0", in.dst);
    }

    void WriteAddr(const Instruction& in) {
        Load("t0", in.args[0]);
        if (in.offset) EmitAddrOffset(os_, "t0", "t0", static_cast<int>(in.offset));
        StoreResult("t0", in.dst);
    }

    void WriteInstruction(size_t index, const Instruction& in) {
        if (ir::IsBinaryArith(in.op)) return WriteBinary(in);
        if (ir::IsCompare(in.op)) return WriteCompare(in);
        if (ir::IsConversion(in.op)) return WriteConversion(in);
        switch (in.op) {
            case Op::Copy:
            case Op::Const:
                Load("t0", in.args[0]);
                return StoreResult("t0", in.dst);
            case Op::Neg:
                Load("t0", in.args[0]);
                os_ << "\tneg t0, t0\n";
                return StoreResult("t0", in.dst);
            case Op::Not:
                Load("t0", in.args[0]);
                os_ << "\tnot t0, t0\n";
                return StoreResult("t0", in.dst);
            case Op::Addr: return WriteAddr(in);
            case Op::Load:
                Load("t0", in.args[0]);
                EmitMem(os_, LoadOp(in.type), "t0", "t0", 0);
                return StoreResult("t0", in.dst);
            case Op::Store:
                Load("t0", in.args[0]);
                Load("t1", in.args[1]);
                return EmitMem(os_, StoreOp(in.type), "t0", "t1", 0);
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
                if (!in.args.empty()) LoadForPiece("a0", in.args[0], entry_.ret.pieces.at(0));
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
            os << "\t.word " << names.Spell(item.bytes) << (item.offset ? "+" + std::to_string(item.offset) : "") << "\n";
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
