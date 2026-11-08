#pragma once
#include <string>
#include <vector>

#include "ast_node.hpp"
#include "ast_stmt.hpp"

namespace ast {

// Anything that can appear at file scope.
class ExternalDecl : public Node {};

using ExternalDeclPtr = std::unique_ptr<ExternalDecl>;

struct Param {
    std::string name;
    TypePtr type;
};

class FunctionDefinition : public ExternalDecl {
public:
    FunctionDefinition(TypePtr return_type, std::string name,
                       std::vector<Param> params,
                       std::unique_ptr<CompoundStmt> body)
        : return_type_(std::move(return_type)),
          name_(std::move(name)),
          params_(std::move(params)),
          body_(std::move(body)) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    void EmitParams(std::ostream& os, Context& ctx) const;

    TypePtr return_type_;
    std::string name_;
    std::vector<Param> params_;
    std::unique_ptr<CompoundStmt> body_;
};

class TranslationUnit : public Node {
public:
    explicit TranslationUnit(std::vector<ExternalDeclPtr> decls)
        : decls_(std::move(decls)) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    std::vector<ExternalDeclPtr> decls_;
};

}  // namespace ast
