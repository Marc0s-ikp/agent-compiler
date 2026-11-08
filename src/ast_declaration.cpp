#include "ast_declaration.hpp"

#include <stdexcept>

namespace ast {

TypePtr TypeSpecifier::Resolve(Context&) const {
    switch (kind_) {
        case Kind::Unsigned: return Type::UInt();
        case Kind::Void: return Type::Void();
        case Kind::Int: break;
    }
    return Type::Int();
}

Declarator::Declarator(Kind kind) : kind_(kind) {}
Declarator::Declarator(Declarator&&) noexcept = default;
Declarator& Declarator::operator=(Declarator&&) noexcept = default;
Declarator::~Declarator() = default;

Declarator Declarator::Name(std::string name) {
    Declarator d(Kind::Name);
    d.name_ = std::move(name);
    return d;
}

Declarator Declarator::Pointer(std::unique_ptr<Declarator> inner) {
    Declarator d(Kind::Pointer);
    d.inner_ = std::move(inner);
    return d;
}

Declarator Declarator::Array(std::unique_ptr<Declarator> inner, ExprPtr size) {
    Declarator d(Kind::Array);
    d.inner_ = std::move(inner);
    d.size_ = std::move(size);
    return d;
}

Declarator Declarator::Function(std::unique_ptr<Declarator> inner,
                                std::vector<Param> params) {
    Declarator d(Kind::Function);
    d.inner_ = std::move(inner);
    d.params_ = std::move(params);
    return d;
}

std::pair<std::string, TypePtr> Declarator::Apply(const TypePtr& base,
                                                  Context& ctx) const {
    switch (kind_) {
        case Kind::Name:
            return {name_, base};
        case Kind::Pointer:
            return inner_->Apply(Type::Pointer(base), ctx);
        case Kind::Array: {
            int length = 0;
            if (size_) {
                std::optional<long> n = size_->EvalConst(ctx);
                if (!n || *n <= 0) {
                    throw std::runtime_error(
                        "array size must be a positive constant");
                }
                length = static_cast<int>(*n);
            }
            return inner_->Apply(Type::Array(base, length), ctx);
        }
        case Kind::Function:
            return inner_->Apply(
                Type::Function(base, ResolveParams(params_, ctx).types), ctx);
    }
    return {name_, base};
}

const std::vector<Param>* Declarator::Parameters() const {
    // Skip pointer wrappers: 'int *f(int x)' declares a function f.
    const Declarator* d = this;
    while (d->kind_ == Kind::Pointer) d = d->inner_.get();
    if (d->kind_ == Kind::Function && d->inner_->kind_ == Kind::Name) {
        return &d->params_;
    }
    return nullptr;
}

ResolvedParams ResolveParams(const std::vector<Param>& params, Context& ctx) {
    ResolvedParams out;
    for (const Param& p : params) {
        auto [name, type] = p.declarator.Apply(p.spec.Resolve(ctx), ctx);
        if (type->IsVoid() && (params.size() > 1 || !name.empty())) {
            throw std::runtime_error("parameter of type void");
        }
        if (type->IsVoid()) return {};
        out.names.push_back(name);
        out.types.push_back(Decay(type));
    }
    return out;
}

}  // namespace ast
