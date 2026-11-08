#pragma once
#include <string>
#include <vector>

#include "ast_declaration.hpp"
#include "ast_node.hpp"
#include "ast_stmt.hpp"

namespace ast {

// Anything that can appear at file scope.
class ExternalDecl : public Node {};

using ExternalDeclPtr = std::unique_ptr<ExternalDecl>;

class FunctionDefinition : public ExternalDecl {
public:
    FunctionDefinition(TypeSpecifier return_spec, Declarator declarator,
                       std::vector<Param> params,
                       std::unique_ptr<CompoundStmt> body)
        : return_spec_(return_spec),
          declarator_(std::move(declarator)),
          params_(std::move(params)),
          body_(std::move(body)) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    void EmitParams(std::ostream& os, Context& ctx) const;

    TypeSpecifier return_spec_;
    Declarator declarator_;
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
