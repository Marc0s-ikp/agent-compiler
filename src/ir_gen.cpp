#include "ir_gen.hpp"

#include <stdexcept>

#include "const_value.hpp"
#include "target.hpp"

using ir::Instruction;
using ir::IrType;
using ir::Op;
using ir::Operand;

// ---- C types at the edge ----

IrType IRGen::ToIr(const Type& type) {
    if (type.IsPointer()) return IrType::Ptr;
    if (type.IsInteger()) {
        switch (type.Size()) {
            case 1: return IrType::I8;
            case 2: return IrType::I16;
            case 4: return IrType::I32;
            case 8: return IrType::I64;
        }
    }
    throw std::runtime_error("not in IR yet: this type");
}

// A constant converts at compile time; anything else gets sext, zext or
// trunc. Equal sizes need nothing, because the IR type carries no sign.
Operand IRGen::ConvertInt(Operand value, const Type& from, const Type& to) {
    IrType result = ToIr(to);
    if (value.kind == Operand::Kind::Int) {
        long canonical = ast::WrapToType(value.int_value, from);
        return Operand::Int(ast::WrapToType(canonical, to), result);
    }
    if (from.Size() == to.Size()) return value;
    Instruction in;
    in.op = to.Size() < from.Size() ? Op::Trunc : (from.IsSigned() ? Op::SExt : Op::ZExt);
    in.type = result;
    in.args = {value};
    return Emit(in, result);
}

// ---- functions ----

void IRGen::BeginFunction(const std::string& name, bool is_global, const TypePtr& return_type) {
    fn_ = ir::Function();
    fn_.name = name;
    fn_.is_global = is_global;
    fn_.returns_value = !return_type->IsVoid();
    if (fn_.returns_value) fn_.return_type = ToIr(*return_type);
    object_count_ = 0;
}

void IRGen::EndFunction() {
    if (fn_.returns_value) {
        Return(Operand::Int(0, fn_.return_type));
    } else {
        Return();
    }
    module_.functions.push_back(std::move(fn_));
    fn_ = ir::Function();
}

void IRGen::AppendPartialFunction() {
    if (fn_.name.empty()) return;
    module_.functions.push_back(std::move(fn_));
    fn_ = ir::Function();
}

Operand IRGen::AddFrameObject(const std::string& name, const Type& type) {
    std::string unique = name + "." + std::to_string(object_count_++);
    fn_.objects.push_back({unique, type.Size(), type.Align()});
    return Operand::Object(unique);
}

// ---- instructions ----

Operand IRGen::NewTemp(IrType type) {
    fn_.temps.push_back(type);
    return Operand::Temp(static_cast<int>(fn_.temps.size()) - 1, type);
}

void IRGen::Append(const Instruction& in) { fn_.body.push_back(in); }

Operand IRGen::Emit(Instruction in, IrType result) {
    in.dst = NewTemp(result);
    Append(in);
    return in.dst;
}

void IRGen::Move(Operand dst, Operand src) {
    Instruction in;
    in.op = src.IsConst() ? Op::Const : Op::Copy;
    in.type = dst.type;
    in.dst = dst;
    in.args = {src};
    Append(in);
}

Operand IRGen::Binary(Op op, IrType type, Operand a, Operand b) {
    Instruction in;
    in.op = op;
    in.type = type;
    in.args = {a, b};
    return Emit(in, type);
}

Operand IRGen::Compare(Op op, IrType type, Operand a, Operand b) {
    Instruction in;
    in.op = op;
    in.type = type;
    in.args = {a, b};
    return Emit(in, IrType::I32);
}

Operand IRGen::Unary(Op op, IrType type, Operand a) {
    Instruction in;
    in.op = op;
    in.type = type;
    in.args = {a};
    return Emit(in, type);
}

Operand IRGen::AddressOf(Operand object_or_symbol, long offset) {
    Instruction in;
    in.op = Op::Addr;
    in.type = IrType::Ptr;
    in.args = {object_or_symbol};
    in.offset = offset;
    return Emit(in, IrType::Ptr);
}

Operand IRGen::Load(IrType type, Operand address) {
    Instruction in;
    in.op = Op::Load;
    in.type = type;
    in.args = {address};
    return Emit(in, type);
}

void IRGen::Store(IrType type, Operand value, Operand address) {
    Instruction in;
    in.op = Op::Store;
    in.type = type;
    in.args = {value, address};
    Append(in);
}

void IRGen::Return() {
    Instruction in;
    in.op = Op::Ret;
    Append(in);
}

void IRGen::Return(Operand value) {
    Instruction in;
    in.op = Op::Ret;
    in.type = value.type;
    in.args = {value};
    Append(in);
}

void IRGen::Label(const std::string& label) {
    Instruction in;
    in.op = Op::Label;
    in.label = label;
    Append(in);
}

void IRGen::Jump(const std::string& label) {
    Instruction in;
    in.op = Op::Jump;
    in.label = label;
    Append(in);
}

void IRGen::Branch(Operand condition, const std::string& if_nonzero,
                   const std::string& if_zero) {
    Instruction in;
    in.op = Op::Branch;
    in.args = {condition};
    in.label = if_nonzero;
    in.label2 = if_zero;
    Append(in);
}
