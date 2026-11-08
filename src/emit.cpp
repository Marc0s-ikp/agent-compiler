#include "emit.hpp"

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
    if (!to.IsInteger() || !from.IsInteger()) return;
    if (to.Size() >= 4) return;
    if (from.Size() == to.Size() && from.IsSigned() == to.IsSigned()) return;
    int shift = 32 - 8 * to.Size();
    os << "\tslli a0, a0, " << shift << "\n";
    os << "\t" << (to.IsSigned() ? "srai" : "srli") << " a0, a0, " << shift
       << "\n";
}
