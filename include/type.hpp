#pragma once
#include <memory>
#include <string>
#include <vector>

// Types are immutable shared value objects (D3). Record kinds are added with
// their feature; everything else asks the Type for size,
// alignment and kind.
class Type;
using TypePtr = std::shared_ptr<const Type>;

// One member of a struct or union: its type and byte offset (always 0 in a
// union).
struct RecordMember {
    std::string name;
    TypePtr type;
    int offset;
};

class Type {
public:
    enum class Kind { Void, Integer, Floating, Pointer, Array, Function, Record };

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
    // A struct or union, incomplete (no members, size 0) until DefineRecord.
    // Every call makes a distinct type: records are the same type only if
    // they are the same object (D19).
    static TypePtr Record(bool is_union, std::string tag);
    // Completes 'record' (made by Record): lays the members out and fixes its
    // size and alignment. Types are otherwise immutable; completing a record
    // is the one change, so a pointer to it made earlier sees the members.
    // Throws if it is already complete, or a member is unusable.
    static void DefineRecord(const TypePtr& record, std::vector<RecordMember> members);
    // Drops the members of 'record'. A record can reach itself through a
    // member (struct node { struct node *next; }), which shared pointers
    // never free; whoever owns the records calls this when compiling is done.
    static void ReleaseRecord(const TypePtr& record);

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
    bool IsRecord() const { return kind_ == Kind::Record; }
    // Record only.
    bool IsUnion() const;
    const std::string& Tag() const;  // empty if anonymous
    bool IsComplete() const;         // false for a record not yet defined
    const std::vector<RecordMember>& Members() const;
    const RecordMember* FindMember(const std::string& name) const;  // null if none
    // Integer or floating: the types the usual arithmetic conversions cover.
    bool IsArithmetic() const { return IsInteger() || IsFloating(); }
    // Scalars can be loaded into a register and tested for truth.
    bool IsScalar() const { return IsArithmetic() || IsPointer(); }

    // Pointee, array element or function return type.
    const TypePtr& Base() const { return base_; }
    int Length() const { return length_; }
    const std::vector<TypePtr>& Params() const { return params_; }

    // Structural equality; two records are equal only if they are identical.
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
    // Record only. Shared so that a completed record is seen by every copy
    // of the pointer; this is also what identity means.
    struct RecordData;
    std::shared_ptr<RecordData> record_;
};

// C rules for arithmetic types.
TypePtr Promote(const TypePtr& t);
TypePtr UsualArithmetic(const TypePtr& a, const TypePtr& b);
// Array to pointer to its element (D10). Other types are unchanged.
TypePtr Decay(const TypePtr& t);
