#pragma once
#include <cstdint>
#include <string>

#include "ast_node.hpp"

namespace ast {

class IntLiteral : public Expr {
public:
    // 'text' is the source spelling: decimal, octal or hex, optional u/l suffix.
    explicit IntLiteral(const std::string& text);
    TypePtr GetType(const Context& ctx) const override;
    std::optional<long> EvalConst(const Context& ctx) const override;
    ir::Operand EmitValue(IRGen& gen) const override;

private:
    // Throws if the value does not fit 32 bits. Checked at code generation,
    // not in the constructor, so the parser never throws.
    void CheckRange() const;

    unsigned long long value_;
    bool has_u_;
    std::string text_;
};

// 1.5, 2e3, .5f. No suffix is a double, f or F is a float. The value is
// rounded once, to the literal's own type, and loaded from the data pool
// (Context::AddFloat). A long double (L suffix) is not supported.
class FloatLiteral : public Expr {
public:
    explicit FloatLiteral(const std::string& text);
    TypePtr GetType(const Context& ctx) const override;
    std::optional<ConstValue> EvalConstValue(Context& ctx) const override;
    ir::Operand EmitValue(IRGen& gen) const override;

private:
    // Throws for the L suffix. Checked at code generation so the parser
    // never throws.
    void CheckSuffix() const;

    double value_;       // exactly the literal's type's value
    bool is_float_;
    bool is_long_;
};

// 'a', '\n', '\x41'. Its type is int (C), and plain char is unsigned (D8), so
// '\377' is 255.
class CharLiteral : public Expr {
public:
    explicit CharLiteral(unsigned char value) : value_(value) {}
    TypePtr GetType(const Context&) const override { return Type::Int(); }
    std::optional<long> EvalConst(const Context&) const override {
        return value_;
    }
    ir::Operand EmitValue(IRGen& gen) const override;

private:
    unsigned char value_;
};

// "text". Adjacent literals are joined by the parser. Its type is an array of
// char with room for the NUL, so sizeof("hi") is 3; it decays like any array
// (D10). The bytes live in the read-only data pool (Context::AddString).
class StringLiteral : public Expr {
public:
    explicit StringLiteral(std::string bytes) : bytes_(std::move(bytes)) {}
    TypePtr GetType(const Context&) const override;
    std::optional<ConstValue> EvalConstValue(Context& ctx) const override;
    std::optional<ConstValue> EvalConstAddress(Context& ctx) const override;
    const std::string* StringBytes() const override { return &bytes_; }
    ir::Operand EmitValue(IRGen& gen) const override;
    ir::Operand EmitIRAddress(IRGen& gen) const override;

private:
    std::string bytes_;
};

}  // namespace ast
