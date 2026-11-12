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
                      Storage storage)
        : spec_(spec),
          declarators_(std::move(declarators)),
          storage_(storage) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;
    void EmitIR(IRGen& gen) const override;

private:
    void Declare(Context& ctx) const;
    void MarkLinkage(const std::string& name, Context& ctx) const;
    void DeclareVariable(const std::string& name, const TypePtr& type,
                         const Initializer* init, Context& ctx) const;

    TypeSpecifier spec_;
    std::vector<InitDeclarator> declarators_;
    Storage storage_;
};

class FunctionDefinition : public ExternalDecl {
public:
    FunctionDefinition(TypeSpecifier spec, Declarator declarator,
                       std::unique_ptr<CompoundStmt> body, Storage storage)
        : storage_(storage),
          spec_(spec),
          declarator_(std::move(declarator)),
          body_(std::move(body)) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;
    void EmitIR(IRGen& gen) const override;

private:
    Storage storage_;
    TypeSpecifier spec_;
    Declarator declarator_;
    std::unique_ptr<CompoundStmt> body_;
};

class TranslationUnit : public Node {
public:
    explicit TranslationUnit(std::vector<ExternalDeclPtr> decls)
        : decls_(std::move(decls)) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;
    void EmitIR(IRGen& gen) const override;

private:
    std::vector<ExternalDeclPtr> decls_;
};

}  // namespace ast
