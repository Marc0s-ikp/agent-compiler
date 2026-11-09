#include "abi.hpp"

namespace {
const int kIntArgRegs = 8;
const int kFloatArgRegs = 8;

int RoundUp(int value, int multiple) {
    return (value + multiple - 1) / multiple * multiple;
}

// The running state of the classifier: the next free register of each file
// and the next free stack byte.
struct Cursor {
    int int_reg = 0;
    int float_reg = 0;
    int stack = 0;

    ArgLoc TakeStack(const Type& type) {
        stack = RoundUp(stack, type.Align());
        ArgLoc loc{ArgLoc::Kind::Stack, 0, stack};
        stack += RoundUp(type.Size(), 4);
        return loc;
    }

    // A double with no fa register left: two a registers if there are two,
    // the last a register plus a stack word if one, else the stack.
    ArgLoc TakeDoubleAsInts(const Type& type) {
        int left = kIntArgRegs - int_reg;
        if (left >= 2) {
            ArgLoc loc{ArgLoc::Kind::IntPair, int_reg, 0};
            int_reg += 2;
            return loc;
        }
        if (left == 1) {
            ArgLoc loc{ArgLoc::Kind::Split, int_reg++, stack};
            stack += 4;
            return loc;
        }
        return TakeStack(type);
    }

    ArgLoc Take(const Type& type) {
        if (type.IsFloating() && float_reg < kFloatArgRegs) {
            return {ArgLoc::Kind::FloatReg, float_reg++, 0};
        }
        if (type.IsFloating() && type.Size() == 8) return TakeDoubleAsInts(type);
        if (int_reg < kIntArgRegs) return {ArgLoc::Kind::IntReg, int_reg++, 0};
        return TakeStack(type);
    }
};
}  // namespace

// Floats and doubles use fa0-fa7 first. Everything else, and a float or
// double that finds those full, uses a0-a7, then 4-byte-aligned stack slots
// (8-aligned for a double).
std::vector<ArgLoc> ClassifyArgs(const std::vector<TypePtr>& param_types) {
    std::vector<ArgLoc> locs;
    Cursor cursor;
    for (const TypePtr& type : param_types) locs.push_back(cursor.Take(*type));
    return locs;
}

int StackExtent(const ArgLoc& loc, const Type& type) {
    switch (loc.kind) {
        case ArgLoc::Kind::Stack: return loc.offset + RoundUp(type.Size(), 4);
        case ArgLoc::Kind::Split: return loc.offset + 4;
        default: return 0;
    }
}
