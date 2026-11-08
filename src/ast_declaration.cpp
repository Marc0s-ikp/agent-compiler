#include "ast_declaration.hpp"

#include <climits>
#include <stdexcept>

namespace ast {

// Enums are int (design section 2). Each constant is declared as soon as its
// value is known, so later enumerators can use earlier ones.
TypePtr TypeSpecifier::ResolveEnum(Context& ctx) const {
    if (!enum_->has_body) {
        if (!enum_->tag.empty()) ctx.LookupTag(enum_->tag);
        return Type::Int();
    }
    long next = 0;
    for (const Enumerator& e : enum_->items) {
        if (e.value) {
            std::optional<long> v = e.value->EvalConst(ctx);
            if (!v) throw std::runtime_error("enumerator value is not constant");
            next = *v;
        }
        if (next < INT_MIN || next > INT_MAX) {
            throw std::runtime_error("enumerator value out of range");
        }
        ctx.Declare(e.name, Symbol::EnumConst(next));
        ++next;
    }
    if (!enum_->tag.empty()) ctx.DeclareTag(enum_->tag, Type::Int());
    return Type::Int();
}

TypePtr TypeSpecifier::Resolve(Context& ctx) const {
    switch (kind_) {
        case Kind::Enum: return ResolveEnum(ctx);
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
                // The inner dimensions are already in 'base', so this one
                // check bounds the whole array. Divide to avoid overflow.
                if (base->Size() == 0) {
                    throw std::runtime_error("array of incomplete element type");
                }
                if (*n > INT_MAX / base->Size()) {
                    throw std::runtime_error("array too large");
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
        if (type->IsFunction()) throw std::runtime_error("parameter of function type");
        if (type->IsVoid()) return {};
        out.names.push_back(name);
        out.types.push_back(Decay(type));
    }
    return out;
}

}  // namespace ast
