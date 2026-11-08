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

// Storage class of a declaration. Extern is file scope only.
enum class Storage { None, Extern, Static };

// One enumerator of 'enum { A, B = 5 }'; no value means previous + 1.
struct Enumerator {
    std::string name;
    ExprPtr value;
};

// 'enum tag { ... }', 'enum { ... }' or just 'enum tag'.
struct EnumSpec {
    std::string tag;  // empty if anonymous
    bool has_body = false;
    std::vector<Enumerator> items;
};

// The type-specifier words of a declaration: int, unsigned, void, enum.
class TypeSpecifier {
public:
    enum class Kind { Int, Unsigned, Void, Enum };
    explicit TypeSpecifier(Kind kind) : kind_(kind) {}
    explicit TypeSpecifier(std::shared_ptr<const EnumSpec> spec)
        : kind_(Kind::Enum), enum_(std::move(spec)) {}
    // Resolving an enum with a body declares its tag and constants in the
    // current scope, so call it once per declaration.
    TypePtr Resolve(Context& ctx) const;

private:
    TypePtr ResolveEnum(Context& ctx) const;

    Kind kind_;
    std::shared_ptr<const EnumSpec> enum_;
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

// The '= ...' part of a declarator: a single expression, or a brace list of
// initialisers (which may nest).
class Initializer {
public:
    using List = std::vector<std::unique_ptr<Initializer>>;
    explicit Initializer(ExprPtr expr) : expr_(std::move(expr)) {}
    explicit Initializer(List items) : items_(std::move(items)), is_list_(true) {}
    bool IsList() const { return is_list_; }
    const Expr& Value() const { return *expr_; }  // only if !IsList()
    const List& Items() const { return items_; }  // only if IsList()

private:
    ExprPtr expr_;
    List items_;
    bool is_list_ = false;
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
