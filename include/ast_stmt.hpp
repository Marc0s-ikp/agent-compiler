#pragma once
#include <string>
#include <vector>

#include "ast_declaration.hpp"
#include "ast_node.hpp"

namespace ast {

class CompoundStmt : public Stmt {
public:
    explicit CompoundStmt(std::vector<StmtPtr> body) : body_(std::move(body)) {}
    void EmitIR(IRGen& gen) const override;

private:
    std::vector<StmtPtr> body_;
};

// An expression followed by ';'. A null expression is the empty statement.
class ExprStmt : public Stmt {
public:
    explicit ExprStmt(ExprPtr expr) : expr_(std::move(expr)) {}
    void EmitIR(IRGen& gen) const override;

private:
    ExprPtr expr_;
};

class DeclarationStmt : public Stmt {
public:
    DeclarationStmt(TypeSpecifier spec, std::vector<InitDeclarator> declarators,
                    Storage storage)
        : storage_(storage),
          spec_(spec),
          declarators_(std::move(declarators)) {}
    void EmitIR(IRGen& gen) const override;

private:
    void DeclareLocalIR(IRGen& gen, const std::string& name, TypePtr type,
                        const Initializer* init) const;
    void DeclareStatic(const std::string& name, const TypePtr& type,
                       const Initializer* init, Context& ctx) const;

    Storage storage_;
    TypeSpecifier spec_;
    std::vector<InitDeclarator> declarators_;
};

class ReturnStmt : public Stmt {
public:
    explicit ReturnStmt(ExprPtr value) : value_(std::move(value)) {}
    void EmitIR(IRGen& gen) const override;

private:
    ExprPtr value_;  // null for 'return;'
};

}  // namespace ast
