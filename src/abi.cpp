#include "abi.hpp"

namespace {
const int kIntArgRegs = 8;

int RoundUp(int value, int multiple) {
    return (value + multiple - 1) / multiple * multiple;
}
}  // namespace

// Integers use a0-a7 in order, then 4-byte-aligned stack slots.
std::vector<ArgLoc> ClassifyArgs(const std::vector<TypePtr>& param_types) {
    std::vector<ArgLoc> locs;
    int next_reg = 0;
    int next_stack = 0;
    for (const TypePtr& type : param_types) {
        if (next_reg < kIntArgRegs) {
            locs.push_back({ArgLoc::Kind::IntReg, next_reg++, 0});
            continue;
        }
        next_stack = RoundUp(next_stack, type->Align());
        locs.push_back({ArgLoc::Kind::Stack, 0, next_stack});
        next_stack += RoundUp(type->Size(), 4);
    }
    return locs;
}
