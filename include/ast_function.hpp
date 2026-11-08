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

// A file-scope declaration: variables with optional initialisers, 'extern'
// declarations, and function prototypes (which record a Function symbol and
// emit nothing, D9). Variable storage is written at the end of the file.
class GlobalDeclaration : public ExternalDecl {
public:
    GlobalDeclaration(TypeSpecifier spec, std::vector<InitDeclarator> declarators,
                      bool is_extern)
        : spec_(spec),
          declarators_(std::move(declarators)),
          is_extern_(is_extern) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    void DeclareVariable(const std::string& name, const TypePtr& type,
                         const Initializer* init, Context& ctx) const;

    TypeSpecifier spec_;
    std::vector<InitDeclarator> declarators_;
    bool is_extern_;
};

class FunctionDefinition : public ExternalDecl {
public:
    FunctionDefinition(TypeSpecifier spec, Declarator declarator,
                       std::unique_ptr<CompoundStmt> body)
        : spec_(spec),
          declarator_(std::move(declarator)),
          body_(std::move(body)) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    TypeSpecifier spec_;
    Declarator declarator_;
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
