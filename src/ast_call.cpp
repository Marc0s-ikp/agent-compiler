#include "ast_call.hpp"

#include <stdexcept>

#include "ast_lower.hpp"
#include "ast_operators.hpp"

namespace ast {

namespace {

}  // namespace

TypePtr FunctionCall::GetType(const Context& ctx) const {
    const Symbol* sym = ctx.Find(name_);
    if (!sym) return Type::Int();
    if (sym->kind != SymbolKind::Function) {
        throw std::runtime_error("'" + name_ + "' is not a function");
    }
    return sym->type->Base();
}

// What an argument with no declared parameter is passed as: integers
// promote to int, and float to double.
static TypePtr DefaultPromote(const TypePtr& t) {
    return t->IsFloating() ? Type::Double() : Promote(t);
}

std::vector<TypePtr> FunctionCall::ArgumentTypes(const Context& ctx) const {
    GetType(ctx);  // rejects calling a non-function
    const Symbol* sym = ctx.Find(name_);
    const std::vector<TypePtr> none;
    const std::vector<TypePtr>& params = sym ? sym->type->Params() : none;
    size_t declared = params.size();
    if (args_.size() < declared) {
        throw std::runtime_error("too few arguments to '" + name_ + "'");
    }
    std::vector<TypePtr> types;
    for (size_t i = 0; i < args_.size(); ++i) {
        TypePtr own = args_[i]->ValueType(ctx);
        if (!own->IsRecord()) RequireScalar(own);
        types.push_back(i < declared ? params[i] : DefaultPromote(own));
    }
    return types;
}

// Arguments are evaluated in source order, each converted to its parameter's
// type (or default-promoted when there is none); where they go is the
// backend's business (21.5). A call is a value-producing instruction, so
// nested calls need no special care: every value is already a temp.
ir::Operand FunctionCall::EmitValue(IRGen& gen) const {
    Context& ctx = gen.Ctx();
    std::vector<TypePtr> types = ArgumentTypes(ctx);
    TypePtr result_type = GetType(ctx);
    if (!result_type->IsVoid()) RequireIrValue(*result_type);
    std::vector<ir::Operand> values;
    for (size_t i = 0; i < args_.size(); ++i) {
        ir::Operand value = ValueAs(gen, *args_[i], *types[i]);
        // A struct argument is a copy, so the callee and later arguments cannot change it (21.5).
        values.push_back(types[i]->IsRecord() ? gen.CopyRecord(value, *types[i]) : value);
    }
    return gen.Call(name_, values, types, result_type);
}

}  // namespace ast
