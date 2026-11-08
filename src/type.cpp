#include "type.hpp"

TypePtr Type::Void() { return TypePtr(new Type(Kind::Void, 0, false)); }

TypePtr Type::Integer(int size, bool is_signed) {
    return TypePtr(new Type(Kind::Integer, size, is_signed));
}

// Integer types narrower than int become int.
TypePtr Promote(const TypePtr& t) {
    if (t->IsInteger() && t->Size() < 4) return Type::Int();
    return t;
}

// The common type both operands are converted to before an arithmetic op.
TypePtr UsualArithmetic(const TypePtr& a, const TypePtr& b) {
    TypePtr pa = Promote(a);
    TypePtr pb = Promote(b);
    if (pa->Size() != pb->Size()) return pa->Size() > pb->Size() ? pa : pb;
    if (!pa->IsSigned()) return pa;
    return pb;
}
