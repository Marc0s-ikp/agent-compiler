#include "ast_declaration.hpp"

#include <climits>
#include <stdexcept>

namespace ast {

// Enums are int (design section 2). Each constant is declared as soon as its
// value is known, so later enumerators can use earlier ones.
TypePtr TypeSpecifier::DeclareEnum(Context& ctx) const {
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

// The record type a tag names. Without a body, any visible tag will do and an
// unknown one is declared incomplete (so 'struct node *next' works inside
// 'struct node'). With a body, or in 'struct T;', only the current scope is
// searched, so an inner definition hides an outer tag instead of changing it.
TypePtr TypeSpecifier::FindOrDeclareTag(Context& ctx) const {
    if (record_->tag.empty()) return ctx.NewRecord(record_->is_union, "");
    bool here_only = record_->has_body || declaration_only_;
    TypePtr found = here_only ? ctx.FindTagHere(record_->tag)
                              : ctx.FindTag(record_->tag);
    if (found) return found;
    TypePtr created = ctx.NewRecord(record_->is_union, record_->tag);
    ctx.DeclareTag(record_->tag, created);
    return created;
}

// The tag is declared before the members are resolved, so a member may point
// to the record being defined.
TypePtr TypeSpecifier::ResolveRecord(Context& ctx) const {
    TypePtr record = FindOrDeclareTag(ctx);
    if (!record->IsRecord() || record->IsUnion() != record_->is_union) {
        throw std::runtime_error("'" + record_->tag + "' is a different kind of tag");
    }
    if (!record_->has_body) return record;
    std::vector<RecordMember> members;
    for (const MemberDecl& line : record_->members) {
        TypePtr base = line.spec.Resolve(ctx);
        for (const Declarator& d : line.declarators) {
            auto [name, type] = d.Apply(base, ctx);
            members.push_back(RecordMember{name, type, 0});
        }
    }
    Type::DefineRecord(record, std::move(members));
    return record;
}

TypePtr TypeSpecifier::Resolve(Context& ctx) const {
    if (kind_ == Kind::Enum && enum_->has_body) return DeclareEnum(ctx);
    if (kind_ == Kind::Record) return ResolveRecord(ctx);
    return Lookup(ctx);
}

TypePtr TypeSpecifier::LookupRecord(const Context& ctx) const {
    if (record_->has_body) {
        throw std::runtime_error("struct or union definition not allowed here");
    }
    TypePtr record = ctx.LookupTag(record_->tag);
    if (!record->IsRecord() || record->IsUnion() != record_->is_union) {
        throw std::runtime_error("'" + record_->tag + "' is a different kind of tag");
    }
    return record;
}

TypePtr TypeSpecifier::Lookup(const Context& ctx) const {
    switch (kind_) {
        case Kind::Enum:
            if (enum_->has_body) {
                throw std::runtime_error("enum definition not allowed here");
            }
            if (!enum_->tag.empty()) ctx.LookupTag(enum_->tag);
            return Type::Int();
        case Kind::Record: return LookupRecord(ctx);
        case Kind::TypedefName: {
            const Symbol& sym = ctx.Lookup(name_);
            if (sym.kind != SymbolKind::Typedef) {
                throw std::runtime_error("'" + name_ + "' is not a type");
            }
            return sym.type;
        }
        case Kind::Unsigned: return Type::UInt();
        case Kind::Char: return Type::Integer(1, false);  // unsigned (D8)
        case Kind::UnsignedChar: return Type::Integer(1, false);
        case Kind::Short: return Type::Integer(2, true);
        case Kind::UnsignedShort: return Type::Integer(2, false);
        case Kind::SignedChar: return Type::Integer(1, true);
        case Kind::Float: return Type::Float();
        case Kind::Double: return Type::Double();
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

std::pair<std::string, TypePtr> Declarator::Apply(
    const TypePtr& base, const Context& ctx) const {
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

const std::string& Declarator::DeclaredName() const {
    return kind_ == Kind::Name ? name_ : inner_->DeclaredName();
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

ResolvedParams ResolveParams(const std::vector<Param>& params,
                             const Context& ctx) {
    ResolvedParams out;
    for (const Param& p : params) {
        auto [name, type] = p.declarator.Apply(p.spec.Lookup(ctx), ctx);
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
