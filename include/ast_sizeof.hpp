#pragma once
#include "ast_declaration.hpp"
#include "ast_node.hpp"

namespace ast {

// sizeof expression. The operand is never evaluated: only its type is used
// (GetType is pure, D3), and the size is known at compile time.
class SizeofExpr : public Expr {
public:
    explicit SizeofExpr(ExprPtr operand) : operand_(std::move(operand)) {}
    TypePtr GetType(const Context& ctx) const override;
    std::optional<long> EvalConst(const Context& ctx) const override;
    ir::Operand EmitValue(IRGen& gen) const override;

private:
    ExprPtr operand_;
};

// sizeof(type-name). A type name is a specifier plus an abstract declarator,
// which is a Param whose declarator has no name: 'int *[5]', 'char'.
class SizeofType : public Expr {
public:
    explicit SizeofType(Param type_name) : type_(std::move(type_name)) {}
    TypePtr GetType(const Context& ctx) const override;
    std::optional<long> EvalConst(const Context& ctx) const override;
    ir::Operand EmitValue(IRGen& gen) const override;

private:
    Param type_;
};

}  // namespace ast
