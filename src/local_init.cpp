#include "local_init.hpp"

#include <stdexcept>

#include "ast_lower.hpp"
#include "emit.hpp"
#include "target.hpp"

namespace ast {

namespace {

// A struct-valued expression is the address of the source; copy it in.
void EmitRecordItem(std::ostream& os, Context& ctx, const InitItem& item,
                    int offset) {
    item.value->EmitRISC(os, ctx);
    EmitAddrOffset(os, "t1", "s0", offset + item.offset);
    EmitCopyMem(os, "t1", "a0", item.type->Size(), item.type->Align());
}

void EmitScalarItem(std::ostream& os, Context& ctx, const InitItem& item,
                    int offset) {
    if (item.type->IsRecord()) return EmitRecordItem(os, ctx, item, offset);
    item.value->EmitRISC(os, ctx);
    EmitConvert(os, *item.value->ValueType(ctx), *item.type);
    EmitStore(os, *item.type, AccReg(*item.type), "s0", offset + item.offset);
}

// A string's chars, one byte store each. The array was zeroed first, so zero
// bytes (and the NUL) need no store.
void EmitStringItem(std::ostream& os, const InitItem& item, int offset) {
    TypePtr byte = Type::Integer(Target().char_size, false);
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
    if (flat.type->IsArray() || flat.type->IsRecord()) EmitZeroMem(os, "s0", offset, flat.type->Size());
    for (const InitItem& item : flat.items) {
        if (item.value) {
            EmitScalarItem(os, ctx, item, offset);
        } else {
            EmitStringItem(os, item, offset);
        }
    }
}

// ---- --ir lowering ----

namespace {

void StoreItemIR(IRGen& gen, const InitItem& item, const ir::Operand& object) {
    ir::Operand address = gen.AddressOf(object, item.offset);
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

void EmitLocalInitIR(IRGen& gen, const FlatInit& flat, const ir::Operand& object) {
    if (flat.type->IsRecord()) throw std::runtime_error("not in IR yet: struct initialiser");
    if (flat.type->IsArray()) gen.MemZero(gen.AddressOf(object), flat.type->Size(), flat.type->Align());
    for (const InitItem& item : flat.items) StoreItemIR(gen, item, object);
}

}  // namespace ast
