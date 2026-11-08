#pragma once
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "ast_node.hpp"

namespace ast {

// Parse-only helpers (design section 1). They are not code-generating Nodes:
// they are turned into types during code generation, when scope information
// (later: tags, enum constants) is available.

// The type-specifier words of a declaration: int, unsigned, void.
class TypeSpecifier {
public:
    enum class Kind { Int, Unsigned, Void };
    explicit TypeSpecifier(Kind kind) : kind_(kind) {}
    TypePtr Resolve(Context& ctx) const;

private:
    Kind kind_;
};

struct Param;

// What is declared and how the base type is modified. One concrete class: a
// kind plus the declarator it wraps, so it can be held by value.
//   Name         the identifier (empty for an unnamed parameter)
//   Pointer      '*' applied to the wrapped declarator
//   Array        wrapped declarator followed by [size]
//   Function     wrapped declarator followed by (parameters)
// Apply works inside-out: each wrapper changes the base type and passes it to
// its inner declarator, so 'int *a[3]' (array of pointers) and
// 'int (*a)[3]' (pointer to array) come out different.
class Declarator {
public:
    static Declarator Name(std::string name);
    static Declarator Pointer(std::unique_ptr<Declarator> inner);
    // 'size' may be null for 'a[]'.
    static Declarator Array(std::unique_ptr<Declarator> inner, ExprPtr size);
    static Declarator Function(std::unique_ptr<Declarator> inner,
                               std::vector<Param> params);

    Declarator(Declarator&&) noexcept;
    Declarator& operator=(Declarator&&) noexcept;
    ~Declarator();

    // Builds the declared type from the base type; returns (name, type).
    std::pair<std::string, TypePtr> Apply(const TypePtr& base,
                                          Context& ctx) const;

    // The parameters of the function this declarator directly declares
    // (the Function wrapper around the name), or null if it is not one.
    const std::vector<Param>* Parameters() const;

private:
    enum class Kind { Name, Pointer, Array, Function };
    explicit Declarator(Kind kind);

    Kind kind_;
    std::string name_;
    std::unique_ptr<Declarator> inner_;
    ExprPtr size_;
    std::vector<Param> params_;
};

// The '= expr' part of a declarator. Brace lists come later.
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

// Parameter names and types after applying the declarators. A lone unnamed
// 'void' means "no parameters"; any other void parameter is an error. Array
// parameters are adjusted to pointers.
struct ResolvedParams {
    std::vector<std::string> names;
    std::vector<TypePtr> types;
};

ResolvedParams ResolveParams(const std::vector<Param>& params, Context& ctx);

}  // namespace ast
