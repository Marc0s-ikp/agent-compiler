// The --ir lowering of the AST (D21, 21.1): every node's EmitIR / EmitValue.
// One file while both paths exist; step 7 deletes EmitRISC and this becomes
// the only code generation. Everything here is target-independent: it uses
// IRGen helpers and never names a register, an instruction or an offset.
#include <stdexcept>

#include "ast_assign.hpp"
#include "ast_cast.hpp"
#include "ast_function.hpp"
#include "ast_identifier.hpp"
#include "ast_literal.hpp"
#include "ast_operators.hpp"
#include "ast_stmt.hpp"
#include "ir_gen.hpp"
#include "target.hpp"

namespace ast {

namespace {

using ir::IrType;
using ir::Op;
using ir::Operand;

// Step 1 handles integer types only; pointers, floats and records come later.
const Type& RequireIrInteger(const Type& type) {
    if (!type.IsInteger()) throw std::runtime_error("not in IR yet: non-integer operand");
    return type;
}

Op ArithOp(BinOp op, bool is_signed) {
    switch (op) {
        case BinOp::Add: return Op::Add;
        case BinOp::Sub: return Op::Sub;
        case BinOp::Mul: return Op::Mul;
        case BinOp::Div: return is_signed ? Op::Div : Op::DivU;
        case BinOp::Mod: return is_signed ? Op::Rem : Op::RemU;
        case BinOp::Shl: return Op::Shl;
        case BinOp::Shr: return is_signed ? Op::Sar : Op::Shr;
        case BinOp::And: return Op::And;
        case BinOp::Or: return Op::Or;
        case BinOp::Xor: return Op::Xor;
    }
    return Op::Add;
}

// a <op> b, both already converted to 'common'.
Operand Arith(IRGen& gen, BinOp op, const Type& common, Operand a, Operand b) {
    return gen.Binary(ArithOp(op, common.IsSigned()), IRGen::ToIr(common), a, b);
}

// The usual arithmetic type of a compare's operands (integers only for now).
TypePtr CompareCommonType(const Context& ctx, const Expr& lhs, const Expr& rhs) {
    TypePtr l = lhs.ValueType(ctx);
    TypePtr r = rhs.ValueType(ctx);
    RequireIrInteger(*l);
    RequireIrInteger(*r);
    return UsualArithmetic(l, r);
}

// Evaluates 'e' and converts it to 'to' (an integer type).
Operand ValueAs(IRGen& gen, const Expr& e, const Type& to) {
    TypePtr from = e.ValueType(gen.Ctx());
    RequireIrInteger(*from);
    return gen.ConvertInt(e.EmitValue(gen), *from, to);
}

}  // namespace

// ---- literals and names ----

Operand IntLiteral::EmitValue(IRGen& gen) const {
    CheckRange();
    return Operand::Int(*EvalConst(gen.Ctx()), IRGen::ToIr(*GetType(gen.Ctx())));
}

Operand CharLiteral::EmitValue(IRGen&) const {
    return Operand::Int(value_, IRGen::ToIr(*Type::Int()));
}

namespace {
const Symbol& IrVariable(const Context& ctx, const std::string& name) {
    const Symbol& sym = ctx.Lookup(name);
    if (sym.kind == SymbolKind::Global) throw std::runtime_error("not in IR yet: global variable");
    return sym;
}
}  // namespace

Operand Identifier::EmitIRAddress(IRGen& gen) const {
    const Symbol& sym = IrVariable(gen.Ctx(), name_);
    if (sym.kind != SymbolKind::Local) throw std::runtime_error("'" + name_ + "' is not a variable");
    return gen.AddressOf(Operand::Object(sym.object));
}

Operand Identifier::EmitValue(IRGen& gen) const {
    const Symbol& sym = IrVariable(gen.Ctx(), name_);
    if (sym.kind == SymbolKind::EnumConst) return Operand::Int(sym.value, IRGen::ToIr(*sym.type));
    RequireIrInteger(*sym.type);
    return gen.Load(IRGen::ToIr(*sym.type), EmitIRAddress(gen));
}

// ---- operators ----

Operand BinaryArith::EmitValue(IRGen& gen) const {
    Context& ctx = gen.Ctx();
    RequireIrInteger(*lhs_->ValueType(ctx));
    RequireIrInteger(*rhs_->ValueType(ctx));
    TypePtr result = GetType(ctx);
    bool shift = op_ == BinOp::Shl || op_ == BinOp::Shr;
    TypePtr rhs_type = shift ? Promote(rhs_->ValueType(ctx)) : result;
    Operand a = ValueAs(gen, *lhs_, *result);
    Operand b = ValueAs(gen, *rhs_, *rhs_type);
    return Arith(gen, op_, *result, a, b);
}

Operand BinaryCompare::EmitValue(IRGen& gen) const {
    TypePtr common = CompareCommonType(gen.Ctx(), *lhs_, *rhs_);
    Operand a = ValueAs(gen, *lhs_, *common);
    Operand b = ValueAs(gen, *rhs_, *common);
    IrType type = IRGen::ToIr(*common);
    bool s = common->IsSigned();
    Op less = s ? Op::Lt : Op::LtU, less_eq = s ? Op::Le : Op::LeU;
    switch (op_) {
        case CmpOp::Lt: return gen.Compare(less, type, a, b);
        case CmpOp::Gt: return gen.Compare(less, type, b, a);
        case CmpOp::Le: return gen.Compare(less_eq, type, a, b);
        case CmpOp::Ge: return gen.Compare(less_eq, type, b, a);
        case CmpOp::Eq: return gen.Compare(Op::Eq, type, a, b);
        case CmpOp::Ne: return gen.Compare(Op::Ne, type, a, b);
    }
    return a;
}

Operand UnaryOp::EmitValue(IRGen& gen) const {
    Context& ctx = gen.Ctx();
    TypePtr operand_type = operand_->ValueType(ctx);
    RequireIrInteger(*operand_type);
    if (op_ == UnOp::LogNot) {  // !x is x == 0
        IrType type = IRGen::ToIr(*operand_type);
        return gen.Compare(Op::Eq, type, operand_->EmitValue(gen), Operand::Int(0, type));
    }
    TypePtr result = GetType(ctx);
    Operand v = ValueAs(gen, *operand_, *result);
    IrType type = IRGen::ToIr(*result);
    switch (op_) {
        case UnOp::Neg: return gen.Unary(Op::Neg, type, v);
        case UnOp::BitNot: return gen.Unary(Op::Not, type, v);
        default: return v;
    }
}

Operand Comma::EmitValue(IRGen& gen) const {
    lhs_->EmitValue(gen);
    return rhs_->EmitValue(gen);
}

Operand Cast::EmitValue(IRGen& gen) const {
    TypePtr to = GetType(gen.Ctx());
    RequireIrInteger(*to);
    return ValueAs(gen, *operand_, *to);
}

// ---- assignment ----

// The address is evaluated first, then the right side, as on the old path.
Operand Assign::EmitValue(IRGen& gen) const {
    TypePtr type = GetType(gen.Ctx());
    RequireIrInteger(*type);
    Operand address = lhs_->EmitIRAddress(gen);
    Operand value = ValueAs(gen, *rhs_, *type);
    gen.Store(IRGen::ToIr(*type), value, address);
    return value;
}

// lhs op= rhs: the old value is converted to the operation's type, combined
// with the right side, and converted back for the store.
Operand CompoundAssign::EmitValue(IRGen& gen) const {
    Context& ctx = gen.Ctx();
    TypePtr type = GetType(ctx);
    TypePtr rhs_type = rhs_->ValueType(ctx);
    RequireIrInteger(*type);
    RequireIrInteger(*rhs_type);
    bool shift = op_ == BinOp::Shl || op_ == BinOp::Shr;
    TypePtr common = shift ? Promote(type) : UsualArithmetic(type, rhs_type);
    TypePtr rhs_to = shift ? Promote(rhs_type) : common;
    IrType ir_type = IRGen::ToIr(*type);

    Operand address = lhs_->EmitIRAddress(gen);
    Operand old = gen.ConvertInt(gen.Load(ir_type, address), *type, *common);
    Operand rhs = ValueAs(gen, *rhs_, *rhs_to);
    Operand result = gen.ConvertInt(Arith(gen, op_, *common, old, rhs), *common, *type);
    gen.Store(ir_type, result, address);
    return result;
}

Operand IncDec::EmitValue(IRGen& gen) const {
    TypePtr type = GetType(gen.Ctx());
    RequireIrInteger(*type);
    TypePtr wide = Promote(type);
    IrType ir_type = IRGen::ToIr(*type);

    Operand address = operand_->EmitIRAddress(gen);
    Operand old = gen.Load(ir_type, address);
    Operand step = Operand::Int(increment_ ? 1 : -1, IRGen::ToIr(*wide));
    Operand sum = Arith(gen, BinOp::Add, *wide, gen.ConvertInt(old, *type, *wide), step);
    Operand updated = gen.ConvertInt(sum, *wide, *type);
    gen.Store(ir_type, updated, address);
    return prefix_ ? updated : old;
}

// ---- statements ----

void CompoundStmt::EmitIR(IRGen& gen) const {
    gen.Ctx().PushScope();
    for (const StmtPtr& stmt : body_) stmt->EmitIR(gen);
    gen.Ctx().PopScope();
}

void ExprStmt::EmitIR(IRGen& gen) const {
    if (expr_) expr_->EmitValue(gen);
}

void DeclarationStmt::EmitIR(IRGen& gen) const {
    Context& ctx = gen.Ctx();
    TypePtr base = spec_.Resolve(ctx);
    for (const InitDeclarator& d : declarators_) {
        auto [name, type] = d.declarator.Apply(base, ctx);
        if (storage_ == Storage::Typedef) {
            ctx.Declare(name, Symbol::Typedef(type));
            continue;
        }
        if (storage_ == Storage::Static) throw std::runtime_error("not in IR yet: static local");
        if (type->IsVoid() || type->IsFunction()) {
            throw std::runtime_error("invalid type for variable '" + name + "'");
        }
        RequireIrInteger(*type);
        if (d.init && d.init->IsList()) throw std::runtime_error("not in IR yet: initialiser list");
        Operand object = gen.AddFrameObject(name, *type);
        ctx.Declare(name, Symbol::IrLocal(type, object.name));
        if (d.init) {
            Operand value = ValueAs(gen, d.init->Value(), *type);
            gen.Store(IRGen::ToIr(*type), value, gen.AddressOf(object));
        }
    }
}

void ReturnStmt::EmitIR(IRGen& gen) const {
    if (!value_) return gen.Return();
    const TypePtr& to = gen.Ctx().ReturnType();
    RequireIrInteger(*to);
    gen.Return(ValueAs(gen, *value_, *to));
}

// ---- functions and the file ----

// Prototypes and typedefs are declarations only; variables need data entries
// (step 4).
void GlobalDeclaration::EmitIR(IRGen& gen) const {
    Context& ctx = gen.Ctx();
    TypePtr base = spec_.Resolve(ctx);
    for (const InitDeclarator& d : declarators_) {
        auto [name, type] = d.declarator.Apply(base, ctx);
        if (storage_ == Storage::Typedef) {
            ctx.Declare(name, Symbol::Typedef(type));
        } else if (type->IsFunction() && !d.init) {
            ctx.DeclareFunction(name, type);
        } else {
            throw std::runtime_error("not in IR yet: global variable");
        }
    }
}

// Parameters wait for the calling convention (step 3).
void FunctionDefinition::EmitIR(IRGen& gen) const {
    Context& ctx = gen.Ctx();
    auto [name, type] = declarator_.Apply(spec_.Resolve(ctx), ctx);
    if (!type->IsFunction()) throw std::runtime_error("not a function");
    const std::vector<Param>* declared = declarator_.Parameters();
    if (!declared) throw std::runtime_error("'" + name + "' is not a function");
    if (!ResolveParams(*declared, ctx).types.empty()) {
        throw std::runtime_error("not in IR yet: function parameters");
    }
    if (!type->Base()->IsVoid()) {
        RequireIrInteger(*type->Base());
        // A narrow return value must be extended to the caller's convention,
        // which is the step 3 classifier's job; fail loudly until then.
        if (type->Base()->Size() < Target().int_size) {
            throw std::runtime_error("not in IR yet: return type narrower than int");
        }
    }
    ctx.DeclareFunction(name, type);
    ctx.BeginFunction(type->Base());
    gen.BeginFunction(name, storage_ != Storage::Static, type->Base());
    ctx.PushScope();
    body_->EmitIR(gen);
    ctx.PopScope();
    gen.EndFunction();
}

void TranslationUnit::EmitIR(IRGen& gen) const {
    gen.Ctx().PushScope();
    for (const ExternalDeclPtr& decl : decls_) decl->EmitIR(gen);
    gen.Ctx().PopScope();
}

}  // namespace ast
