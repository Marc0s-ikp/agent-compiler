#pragma once
#include "call_abi.hpp"

// The AArch64 calling convention, AAPCS64 with LP64 (D27, 27.2), implemented in
// src/call_abi_aarch64.cpp, which holds every one of its rules (D25).
class AArch64Abi : public TargetAbi {
public:
    CallLayout Classify(const std::vector<TypePtr>& params, const TypePtr& ret) const override;
};
