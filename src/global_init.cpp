#include "global_init.hpp"

#include <cstdint>
#include <cstring>
#include <sstream>
#include <stdexcept>

#include "const_value.hpp"
#include "escapes.hpp"
#include "init_layout.hpp"

namespace ast {

namespace {

// Writes the directives for one laid-out object. This is the one place that
// knows how values are written into .data.
class InitWriter {
public:
    explicit InitWriter(Context& ctx) : ctx_(ctx) {}
    std::string Text() const { return out_.str(); }

    // Writes the whole object of 'size' bytes: items in order, zeros between.
    void Object(const FlatInit& flat) {
        int cursor = 0;
        for (const InitItem& item : flat.items) {
            Zero(item.offset - cursor);
            if (item.value) {
                Scalar(*item.type, *item.value);
            } else {
                Bytes(item.type->Size(), item.bytes);
            }
            cursor = item.offset + item.type->Size();
        }
        Zero(flat.type->Size() - cursor);
    }

private:
    // The value is converted to the object's type, as an assignment would.
    void Scalar(const Type& type, const Expr& expr) {
        std::optional<ConstValue> value = expr.EvalConstValue(ctx_);
        if (!value) throw std::runtime_error("initialiser is not a constant");
        std::optional<ConstValue> converted = ConvertConst(*value, type);
        if (!converted) throw std::runtime_error("initialiser is not a constant");
        if (type.IsFloating()) {
            Real(type.Size(), converted->real);
        } else {
            Word(type.Size(), *converted);
        }
    }

    // A float is one word of IEEE bits, a double two (low word first).
    void Real(int size, double value) {
        uint64_t bits;
        if (size == 4) {
            float narrow = static_cast<float>(value);
            uint32_t word;
            std::memcpy(&word, &narrow, sizeof word);
            bits = word;
        } else {
            std::memcpy(&bits, &value, sizeof bits);
        }
        out_ << "\t.word " << (bits & 0xffffffffu) << "\n";
        if (size == 8) out_ << "\t.word " << (bits >> 32) << "\n";
    }

    // The chars of a string initialiser, then zeros up to 'size' bytes.
    void Bytes(int size, const std::string& bytes) {
        if (!bytes.empty()) out_ << "\t.ascii " << AsmQuoted(bytes) << "\n";
        Zero(size - static_cast<int>(bytes.size()));
    }

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
        else if (size == 2) out_ << "\t.half " << (bits & 0xffff) << "\n";
        else if (size == 4) out_ << "\t.word " << bits << "\n";
        else throw std::runtime_error("unsupported initialiser size");
    }

    Context& ctx_;
    std::ostringstream out_;
};

}  // namespace

GlobalInit BuildGlobalInit(const TypePtr& type, const Initializer& init,
                           Context& ctx) {
    FlatInit flat = FlattenInit(type, init);
    InitWriter writer(ctx);
    writer.Object(flat);
    return GlobalInit{flat.type, writer.Text()};
}

}  // namespace ast
