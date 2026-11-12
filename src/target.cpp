#include "target.hpp"

#include "call_abi_riscv.hpp"

#include <cstdlib>
#include <iostream>

namespace {
bool g_is_set = false;
TargetInfo g_target;
}  // namespace

TargetInfo Riscv32Target() {
    TargetInfo t;
    t.char_size = 1;
    t.short_size = 2;
    t.int_size = 4;
    t.long_size = 4;
    t.pointer_size = 4;
    t.float_size = 4;
    t.double_size = 8;
    t.double_align = 8;
    t.char_is_signed = false;
    t.stack_align = 16;
    t.abi_kind = AbiKind::RiscV32;
    return t;
}

bool FindTarget(const std::string& name, TargetInfo& out) {
    if (name != "riscv32") return false;
    out = Riscv32Target();
    return true;
}

const TargetAbi& TargetInfo::Abi() const {
    if (!abi) {
        std::cerr << "internal error: Target().Abi() used but no ABI was set\n";
        std::abort();
    }
    return *abi;
}

namespace {
// One static object per ABI class; a target only points at it.
const TargetAbi* AbiFor(AbiKind kind) {
    static const RiscV32Abi riscv32;
    switch (kind) {
        case AbiKind::RiscV32: return &riscv32;
        case AbiKind::None: return nullptr;
    }
    return nullptr;
}
}  // namespace

void SetTarget(const TargetInfo& info) {
    if (g_is_set) {
        std::cerr << "internal error: SetTarget called twice\n";
        std::abort();
    }
    g_target = info;
    g_target.abi = AbiFor(info.abi_kind);
    g_is_set = true;
}

const TargetInfo& Target() {
    if (!g_is_set) {
        std::cerr << "internal error: Target() used before SetTarget()\n";
        std::abort();
    }
    return g_target;
}
