#pragma once
#include <string>
#include <vector>

#include "ast_declaration.hpp"
#include "ast_node.hpp"

namespace ast {

class CompoundStmt : public Stmt {
public:
    explicit CompoundStmt(std::vector<StmtPtr> body) : body_(std::move(body)) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    std::vector<StmtPtr> body_;
};

// An expression followed by ';'. A null expression is the empty statement.
class ExprStmt : public Stmt {
public:
    explicit ExprStmt(ExprPtr expr) : expr_(std::move(expr)) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    ExprPtr expr_;
};

class DeclarationStmt : public Stmt {
public:
    DeclarationStmt(TypeSpecifier spec, std::vector<InitDeclarator> declarators)
        : spec_(spec), declarators_(std::move(declarators)) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    TypeSpecifier spec_;
    std::vector<InitDeclarator> declarators_;
};

class ReturnStmt : public Stmt {
public:
    explicit ReturnStmt(ExprPtr value) : value_(std::move(value)) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    ExprPtr value_;  // null for 'return;'
};

}  // namespace ast
