#include "global_init.hpp"

#include <cstdint>
#include <sstream>
#include <stdexcept>

namespace ast {

namespace {

// Writes the directives for one object, walking the initialiser tree. This
// is the one place that knows how a type's bytes are laid out in .data;
// floating types (feature 7) and records (feature 8) become new cases of
// Scalar and Object.
class InitWriter {
public:
    explicit InitWriter(const Context& ctx) : ctx_(ctx) {}
    std::string Text() const { return out_.str(); }

    // One scalar from a bare expression or a braced single value.
    void Scalar(const TypePtr& type, const Initializer& item) {
        if (item.IsList()) {
            const Initializer::List& inner = item.Items();
            if (inner.size() != 1 || inner[0]->IsList()) {
                throw std::runtime_error("invalid scalar initialiser");
            }
            return Scalar(type, *inner[0]);
        }
        if (!type->IsScalar()) {
            throw std::runtime_error("unsupported initialiser type");
        }
        std::optional<ConstValue> value = item.Value().EvalConstValue(ctx_);
        if (!value) {
            throw std::runtime_error("initialiser is not a constant");
        }
        Word(type->Size(), *value);
    }

    // The object that starts at list[pos]; advances pos past what it used.
    void Object(const TypePtr& type, const Initializer::List& list,
                size_t& pos) {
        const Initializer& item = *list[pos];
        if (!type->IsArray()) {
            Scalar(type, item);
            ++pos;
        } else if (item.IsList()) {
            size_t inner_pos = 0;
            Array(type, item.Items(), inner_pos, true);
            ++pos;
        } else {
            Array(type, list, pos, false);  // brace elision: {1, 2, 3, 4}
        }
    }

    // Fills an array from list[pos...] and pads with zeros. 'braced' means
    // the list belongs to this array, so leftovers are an error. A length of
    // 0 is unspecified: all elements are taken. Returns the element count.
    int Array(const TypePtr& type, const Initializer::List& list, size_t& pos,
              bool braced) {
        int length = type->Length();
        int count = 0;
        while (pos < list.size() && (length == 0 || count < length)) {
            Object(type->Base(), list, pos);
            ++count;
        }
        if (length == 0 && count == 0) {
            throw std::runtime_error("empty initialiser for unsized array");
        }
        if (braced && pos < list.size()) {
            throw std::runtime_error("excess elements in array initialiser");
        }
        if (length > count) Zero((length - count) * type->Base()->Size());
        return count;
    }

private:
    void Zero(int bytes) {
        if (bytes > 0) out_ << "\t.zero " << bytes << "\n";
    }

    // One integer or address of 'size' bytes.
    void Word(int size, const ConstValue& value) {
        if (!value.label.empty()) {
            if (size != 4) throw std::runtime_error("address does not fit");
            long magnitude = value.offset < 0 ? -value.offset : value.offset;
            out_ << "\t.word " << value.label << (value.offset < 0 ? "-" : "+")
                 << magnitude << "\n";
            return;
        }
        uint32_t bits = static_cast<uint32_t>(value.offset);
        if (size == 1) out_ << "\t.byte " << (bits & 0xff) << "\n";
        if (size == 2) out_ << "\t.half " << (bits & 0xffff) << "\n";
        if (size == 4) out_ << "\t.word " << bits << "\n";
    }

    const Context& ctx_;
    std::ostringstream out_;
};

}  // namespace

GlobalInit BuildGlobalInit(const TypePtr& type, const Initializer& init,
                           const Context& ctx) {
    InitWriter writer(ctx);
    TypePtr result = type;
    if (type->IsArray()) {
        if (!init.IsList()) {
            throw std::runtime_error("array initialiser must be a brace list");
        }
        size_t pos = 0;
        int count = writer.Array(type, init.Items(), pos, true);
        if (type->Length() == 0) result = Type::Array(type->Base(), count);
    } else {
        writer.Scalar(type, init);
    }
    return GlobalInit{result, writer.Text()};
}

}  // namespace ast
