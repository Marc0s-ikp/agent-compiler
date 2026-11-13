#include "type.hpp"

#include <algorithm>
#include <stdexcept>

#include "target.hpp"

struct Type::RecordData {
    bool is_union;
    std::string tag;
    bool complete = false;
    std::vector<RecordMember> members;
    int size = 0;
    int align = 1;
};

TypePtr Type::Void() { return TypePtr(new Type(Kind::Void, 0, false)); }

TypePtr Type::Integer(int size, bool is_signed) {
    return TypePtr(new Type(Kind::Integer, size, is_signed));
}

TypePtr Type::Int() { return Integer(Target().int_size, true); }
TypePtr Type::UInt() { return Integer(Target().int_size, false); }
TypePtr Type::PtrDiff() { return Integer(Target().pointer_size, true); }
TypePtr Type::Float() { return Floating(Target().float_size); }
TypePtr Type::Double() { return Floating(Target().double_size); }

TypePtr Type::Floating(int size) {
    return TypePtr(new Type(Kind::Floating, size, true));
}

TypePtr Type::Pointer(TypePtr pointee) {
    Type* t = new Type(Kind::Pointer, Target().pointer_size, false);
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

TypePtr Type::Record(bool is_union, std::string tag) {
    Type* t = new Type(Kind::Record, 0, false);
    t->record_ = std::make_shared<RecordData>();
    t->record_->is_union = is_union;
    t->record_->tag = std::move(tag);
    return TypePtr(t);
}

namespace {
int RoundUpTo(int value, int multiple) {
    return (value + multiple - 1) / multiple * multiple;
}
}  // namespace

// A struct places each member at the next offset its alignment allows; a
// union puts every member at 0. The size is rounded up to the alignment so
// that arrays of the record keep every element aligned.
void Type::DefineRecord(const TypePtr& record, std::vector<RecordMember> members) {
    RecordData& data = *record->record_;
    if (data.complete) throw std::runtime_error("redefinition of struct or union");
    int offset = 0;
    int size = 0;
    int align = 1;
    for (size_t i = 0; i < members.size(); ++i) {
        RecordMember& m = members[i];
        if (m.type->IsVoid() || m.type->IsFunction() || m.type->Size() == 0) {
            throw std::runtime_error("member '" + m.name + "' has incomplete type");
        }
        for (size_t j = 0; j < i; ++j) {
            if (members[j].name == m.name) {
                throw std::runtime_error("duplicate member '" + m.name + "'");
            }
        }
        align = std::max(align, m.type->Align());
        if (data.is_union) {
            m.offset = 0;
            size = std::max(size, m.type->Size());
        } else {
            offset = RoundUpTo(offset, m.type->Align());
            m.offset = offset;
            offset += m.type->Size();
            size = offset;
        }
    }
    if (members.empty()) throw std::runtime_error("struct or union has no members");
    data.members = std::move(members);
    data.size = RoundUpTo(size, align);
    data.align = align;
    data.complete = true;
}

void Type::ReleaseRecord(const TypePtr& record) {
    record->record_->members.clear();
}

bool Type::IsUnion() const { return record_->is_union; }
const std::string& Type::Tag() const { return record_->tag; }
bool Type::IsComplete() const { return record_->complete; }
const std::vector<RecordMember>& Type::Members() const { return record_->members; }

const RecordMember* Type::FindMember(const std::string& name) const {
    for (const RecordMember& m : record_->members) {
        if (m.name == name) return &m;
    }
    return nullptr;
}

int Type::Size() const {
    if (kind_ == Kind::Array) return base_->Size() * length_;
    if (kind_ == Kind::Record) return record_->size;
    return size_;
}

// An array aligns as its element, a record as its most aligned member.
int Type::Align() const {
    if (kind_ == Kind::Array) return base_->Align();
    if (kind_ == Kind::Record) return record_->align;
    if (kind_ == Kind::Floating && size_ == Target().double_size) {
        return Target().double_align;
    }
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
        case Kind::Record: return record_ == o.record_;
    }
    return false;
}

// Integer types narrower than int become int.
TypePtr Promote(const TypePtr& t) {
    if (t->IsInteger() && t->Size() < Target().int_size) return Type::Int();
    return t;
}

// The common type both operands are converted to before an arithmetic op.
// A floating operand wins; of two, the wider one.
TypePtr UsualArithmetic(const TypePtr& a, const TypePtr& b) {
    if (a->IsFloating() || b->IsFloating()) {
        bool wide = (a->IsFloating() && a->Size() == Target().double_size) ||
                    (b->IsFloating() && b->Size() == Target().double_size);
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
