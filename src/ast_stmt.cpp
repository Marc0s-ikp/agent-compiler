#include "ast_stmt.hpp"
#include "ir_gen.hpp"
#include "ast_lower.hpp"

#include <stdexcept>

#include "emit.hpp"
#include "global_init.hpp"
#include "local_init.hpp"
#include "record_abi.hpp"

namespace ast {

void CompoundStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    ctx.PushScope();
    for (const StmtPtr& stmt : body_) stmt->EmitRISC(os, ctx);
    ctx.PopScope();
}

void ExprStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    if (expr_) expr_->EmitRISC(os, ctx);
}

// A static local lives in .data/.bss under its own label; its initialiser is
// a compile-time constant, evaluated once like a global's.
void DeclarationStmt::DeclareStatic(const std::string& name, const TypePtr& type,
                                    const Initializer* init,
                                    Context& ctx) const {
    // Declared before the initialiser is built, so that it can name itself.
    std::string label = ctx.AddStaticLocal(type);
    ctx.Declare(name, Symbol::Global(type, label));
    if (!init) {
        if (type->Size() == 0) throw std::runtime_error("incomplete type or missing array size");
        return;
    }
    GlobalInit built = BuildGlobalInit(type, *init, ctx);
    if (built.type->Size() == 0) throw std::runtime_error("incomplete type or missing array size");
    ctx.CompleteStaticLocal(label, built.type, built.data);
    ctx.Declare(name, Symbol::Global(built.type, label));
}

// An ordinary local. The initialiser is laid out first because an array
// declared without a length takes it from there ('char s[] = "abc"').
void DeclarationStmt::DeclareLocal(std::ostream& os, const std::string& name,
                                   TypePtr type, const Initializer* init,
                                   Context& ctx) const {
    FlatInit flat;
    if (init) {
        flat = FlattenInit(type, *init, ctx);
        type = flat.type;
    }
    if (type->Size() == 0) throw std::runtime_error("incomplete type or missing array size");
    int offset = ctx.AllocLocal(type);
    ctx.Declare(name, Symbol::Local(type, offset));
    if (init) EmitLocalInit(os, ctx, flat, offset);
}

void DeclarationStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    TypePtr base = spec_.Resolve(ctx);
    for (const InitDeclarator& d : declarators_) {
        auto [name, type] = d.declarator.Apply(base, ctx);
        if (storage_ != Storage::Typedef &&
            (type->IsVoid() || type->IsFunction())) {
            throw std::runtime_error("invalid type for variable '" + name + "'");
        }
        if (storage_ == Storage::Typedef) {
            ctx.Declare(name, Symbol::Typedef(type));
            continue;
        }
        if (storage_ == Storage::Static) {
            DeclareStatic(name, type, d.init.get(), ctx);
            continue;
        }
        DeclareLocal(os, name, type, d.init.get(), ctx);
    }
}

void ReturnStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    if (value_) {
        value_->EmitRISC(os, ctx);
        const TypePtr& to = ctx.ReturnType();
        EmitConvert(os, *value_->ValueType(ctx), *to);
        if (to->IsRecord()) EmitReturnRecord(os, ctx, *to);
    }
    os << "\tj " << ctx.ReturnLabel() << "\n";
}


// ---- --ir lowering ----

using ir::Operand;

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
        RequireIrScalar(*type);
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
    RequireIrScalar(*to);
    gen.Return(ValueAs(gen, *value_, *to));
}

}  // namespace ast
