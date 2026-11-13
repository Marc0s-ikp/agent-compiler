#include "global_init.hpp"

#include <cstdint>
#include <cstring>
#include <stdexcept>

#include "const_value.hpp"
#include "init_layout.hpp"
#include "target.hpp"

namespace ast {

namespace {

// Builds the data items for one laid-out object. This is the one place that
// knows how values are written into .data (the backend spells them).
class InitWriter {
public:
    explicit InitWriter(Context& ctx) : ctx_(ctx) {}
    const std::vector<ir::DataItem>& Items() const { return items_; }

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
        items_.push_back(Item(ir::DataItem::Kind::Float, size, static_cast<long long>(bits)));
    }

    // The chars of a string initialiser, then zeros up to 'size' bytes.
    void Bytes(int size, const std::string& bytes) {
        if (!bytes.empty()) {
            ir::DataItem item = Item(ir::DataItem::Kind::Bytes, 0, 0);
            item.bytes = bytes;
            items_.push_back(item);
        }
        Zero(size - static_cast<int>(bytes.size()));
    }

    void Zero(int bytes) {
        if (bytes <= 0) return;
        items_.push_back(Item(ir::DataItem::Kind::Zero, bytes, 0));
    }

    // One integer or address of 'size' bytes. An address is pointer-wide.
    void Word(int size, const ConstValue& value) {
        if (!value.label.empty()) {
            if (size != Target().pointer_size) throw std::runtime_error("address does not fit");
            ir::DataItem item = Item(ir::DataItem::Kind::Address, size, 0);
            item.bytes = value.label;
            item.offset = value.offset;
            items_.push_back(item);
            return;
        }
        if (size != 1 && size != 2 && size != 4 && size != 8) throw std::runtime_error("unsupported initialiser size");
        long long bits = value.offset;
        long long kept = size == 8 ? bits : bits & ((1LL << (8 * size)) - 1);  // the low 'size' bytes
        items_.push_back(Item(ir::DataItem::Kind::Int, size, kept));
    }

    static ir::DataItem Item(ir::DataItem::Kind kind, int size, long long value) {
        ir::DataItem item;
        item.kind = kind;
        item.size = size;
        item.value = value;
        return item;
    }

    Context& ctx_;
    std::vector<ir::DataItem> items_;
};

}  // namespace

GlobalInit BuildGlobalInit(const TypePtr& type, const Initializer& init,
                           Context& ctx) {
    FlatInit flat = FlattenInit(type, init, ctx);
    InitWriter writer(ctx);
    writer.Object(flat);
    return GlobalInit{flat.type, writer.Items()};
}

}  // namespace ast
