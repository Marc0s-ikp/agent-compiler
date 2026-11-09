#include "emit.hpp"

#include <stdexcept>

namespace {
bool FitsImm12(int v) { return v >= -2048 && v <= 2047; }
}  // namespace

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

std::string AccReg(const Type& type) { return type.IsFloating() ? "fa0" : "a0"; }

std::string ScratchReg(const Type& type) {
    return type.IsFloating() ? "ft0" : "t0";
}

namespace {
// The instruction suffix of a floating type: 's' for float, 'd' for double.
char FloatSuffix(const Type& type) { return type.Size() == 4 ? 's' : 'd'; }
}  // namespace

void EmitMoveToScratch(std::ostream& os, const Type& type) {
    if (type.IsFloating()) {
        os << "\tfmv." << FloatSuffix(type) << " ft0, fa0\n";
    } else {
        os << "\tmv t0, a0\n";
    }
}

void EmitLoad(std::ostream& os, const Type& type, const std::string& reg,
              const std::string& base, int off) {
    // An array has no scalar value: its value is its address (D10).
    if (type.IsArray()) {
        EmitAddrOffset(os, reg, base, off);
        return;
    }
    const char* op = "lw";
    if (type.IsFloating()) op = type.Size() == 4 ? "flw" : "fld";
    if (type.Size() == 1) op = type.IsSigned() ? "lb" : "lbu";
    if (type.Size() == 2) op = type.IsSigned() ? "lh" : "lhu";
    EmitMem(os, op, reg, base, off);
}

void EmitStore(std::ostream& os, const Type& type, const std::string& reg,
               const std::string& base, int off) {
    const char* op = "sw";
    if (type.IsFloating()) op = type.Size() == 4 ? "fsw" : "fsd";
    if (type.Size() == 1) op = "sb";
    if (type.Size() == 2) op = "sh";
    EmitMem(os, op, reg, base, off);
}

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

namespace {

// Truncation to a narrower integer type: shift left then right to drop high
// bits and re-extend. Registers always hold a canonical 32-bit value.
void EmitNarrow(std::ostream& os, const Type& from, const Type& to) {
    if (to.Size() >= 4) return;
    if (from.Size() == to.Size() && from.IsSigned() == to.IsSigned()) return;
    int shift = 32 - 8 * to.Size();
    os << "\tslli a0, a0, " << shift << "\n";
    os << "\t" << (to.IsSigned() ? "srai" : "srli") << " a0, a0, " << shift
       << "\n";
}

// int -> float: the .wu form for unsigned sources (narrow ones are already
// zero-extended, so it is right for them too).
void EmitIntToFloat(std::ostream& os, const Type& from, const Type& to) {
    os << "\tfcvt." << FloatSuffix(to) << (from.IsSigned() ? ".w" : ".wu")
       << " fa0, a0\n";
}

// float -> int, rounding toward zero as C requires. Only a 32-bit unsigned
// target uses .wu; a narrower target converts through int, then narrows.
void EmitFloatToInt(std::ostream& os, const Type& from, const Type& to) {
    bool wide_unsigned = !to.IsSigned() && to.Size() == 4;
    os << "\tfcvt." << (wide_unsigned ? "wu." : "w.") << FloatSuffix(from)
       << " a0, fa0, rtz\n";
    if (to.Size() < 4) EmitNarrow(os, *Type::Int(), to);
}

}  // namespace

void EmitConvert(std::ostream& os, const Type& from, const Type& to) {
    if (from.IsVoid() && to.IsVoid()) return;
    if (!from.IsScalar() || !to.IsScalar()) {
        throw std::runtime_error("invalid type conversion");
    }
    if (from.IsFloating() && to.IsFloating()) {
        if (from.Size() == to.Size()) return;
        os << "\tfcvt." << FloatSuffix(to) << "." << FloatSuffix(from)
           << " fa0, fa0\n";
    } else if (to.IsFloating()) {
        if (!from.IsInteger()) throw std::runtime_error("invalid type conversion");
        EmitIntToFloat(os, from, to);
    } else if (from.IsFloating()) {
        if (!to.IsInteger()) throw std::runtime_error("invalid type conversion");
        EmitFloatToInt(os, from, to);
    } else if (to.IsInteger() && from.IsInteger()) {  // 32-bit pointers
        EmitNarrow(os, from, to);
    }
}

void EmitFloatTruth(std::ostream& os, const Type& type) {
    if (type.Size() == 4) {
        os << "\tfmv.w.x ft0, zero\n";
    } else {
        os << "\tfcvt.d.w ft0, zero\n";  // RV32 has no fmv.d.x
    }
    os << "\tfeq." << FloatSuffix(type) << " a0, fa0, ft0\n\txori a0, a0, 1\n";
}

void EmitZeroMem(std::ostream& os, const std::string& base, int off,
                 int bytes) {
    const int kUnrolledMax = 64;
    if (bytes <= kUnrolledMax) {
        for (int i = 0; i < bytes;) {
            bool word = (off + i) % 4 == 0 && bytes - i >= 4;
            EmitMem(os, word ? "sw" : "sb", "zero", base, off + i);
            i += word ? 4 : 1;
        }
        return;
    }
    bool word = off % 4 == 0 && bytes % 4 == 0;
    EmitAddrOffset(os, "t0", base, off);
    os << "\tli t1, " << bytes << "\n\tadd t1, t0, t1\n";
    os << "1:\n\t" << (word ? "sw" : "sb") << " zero, 0(t0)\n";
    os << "\taddi t0, t0, " << (word ? 4 : 1) << "\n\tbne t0, t1, 1b\n";
}

void EmitJump(std::ostream& os, const std::string& label) {
    os << "\tj " << label << "\n";
}

// '1f' is the assembler's local label 1 just ahead, so no unique name is needed.
void EmitBranchIfZero(std::ostream& os, const std::string& label) {
    os << "\tbnez a0, 1f\n";
    EmitJump(os, label);
    os << "1:\n";
}

void EmitBranchIfNonZero(std::ostream& os, const std::string& label) {
    os << "\tbeqz a0, 1f\n";
    EmitJump(os, label);
    os << "1:\n";
}
