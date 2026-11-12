#pragma once
#include "call_abi.hpp"

// The RISC-V ilp32d calling convention (D5, D18(d), D19(d)), implemented in
// src/call_abi_riscv.cpp, which holds every one of its rules (D25).
class RiscV32Abi : public TargetAbi {
public:
    CallLayout Classify(const std::vector<TypePtr>& params, const TypePtr& ret) const override;
};
