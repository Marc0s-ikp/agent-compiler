#include "ir_gen.hpp"

#include <stdexcept>

#include "const_value.hpp"
#include "target.hpp"

using ir::Instruction;
using ir::IrType;
using ir::Op;
using ir::Operand;

// ---- C types at the edge ----

IrType IRGen::ToIr(const Type& type) { return ir::IrTypeOf(type); }

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
    fn_.c_return = return_type;
    if (fn_.returns_value) fn_.return_type = ToIr(*return_type);
    object_count_ = 0;
}

void IRGen::EndFunction() {
    if (fn_.returns_value) {  // falling off the end: return a zero of the right type
        IrType type = fn_.return_type;
        if (ir::IsFloatType(type)) {
            Return(FloatConstant(0.0, type));
        } else if (fn_.c_return->IsRecord()) {  // no value was returned: any object will do
            Return(AddressOf(AddFrameObject("noret", *fn_.c_return)));
        } else if (type == IrType::Ptr) {
            Return(IntToPtr(Operand::Int(0, ir::PointerIntType())));
        } else {
            Return(Operand::Int(0, type));
        }
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

Operand IRGen::AddParam(const std::string& name, const TypePtr& type) {
    ToIr(*type);  // rejects a type the IR cannot hold yet
    Operand home = AddFrameObject(name, *type);
    fn_.params.push_back({home.name, type});
    return home;
}

Operand IRGen::AddFrameObject(const std::string& name, const Type& type) {
    std::string unique = name + "." + std::to_string(object_count_++);
    fn_.objects.push_back({unique, type.Size(), type.Align()});
    return Operand::Object(unique);
}

// ---- instructions ----

void IRGen::Append(const Instruction& in) { fn_.body.push_back(in); }

Operand IRGen::Emit(Instruction in, IrType result) {
    fn_.temps.push_back(result);
    in.dst = Operand::Temp(static_cast<int>(fn_.temps.size()) - 1, result);
    Append(in);
    return in.dst;
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

Operand IRGen::PtrAdd(Operand pointer, Operand offset) {
    Instruction in;
    in.op = Op::PtrAdd;
    in.type = IrType::Ptr;
    in.args = {pointer, offset};
    return Emit(in, IrType::Ptr);
}

Operand IRGen::PtrToInt(Operand pointer) {
    Instruction in;
    in.op = Op::PtrToInt;
    in.type = ir::PointerIntType();
    in.args = {pointer};
    return Emit(in, in.type);
}

Operand IRGen::IntToPtr(Operand integer) {
    Instruction in;
    in.op = Op::IntToPtr;
    in.type = IrType::Ptr;
    in.args = {integer};
    return Emit(in, IrType::Ptr);
}

Operand IRGen::Convert(Op op, IrType result, Operand value) {
    Instruction in;
    in.op = op;
    in.type = result;
    in.args = {value};
    return Emit(in, result);
}

Operand IRGen::FloatConstant(double value, IrType type) {
    int size = ir::SizeOf(type);
    std::string label = ctx_.AddFloat(ast::FloatBits(value, size), size);
    return Load(type, AddressOf(Operand::Symbol(label)));
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

void IRGen::MemZero(Operand address, long size, long align) {
    Instruction in;
    in.op = Op::MemZero;
    in.args = {address};
    in.size = size;
    in.align = align;
    Append(in);
}

void IRGen::MemCpy(Operand dst, Operand src, long size, long align) {
    Instruction in;
    in.op = Op::MemCpy;
    in.args = {dst, src};
    in.size = size;
    in.align = align;
    Append(in);
}

Operand IRGen::CopyRecord(Operand source, const Type& type) {
    Operand copy = AddFrameObject("copy", type);
    Operand address = AddressOf(copy);
    MemCpy(address, source, type.Size(), type.Align());
    return address;
}

// The result is None for a void function. Every callee is recorded as an
// external; EndModule removes the ones this module defines.
Operand IRGen::Call(const std::string& callee, const std::vector<Operand>& args,
                    const std::vector<TypePtr>& arg_types, const TypePtr& ret_type) {
    Instruction in;
    in.op = Op::Call;
    in.callee = callee;
    in.args = args;
    in.arg_types = arg_types;
    in.ret_type = ret_type;
    module_.externals.insert(callee);
    if (ret_type->IsVoid()) {
        Append(in);
        return Operand();
    }
    if (ret_type->IsRecord()) {  // received in a frame object of the call site
        in.result_object = AddFrameObject("ret", *ret_type);
        Append(in);
        return AddressOf(in.result_object);
    }
    in.type = ToIr(*ret_type);
    return Emit(in, in.type);
}

void IRGen::EndModule() {
    for (const ir::Function& fn : module_.functions) module_.externals.erase(fn.name);
    ctx_.ExportGlobals(module_);
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
