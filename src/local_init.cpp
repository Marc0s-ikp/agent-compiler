#include "local_init.hpp"

#include "emit.hpp"

namespace ast {

namespace {

void EmitScalarItem(std::ostream& os, Context& ctx, const InitItem& item,
                    int offset) {
    item.value->EmitRISC(os, ctx);
    EmitConvert(os, *item.value->ValueType(ctx), *item.type);
    EmitStore(os, *item.type, AccReg(*item.type), "s0", offset + item.offset);
}

// A string's chars, one byte store each. The array was zeroed first, so zero
// bytes (and the NUL) need no store.
void EmitStringItem(std::ostream& os, const InitItem& item, int offset) {
    TypePtr byte = Type::Integer(1, false);
    for (size_t i = 0; i < item.bytes.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(item.bytes[i]);
        if (c == 0) continue;
        os << "\tli t0, " << static_cast<int>(c) << "\n";
        EmitStore(os, *byte, "t0", "s0", offset + item.offset + static_cast<int>(i));
    }
}

}  // namespace

void EmitLocalInit(std::ostream& os, Context& ctx, const FlatInit& flat,
                   int offset) {
    if (flat.type->IsArray()) EmitZeroMem(os, "s0", offset, flat.type->Size());
    for (const InitItem& item : flat.items) {
        if (item.value) {
            EmitScalarItem(os, ctx, item, offset);
        } else {
            EmitStringItem(os, item, offset);
        }
    }
}

}  // namespace ast
