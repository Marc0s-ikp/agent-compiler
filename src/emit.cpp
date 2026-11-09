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

void EmitLoad(std::ostream& os, const Type& type, const std::string& reg,
              const std::string& base, int off) {
    // An array has no scalar value: its value is its address (D10).
    if (type.IsArray()) {
        EmitAddrOffset(os, reg, base, off);
        return;
    }
    const char* op = "lw";
    if (type.Size() == 1) op = type.IsSigned() ? "lb" : "lbu";
    if (type.Size() == 2) op = type.IsSigned() ? "lh" : "lhu";
    EmitMem(os, op, reg, base, off);
}

void EmitStore(std::ostream& os, const Type& type, const std::string& reg,
               const std::string& base, int off) {
    const char* op = "sw";
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

// Registers always hold a canonical 32-bit value, so only narrowing needs
// code: shift left then right to drop high bits and re-extend.
void EmitConvert(std::ostream& os, const Type& from, const Type& to) {
    if (from.IsVoid() && to.IsVoid()) return;
    if (!from.IsScalar() || !to.IsScalar()) {
        throw std::runtime_error("invalid type conversion");
    }
    if (!to.IsInteger() || !from.IsInteger()) return;  // 32-bit pointers
    if (to.Size() >= 4) return;
    if (from.Size() == to.Size() && from.IsSigned() == to.IsSigned()) return;
    int shift = 32 - 8 * to.Size();
    os << "\tslli a0, a0, " << shift << "\n";
    os << "\t" << (to.IsSigned() ? "srai" : "srli") << " a0, a0, " << shift
       << "\n";
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
