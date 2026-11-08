#pragma once
#include <memory>
#include <string>
#include <utility>

#include "ast_node.hpp"

namespace ast {

// Parse-only helpers (design section 1). They are not code-generating Nodes:
// they are turned into types during code generation, when scope information
// (later: tags, enum constants, array bounds) is available.

// The type-specifier words of a declaration: int, unsigned, void.
class TypeSpecifier {
public:
    enum class Kind { Int, Unsigned, Void };
    explicit TypeSpecifier(Kind kind) : kind_(kind) {}
    TypePtr Resolve(Context& ctx) const;

private:
    Kind kind_;
};

// What is declared and how the base type is modified. Only the plain name
// exists so far; pointer, array and function declarators come with later
// features and extend Apply.
class Declarator {
public:
    explicit Declarator(std::string name) : name_(std::move(name)) {}
    const std::string& Name() const { return name_; }
    // Builds the declared type from the base type; returns (name, type).
    std::pair<std::string, TypePtr> Apply(const TypePtr& base,
                                          Context& ctx) const;

private:
    std::string name_;
};

// The '= expr' part of a declarator. Brace lists are added with arrays.
class Initializer {
public:
    explicit Initializer(ExprPtr expr) : expr_(std::move(expr)) {}
    const Expr& Value() const { return *expr_; }

private:
    ExprPtr expr_;
};

// One declarator with its own optional initialiser.
struct InitDeclarator {
    Declarator declarator;
    std::unique_ptr<Initializer> init;  // null if absent
};

// A function parameter: specifier plus declarator.
struct Param {
    TypeSpecifier spec;
    Declarator declarator;
};

}  // namespace ast
