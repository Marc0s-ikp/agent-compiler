#pragma once
#include <memory>

// Types are immutable shared value objects (D3). Feature 1 needs only void
// and integers; later kinds (floating, pointer, array, record, function) are
// added here and everything else asks the Type for size, alignment and kind.
class Type;
using TypePtr = std::shared_ptr<const Type>;

class Type {
public:
    enum class Kind { Void, Integer };

    static TypePtr Void();
    static TypePtr Integer(int size, bool is_signed);
    static TypePtr Int() { return Integer(4, true); }
    static TypePtr UInt() { return Integer(4, false); }

    Kind GetKind() const { return kind_; }
    int Size() const { return size_; }
    int Align() const { return size_ == 0 ? 1 : size_; }
    bool IsSigned() const { return signed_; }
    bool Equals(const Type& o) const {
        return kind_ == o.kind_ && size_ == o.size_ && signed_ == o.signed_;
    }
    bool IsVoid() const { return kind_ == Kind::Void; }
    bool IsInteger() const { return kind_ == Kind::Integer; }
    bool IsScalar() const { return IsInteger(); }

private:
    Type(Kind kind, int size, bool is_signed)
        : kind_(kind), size_(size), signed_(is_signed) {}
    Kind kind_;
    int size_;
    bool signed_;
};

// C rules for arithmetic types.
TypePtr Promote(const TypePtr& t);
TypePtr UsualArithmetic(const TypePtr& a, const TypePtr& b);
