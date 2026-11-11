#include "ir.hpp"

#include <stdexcept>

#include "target.hpp"

namespace ir {

int SizeOf(IrType type) {
    switch (type) {
        case IrType::I8: return 1;
        case IrType::I16: return 2;
        case IrType::I32: return 4;
        case IrType::I64: return 8;
        case IrType::F32: return 4;
        case IrType::F64: return 8;
        case IrType::Ptr: return Target().pointer_size;
    }
    return 0;
}

bool IsIntType(IrType t) { return t <= IrType::I64; }
bool IsFloatType(IrType t) { return t == IrType::F32 || t == IrType::F64; }

const char* TypeName(IrType type) {
    static const char* names[] = {"i8", "i16", "i32", "i64", "f32", "f64", "ptr"};
    return names[static_cast<int>(type)];
}

Operand Operand::Temp(int id, IrType type) {
    Operand o;
    o.kind = Kind::Temp;
    o.temp = id;
    o.type = type;
    return o;
}

Operand Operand::Int(long value, IrType type) {
    Operand o;
    o.kind = Kind::Int;
    o.int_value = value;
    o.type = type;
    return o;
}

Operand Operand::Float(double value, IrType type) {
    Operand o;
    o.kind = Kind::Float;
    o.float_value = value;
    o.type = type;
    return o;
}

Operand Operand::Object(const std::string& name) {
    Operand o;
    o.kind = Kind::Object;
    o.type = IrType::Ptr;
    o.name = name;
    return o;
}

Operand Operand::Symbol(const std::string& name) {
    Operand o = Object(name);
    o.kind = Kind::Symbol;
    return o;
}

const char* OpName(Op op) {
    static const char* names[] = {
        "copy", "const", "addr",
        "add", "sub", "mul", "div", "divu", "rem", "remu", "and", "or", "xor",
        "shl", "shr", "sar", "neg", "not",
        "eq", "ne", "lt", "ltu", "le", "leu", "feq", "flt", "fle",
        "sext", "zext", "trunc", "itof", "utof", "ftoi", "ftou", "fext", "ftrunc",
        "load", "store", "memcpy", "memzero",
        "label", "jump", "branch", "ret"};
    return names[static_cast<int>(op)];
}

bool IsBinaryArith(Op op) { return op >= Op::Add && op <= Op::Sar; }
bool IsCompare(Op op) { return op >= Op::Eq && op <= Op::FLe; }
bool IsConversion(Op op) { return op >= Op::SExt && op <= Op::FTrunc; }

}  // namespace ir
