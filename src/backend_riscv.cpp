#include "backend.hpp"

#include <cstdint>
#include <map>
#include <stdexcept>

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

int RoundUp(int value, int multiple) { return (value + multiple - 1) / multiple * multiple; }

// ---- frame layout (21.6) ----
// Below s0: saved ra and s0, then the frame objects, then one slot per temp.
// (Register-parameter homes and the outgoing argument area arrive with calls.)

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

Frame LayoutFrame(const ir::Function& fn) {
    Frame frame;
    for (const ir::FrameObject& o : fn.objects) frame.objects[o.name] = frame.Reserve(o.size, o.align);
    for (ir::IrType t : fn.temps) frame.temps.push_back(frame.Reserve(ir::SizeOf(t), ir::SizeOf(t)));
    frame.size = RoundUp(frame.used, Target().stack_align);
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
    FunctionWriter(const ir::Function& fn, std::ostream& os)
        : fn_(fn), os_(os), frame_(LayoutFrame(fn)) {}

    void Write() {
        os_ << "\t.text\n";
        if (fn_.is_global) os_ << "\t.globl " << fn_.name << "\n";
        os_ << fn_.name << ":\n";
        WritePrologue();
        for (const Instruction& in : fn_.body) WriteInstruction(in);
        os_ << ReturnLabel() << ":\n";
        WriteEpilogue();
    }

private:
    std::string ReturnLabel() const { return ".Lret_" + fn_.name; }

    void WritePrologue() {
        int size = frame_.size;
        EmitAddrOffset(os_, "sp", "sp", -size);
        EmitMem(os_, "sw", "ra", "sp", size - 4);
        EmitMem(os_, "sw", "s0", "sp", size - 8);
        EmitAddrOffset(os_, "s0", "sp", size);
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
            case Operand::Kind::Symbol: os_ << "\tla " << reg << ", " << o.name << "\n"; return;
            default: NotYet("floating constant");
        }
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

    void WriteInstruction(const Instruction& in) {
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
            case Op::Label: os_ << in.label << ":\n"; return;
            case Op::Jump: os_ << "\tj " << in.label << "\n"; return;
            case Op::Branch:
                Load("t0", in.args[0]);
                os_ << "\tbnez t0, " << in.label << "\n\tj " << in.label2 << "\n";
                return;
            case Op::Ret:
                if (!in.args.empty()) Load("a0", in.args[0]);
                os_ << "\tj " << ReturnLabel() << "\n";
                return;
            default: NotYet(ir::OpName(in.op));
        }
    }

    const ir::Function& fn_;
    std::ostream& os_;
    Frame frame_;
};

// ---- data (21.2a): the only place that writes data directives ----

void WriteItem(const ir::DataItem& item, std::ostream& os) {
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
            os << "\t.word " << item.bytes << (item.offset ? "+" + std::to_string(item.offset) : "") << "\n";
            break;
        case Kind::Zero: os << "\t.zero " << item.size << "\n"; break;
    }
}

void WriteData(const ir::Module& module, std::ostream& os) {
    for (const ir::DataEntry& d : module.data) {
        os << (d.read_only ? "\t.section .rodata\n" : "\t.data\n");
        os << "\t.balign " << d.align << "\n";
        if (!d.internal) os << "\t.globl " << d.label << "\n";
        os << d.label << ":\n";
        if (d.items.empty()) os << "\t.zero " << d.size << "\n";
        for (const ir::DataItem& item : d.items) WriteItem(item, os);
    }
}

}  // namespace

void EmitRiscv(const ir::Module& module, std::ostream& os) {
    for (const ir::Function& fn : module.functions) FunctionWriter(fn, os).Write();
    WriteData(module, os);
}
