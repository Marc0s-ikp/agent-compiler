#include "ast_node.hpp"

#include <cxxabi.h>

#include <cstdlib>
#include <stdexcept>
#include <typeinfo>

namespace ast {

namespace {
// "not in IR yet: ast::Foo", so a failing test says which node is missing.
std::runtime_error NotInIR(const Node& node) {
    int status = 0;
    char* name = abi::__cxa_demangle(typeid(node).name(), nullptr, nullptr, &status);
    std::string text = std::string("not in IR yet: ") + (status == 0 ? name : typeid(node).name());
    std::free(name);
    return std::runtime_error(text);
}
}  // namespace

void Node::EmitIR(IRGen&) const { throw NotInIR(*this); }

ir::Operand Expr::EmitValue(IRGen&) const { throw NotInIR(*this); }

ir::Operand Expr::EmitIRAddress(IRGen&) const {
    throw std::runtime_error("expression is not an lvalue (or not in IR yet)");
}

bool Expr::IsNullPointerConstant(const Context& ctx) const {
    return GetType(ctx)->IsInteger() && EvalConst(ctx) == 0L;
}

std::optional<ConstValue> Expr::EvalConstValue(Context& ctx) const {
    std::optional<long> n = EvalConst(ctx);
    if (!n) return std::nullopt;
    return ConstValue{"", *n};
}

}  // namespace ast
