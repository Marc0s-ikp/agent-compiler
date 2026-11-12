#include "local_init.hpp"

#include <stdexcept>

#include "ast_lower.hpp"
#include "target.hpp"

namespace ast {

namespace {

void StoreItem(IRGen& gen, const InitItem& item, const ir::Operand& object) {
    ir::Operand address = gen.AddressOf(object, item.offset);
    if (item.value && item.type->IsRecord()) {  // a whole struct from an expression
        gen.MemCpy(address, ValueAs(gen, *item.value, *item.type), item.type->Size(), item.type->Align());
        return;
    }
    if (item.value) {
        RequireIrScalar(*item.type);
        gen.Store(IRGen::ToIr(*item.type), ValueAs(gen, *item.value, *item.type), address);
        return;
    }
    ir::IrType byte = ir::IrType::I8;  // a char array's string bytes
    for (size_t i = 0; i < item.bytes.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(item.bytes[i]);
        if (c == 0) continue;  // already zero
        gen.Store(byte, ir::Operand::Int(c, byte), gen.AddressOf(object, item.offset + static_cast<long>(i)));
    }
}

}  // namespace

void EmitLocalInit(IRGen& gen, const FlatInit& flat, const ir::Operand& object) {
    if (flat.type->IsArray() || flat.type->IsRecord()) gen.MemZero(gen.AddressOf(object), flat.type->Size(), flat.type->Align());
    for (const InitItem& item : flat.items) StoreItem(gen, item, object);
}

}  // namespace ast
