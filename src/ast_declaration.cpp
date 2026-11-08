#include "ast_declaration.hpp"

namespace ast {

TypePtr TypeSpecifier::Resolve(Context&) const {
    switch (kind_) {
        case Kind::Unsigned: return Type::UInt();
        case Kind::Void: return Type::Void();
        case Kind::Int: break;
    }
    return Type::Int();
}

std::pair<std::string, TypePtr> Declarator::Apply(const TypePtr& base,
                                                  Context&) const {
    return {name_, base};
}

}  // namespace ast
