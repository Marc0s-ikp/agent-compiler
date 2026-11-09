#include "type.hpp"

TypePtr Type::Void() { return TypePtr(new Type(Kind::Void, 0, false)); }

TypePtr Type::Integer(int size, bool is_signed) {
    return TypePtr(new Type(Kind::Integer, size, is_signed));
}

TypePtr Type::Floating(int size) {
    return TypePtr(new Type(Kind::Floating, size, true));
}

TypePtr Type::Pointer(TypePtr pointee) {
    Type* t = new Type(Kind::Pointer, 4, false);
    t->base_ = std::move(pointee);
    return TypePtr(t);
}

TypePtr Type::Array(TypePtr element, int length) {
    Type* t = new Type(Kind::Array, 0, false);
    t->base_ = std::move(element);
    t->length_ = length;
    return TypePtr(t);
}

TypePtr Type::Function(TypePtr return_type, std::vector<TypePtr> params) {
    Type* t = new Type(Kind::Function, 0, false);
    t->base_ = std::move(return_type);
    t->params_ = std::move(params);
    return TypePtr(t);
}

int Type::Size() const {
    if (kind_ == Kind::Array) return base_->Size() * length_;
    return size_;
}

// An array aligns as its element.
int Type::Align() const {
    if (kind_ == Kind::Array) return base_->Align();
    return size_ == 0 ? 1 : size_;
}

bool Type::Equals(const Type& o) const {
    if (kind_ != o.kind_) return false;
    switch (kind_) {
        case Kind::Void: return true;
        case Kind::Floating: return size_ == o.size_;
        case Kind::Integer: return size_ == o.size_ && signed_ == o.signed_;
        case Kind::Pointer: return base_->Equals(*o.base_);
        case Kind::Array:
            return length_ == o.length_ && base_->Equals(*o.base_);
        case Kind::Function:
            if (!base_->Equals(*o.base_) || params_.size() != o.params_.size()) {
                return false;
            }
            for (size_t i = 0; i < params_.size(); ++i) {
                if (!params_[i]->Equals(*o.params_[i])) return false;
            }
            return true;
    }
    return false;
}

// Integer types narrower than int become int.
TypePtr Promote(const TypePtr& t) {
    if (t->IsInteger() && t->Size() < 4) return Type::Int();
    return t;
}

// The common type both operands are converted to before an arithmetic op.
// A floating operand wins; of two, the wider one.
TypePtr UsualArithmetic(const TypePtr& a, const TypePtr& b) {
    if (a->IsFloating() || b->IsFloating()) {
        bool wide = (a->IsFloating() && a->Size() == 8) ||
                    (b->IsFloating() && b->Size() == 8);
        return wide ? Type::Double() : Type::Float();
    }
    TypePtr pa = Promote(a);
    TypePtr pb = Promote(b);
    if (pa->Size() != pb->Size()) return pa->Size() > pb->Size() ? pa : pb;
    if (!pa->IsSigned()) return pa;
    return pb;
}

TypePtr Decay(const TypePtr& t) {
    if (t->IsArray()) return Type::Pointer(t->Base());
    return t;
}
