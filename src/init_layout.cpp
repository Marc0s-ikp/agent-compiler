#include "init_layout.hpp"

#include <stdexcept>

namespace ast {

namespace {

bool IsCharArray(const Type& t) {
    return t.IsArray() && t.Base()->IsInteger() && t.Base()->Size() == 1;
}

// Walks the initialiser tree and records the items. This is the one place
// that knows how a type's elements and members are laid out.
class Flattener {
public:
    explicit Flattener(const Context& ctx) : ctx_(ctx) {}

    std::vector<InitItem> TakeItems() { return std::move(items_); }

    // The string literal that initialises this char array: 'item' is "ab" or
    // {"ab"}. Null when the item is anything else.
    const std::string* StringFor(const TypePtr& type,
                                 const Initializer& item) const {
        if (!IsCharArray(*type)) return nullptr;
        if (!item.IsList()) return item.Value().StringBytes();
        const Initializer::List& inner = item.Items();
        if (inner.size() != 1 || inner[0]->IsList()) return nullptr;
        return inner[0]->Value().StringBytes();
    }

    // A char array from a string; returns the array length. The NUL is only
    // stored if there is room, as in C (char s[2] = "ab" has no NUL).
    int Bytes(const TypePtr& type, const std::string& bytes, int offset) {
        int length = type->Length();
        if (length == 0) length = static_cast<int>(bytes.size()) + 1;
        if (static_cast<int>(bytes.size()) > length) {
            throw std::runtime_error("initialiser string is too long");
        }
        items_.push_back(InitItem{offset, Type::Array(type->Base(), length),
                                  nullptr, bytes});
        return length;
    }

    // A whole struct or union object: 'T x = { ... }' or 'T x = other'.
    void TopRecord(const TypePtr& type, const Initializer& init) {
        if (init.IsList()) {
            size_t pos = 0;
            return Record(type, init.Items(), pos, true, 0);
        }
        if (!IsRecordValue(type, init)) {
            throw std::runtime_error("invalid initialiser for a struct or union");
        }
        items_.push_back(InitItem{0, type, &init.Value(), ""});
    }

    // One scalar from a bare expression or a braced single value.
    void Scalar(const TypePtr& type, const Initializer& item, int offset) {
        if (item.IsList()) {
            const Initializer::List& inner = item.Items();
            if (inner.size() != 1 || inner[0]->IsList()) {
                throw std::runtime_error("invalid scalar initialiser");
            }
            return Scalar(type, *inner[0], offset);
        }
        if (!type->IsScalar()) {
            throw std::runtime_error("unsupported initialiser type");
        }
        items_.push_back(InitItem{offset, type, &item.Value(), ""});
    }

    // True if 'item' is an expression whose type is exactly this record.
    bool IsRecordValue(const TypePtr& type, const Initializer& item) const {
        return !item.IsList() && item.Value().ValueType(ctx_)->Equals(*type);
    }

    // Fills a struct or union from list[pos...], the way Array does. A union
    // takes only its first member.
    void Record(const TypePtr& type, const Initializer::List& list, size_t& pos,
                bool braced, int offset) {
        if (!type->IsComplete()) throw std::runtime_error("initialiser for an incomplete type");
        const std::vector<RecordMember>& members = type->Members();
        size_t used = type->IsUnion() ? 1 : members.size();
        for (size_t i = 0; i < used && pos < list.size(); ++i) {
            Object(members[i].type, list, pos, offset + members[i].offset);
        }
        if (braced && pos < list.size()) {
            throw std::runtime_error("excess elements in struct initialiser");
        }
    }

    // A struct at list[pos]: a struct-valued expression, a braced list for
    // it, or (brace elision) the start of its members in the enclosing list.
    void RecordObject(const TypePtr& type, const Initializer::List& list,
                      size_t& pos, int offset) {
        const Initializer& item = *list[pos];
        if (IsRecordValue(type, item)) {
            items_.push_back(InitItem{offset, type, &item.Value(), ""});
            ++pos;
        } else if (item.IsList()) {
            size_t inner_pos = 0;
            Record(type, item.Items(), inner_pos, true, offset);
            ++pos;
        } else {
            Record(type, list, pos, false, offset);
        }
    }

    // The object that starts at list[pos]; advances pos past what it used.
    void Object(const TypePtr& type, const Initializer::List& list,
                size_t& pos, int offset) {
        const Initializer& item = *list[pos];
        if (type->IsRecord()) {
            RecordObject(type, list, pos, offset);
        } else if (!type->IsArray()) {
            Scalar(type, item, offset);
            ++pos;
        } else if (const std::string* s = StringFor(type, item)) {
            Bytes(type, *s, offset);
            ++pos;
        } else if (item.IsList()) {
            size_t inner_pos = 0;
            Array(type, item.Items(), inner_pos, true, offset);
            ++pos;
        } else {
            Array(type, list, pos, false, offset);  // brace elision: {1, 2, 3, 4}
        }
    }

    // Fills an array from list[pos...]. 'braced' means the list belongs to
    // this array, so leftovers are an error. A length of 0 is unspecified:
    // all elements are taken. Returns the element count.
    int Array(const TypePtr& type, const Initializer::List& list, size_t& pos,
              bool braced, int offset) {
        int length = type->Length();
        int step = type->Base()->Size();
        int count = 0;
        while (pos < list.size() && (length == 0 || count < length)) {
            Object(type->Base(), list, pos, offset + count * step);
            ++count;
        }
        if (length == 0 && count == 0) {
            throw std::runtime_error("empty initialiser for unsized array");
        }
        if (braced && pos < list.size()) {
            throw std::runtime_error("excess elements in array initialiser");
        }
        return count;
    }

private:
    const Context& ctx_;
    std::vector<InitItem> items_;
};

}  // namespace

FlatInit FlattenInit(const TypePtr& type, const Initializer& init,
                     const Context& ctx) {
    Flattener flat(ctx);
    TypePtr result = type;
    if (type->IsRecord()) {
        flat.TopRecord(type, init);
    } else if (!type->IsArray()) {
        flat.Scalar(type, init, 0);
    } else if (const std::string* s = flat.StringFor(type, init)) {
        int length = flat.Bytes(type, *s, 0);
        result = Type::Array(type->Base(), length);
    } else if (!init.IsList()) {
        throw std::runtime_error("array initialiser must be a brace list");
    } else {
        size_t pos = 0;
        int count = flat.Array(type, init.Items(), pos, true, 0);
        if (type->Length() == 0) result = Type::Array(type->Base(), count);
    }
    return FlatInit{result, flat.TakeItems()};
}

}  // namespace ast
