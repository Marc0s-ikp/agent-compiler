#include <algorithm>
#include <cstdint>
#include <map>
#include <ostream>
#include <stdexcept>
#include <string>

#include "backend.hpp"
#include "call_abi.hpp"
#include "target.hpp"

// The AArch64 backend (D27, 27.3), AAPCS64 with LP64. It has the same shape as
// the RISC-V one (21.4, 21.6) but shares no code with it: every IR temporary
// owns a stack slot, and each instruction loads its operands into the scratch
// registers x9 and x10 (w9, w10), computes, and stores the result to the
// destination's slot. Nothing is held in a register between instructions.
// x16 and x17 hold addresses and large constants. All four are caller-saved,
// so the prologue saves only the frame record (x29 and x30).
//
// This is step A2: integer operators, assignment, locals and control flow. An instruction
// that belongs to a later step throws "not in AArch64 yet".
namespace {

using ir::Instruction;
using ir::IrType;
using ir::Op;
using ir::Operand;

[[noreturn]] void NotYet(const std::string& what) {
    throw std::runtime_error("not in AArch64 yet: " + what);
}

int RoundUp(int value, int multiple) { return (value + multiple - 1) / multiple * multiple; }

std::string LocalLabel(const std::string& name) { return ".L" + name; }

// ---- registers and constants ----

// True for the types that live in an x register (64 bits); the others use w.
bool IsWide(IrType type) { return type == IrType::I64 || type == IrType::Ptr; }

// Register number n, as w<n> or x<n>, for a value of this IR type.
std::string Reg(int n, IrType type) {
    if (ir::IsFloatType(type)) NotYet("floating registers");
    return std::string(IsWide(type) ? "x" : "w") + std::to_string(n);
}

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
        case IrType::Ptr: return "ldr";
        default: NotYet("floating load");
    }
}

const char* StoreOp(IrType type) {
    switch (type) {
        case IrType::I8: return "strb";
        case IrType::I16: return "strh";
        case IrType::I32:
        case IrType::I64:
        case IrType::Ptr: return "str";
        default: NotYet("floating store");
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
    int upper = 0;                       // size of the upper part
    int outgoing = 0;                    // size of the area at sp

    int used = 16;  // bytes handed out from x29 so far (the frame record)

    int Reserve(int bytes, int align) {
        int offset = RoundUp(used, align);
        used = offset + bytes;
        return offset;
    }
};

Frame LayoutFrame(const ir::Function& fn, int outgoing_bytes) {
    Frame frame;
    for (const ir::FrameObject& o : fn.objects) frame.objects[o.name] = frame.Reserve(o.size, o.align);
    for (IrType t : fn.temps) frame.temps.push_back(frame.Reserve(ir::SizeOf(t), ir::SizeOf(t)));
    frame.upper = RoundUp(frame.used, 16);
    frame.outgoing = RoundUp(outgoing_bytes, 16);
    return frame;
}

class FunctionWriter {
public:
    FunctionWriter(const ir::Function& fn, std::ostream& os) : fn_(fn), os_(os) {
        entry_ = Target().Abi().Classify(ParamTypes(), fn.c_return);
        int outgoing = 0;  // A3: the largest stack_bytes over the function's calls
        frame_ = LayoutFrame(fn, outgoing);
    }

    void Write() {
        os_ << "\t.text\n\t.p2align 2\n";
        if (fn_.is_global) os_ << "\t.globl " << fn_.name << "\n";
        os_ << fn_.name << ":\n";
        WritePrologue();
        for (const Instruction& in : fn_.body) WriteInstruction(in);
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
            case Operand::Kind::Symbol: NotYet("symbol address");
            case Operand::Kind::Float: NotYet("floating constant");
            case Operand::Kind::None: break;
        }
        NotYet("operand");
    }

    void StoreResult(int n, const Operand& dst) {
        EmitMem(os_, StoreOp(dst.type), Reg(n, dst.type), "x29", frame_.temps[dst.temp], ir::SizeOf(dst.type));
    }

    // The value comes back in x0 or w0 (27.2); the upper bits of a narrow
    // value are unspecified, so nothing is extended.
    void LoadReturnValue(const Operand& value) {
        if (entry_.ret.pieces.at(0).where != Piece::Where::IntReg) NotYet("return in a floating register");
        Load(0, value);
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

    void WriteBinary(const Instruction& in) {
        if (ir::IsFloatType(in.type)) NotYet("floating arithmetic");
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

    void WriteCompare(const Instruction& in) {
        if (ir::IsFloatType(in.type)) NotYet("floating compare");
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

    // 'cbz' reaches only +-1 MB, so it skips one 'b' and every jump to a label
    // is a 'b' (+-128 MB). The condition is in w9, or x9 for an I64 or Ptr.
    void WriteBranch(const Instruction& in) {
        Load(9, in.args[0]);
        os_ << "\tcbz " << Reg(9, in.args[0].type) << ", 1f\n\tb " << LocalLabel(in.label)
            << "\n1:\n\tb " << LocalLabel(in.label2) << "\n";
    }

    void WriteInstruction(const Instruction& in) {
        if (ir::IsBinaryArith(in.op)) return WriteBinary(in);
        if (ir::IsCompare(in.op)) return WriteCompare(in);
        if (ir::IsConversion(in.op)) return WriteConversion(in);
        switch (in.op) {
            case Op::Copy:
            case Op::Const:
                Load(9, in.args[0]);
                return StoreResult(9, in.dst);
            case Op::Neg:
            case Op::Not:
                if (ir::IsFloatType(in.type)) NotYet("floating neg");
                Load(9, in.args[0]);
                os_ << "\t" << (in.op == Op::Neg ? "neg " : "mvn ") << Reg(9, in.type) << ", " << Reg(9, in.type) << "\n";
                return StoreResult(9, in.dst);
            case Op::Addr:
                Load(9, in.args[0]);
                EmitAddrOffset(os_, "x9", "x9", in.offset);
                return StoreResult(9, in.dst);
            case Op::Load:
                Load(9, in.args[0]);
                EmitMem(os_, LoadOp(in.type), Reg(9, in.type), "x9", 0, ir::SizeOf(in.type));
                return StoreResult(9, in.dst);
            case Op::Store:
                Load(9, in.args[0]);
                Load(10, in.args[1]);
                return EmitMem(os_, StoreOp(in.type), Reg(9, in.type), "x10", 0, ir::SizeOf(in.type));
            case Op::Label: os_ << LocalLabel(in.label) << ":\n"; return;
            case Op::Jump: os_ << "\tb " << LocalLabel(in.label) << "\n"; return;
            case Op::Branch: return WriteBranch(in);
            case Op::Ret:
                if (!in.args.empty()) LoadReturnValue(in.args[0]);
                os_ << "\tb " << ReturnLabel() << "\n";
                return;
            default: NotYet(ir::OpName(in.op));
        }
    }

    const ir::Function& fn_;
    std::ostream& os_;
    CallLayout entry_;
    Frame frame_;
};

// ---- data (21.2a): the only place that writes data directives ----
// A1 has no data entries.

void WriteData(const ir::Module& module, std::ostream&) {
    if (!module.data.empty()) NotYet("data entries");
}

}  // namespace

void EmitAArch64(const ir::Module& module, std::ostream& os) {
    for (const ir::Function& fn : module.functions) FunctionWriter(fn, os).Write();
    WriteData(module, os);
}
