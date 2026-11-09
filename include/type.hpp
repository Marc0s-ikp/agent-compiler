#pragma once
#include <memory>
#include <vector>

// Types are immutable shared value objects (D3). Record kinds are added with
// their feature; everything else asks the Type for size,
// alignment and kind.
class Type;
using TypePtr = std::shared_ptr<const Type>;

class Type {
public:
    enum class Kind { Void, Integer, Floating, Pointer, Array, Function };

    static TypePtr Void();
    static TypePtr Integer(int size, bool is_signed);
    static TypePtr Int() { return Integer(4, true); }
    static TypePtr UInt() { return Integer(4, false); }
    // size 4 is float, size 8 is double.
    static TypePtr Floating(int size);
    static TypePtr Float() { return Floating(4); }
    static TypePtr Double() { return Floating(8); }
    static TypePtr Pointer(TypePtr pointee);
    // length 0 means "unspecified" (only legal for parameters).
    static TypePtr Array(TypePtr element, int length);
    // An empty parameter list means none, or unspecified for 'f()'.
    static TypePtr Function(TypePtr return_type, std::vector<TypePtr> params);

    Kind GetKind() const { return kind_; }
    int Size() const;
    int Align() const;
    bool IsSigned() const { return signed_; }
    bool IsVoid() const { return kind_ == Kind::Void; }
    bool IsInteger() const { return kind_ == Kind::Integer; }
    bool IsPointer() const { return kind_ == Kind::Pointer; }
    bool IsArray() const { return kind_ == Kind::Array; }
    bool IsFunction() const { return kind_ == Kind::Function; }
    bool IsFloating() const { return kind_ == Kind::Floating; }
    // Integer or floating: the types the usual arithmetic conversions cover.
    bool IsArithmetic() const { return IsInteger() || IsFloating(); }
    // Scalars can be loaded into a register and tested for truth.
    bool IsScalar() const { return IsArithmetic() || IsPointer(); }

    // Pointee, array element or function return type.
    const TypePtr& Base() const { return base_; }
    int Length() const { return length_; }
    const std::vector<TypePtr>& Params() const { return params_; }

    // Structural equality.
    bool Equals(const Type& other) const;

private:
    Type(Kind kind, int size, bool is_signed)
        : kind_(kind), size_(size), signed_(is_signed) {}

    Kind kind_;
    int size_;      // Integer and Floating only; other kinds compute their size
    bool signed_;
    TypePtr base_;
    int length_ = 0;
    std::vector<TypePtr> params_;
};

// C rules for arithmetic types.
TypePtr Promote(const TypePtr& t);
TypePtr UsualArithmetic(const TypePtr& a, const TypePtr& b);
// Array to pointer to its element (D10). Other types are unchanged.
TypePtr Decay(const TypePtr& t);
