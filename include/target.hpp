#pragma once
#include <string>

class TargetAbi;  // call_abi.hpp

// Which calling convention a target uses; SetTarget maps it to an object.
enum class AbiKind { None, RiscV32 };

// Facts about the machine being compiled for (D21, 21.3). Sizes double as
// alignments unless a field says otherwise.
struct TargetInfo {
    int char_size, short_size, int_size, long_size, pointer_size;
    int float_size, double_size, double_align;
    bool char_is_signed;  // D8: false on riscv32
    int stack_align;      // 16 on ilp32d
    AbiKind abi_kind = AbiKind::None;
    const TargetAbi* abi = nullptr;  // set by SetTarget from abi_kind

    // The calling convention. Aborts if SetTarget chose none (no default).
    const TargetAbi& Abi() const;
};

// The RISC-V 32-bit target (ilp32d).
TargetInfo Riscv32Target();

// Looks a target up by its --target name; returns false if there is none.
bool FindTarget(const std::string& name, TargetInfo& out);

// Called once, in main, before parsing. A second call aborts.
void SetTarget(const TargetInfo& info);

// The target of this compile. Aborts if SetTarget has not been called: there
// is no default target, so a missed initialisation cannot go unnoticed.
const TargetInfo& Target();
