#pragma once
#include <string>

#include "context.hpp"
#include "ir.hpp"
#include "type.hpp"

// The frontend's handle on the IR being built (D21, 21.1). AST nodes call
// these helpers from EmitIR and never build ir::Instruction by hand. It knows
// C types only at the edges (ToIr, Convert); the instructions it makes do not.
class IRGen {
public:
    IRGen(ir::Module& module, Context& ctx) : module_(module), ctx_(ctx) {}

    Context& Ctx() { return ctx_; }
    ir::Module& Module() { return module_; }

    // ---- functions ----
    void BeginFunction(const std::string& name, bool is_global, const TypePtr& return_type);
    void EndFunction();  // adds an implicit return, then appends the function
    // After a lowering error: appends the function being built as it stands,
    // so --emit-ir can dump it (D23 c). Does nothing between functions.
    void AppendPartialFunction();

    // A frame object for a local. The returned operand names it uniquely
    // ('x' and a shadowing 'x' become $x.0 and $x.1).
    // The home of the next parameter, a frame object; the backend fills it
    // from the incoming registers or stack (21.5).
    ir::Operand AddParam(const std::string& name, const TypePtr& type);
    ir::Operand AddFrameObject(const std::string& name, const Type& type);

    // ---- instructions ----
    ir::Operand Binary(ir::Op op, ir::IrType type, ir::Operand a, ir::Operand b);
    ir::Operand Compare(ir::Op op, ir::IrType type, ir::Operand a, ir::Operand b);
    ir::Operand Unary(ir::Op op, ir::IrType type, ir::Operand a);
    ir::Operand AddressOf(ir::Operand object_or_symbol, long offset = 0);
    // Pointer arithmetic (21.10). 'offset' is a byte count of
    // ir::PointerIntType(); the frontend scales and widens it first.
    ir::Operand PtrAdd(ir::Operand pointer, ir::Operand offset);
    ir::Operand PtrToInt(ir::Operand pointer);
    ir::Operand IntToPtr(ir::Operand integer);
    // A conversion instruction (sext, itof, ftoi, fext, ...) to 'result'.
    ir::Operand Convert(ir::Op op, ir::IrType result, ir::Operand value);
    // A floating constant: an entry in the float pool, loaded from memory
    // (21.2a). 'type' is F32 or F64.
    ir::Operand FloatConstant(double value, ir::IrType type);
    ir::Operand Load(ir::IrType type, ir::Operand address);
    void Store(ir::IrType type, ir::Operand value, ir::Operand address);
    // Zeroes 'size' bytes at 'address', which is aligned to 'align'.
    void MemZero(ir::Operand address, long size, long align);
    // A call; args are already converted to arg_types. Returns the result, or
    // an operand of kind None for a void callee.
    ir::Operand Call(const std::string& callee, const std::vector<ir::Operand>& args,
                     const std::vector<TypePtr>& arg_types, const TypePtr& ret_type);
    // After the last function: a callee defined in this module is not external.
    void EndModule();
    void Return();                      // 'return;'
    void Return(ir::Operand value);

    // ---- labels and jumps ----
    std::string NewLabel(const std::string& prefix) { return ctx_.NewLabel(prefix); }
    void Label(const std::string& label);
    void Jump(const std::string& label);
    void Branch(ir::Operand condition, const std::string& if_nonzero,
                const std::string& if_zero);

    // ---- C types at the edge ----
    // The IR type of a scalar C type; throws "not in IR yet" for the others.
    static ir::IrType ToIr(const Type& type);
    // Converts an integer value between two C integer types.
    ir::Operand ConvertInt(ir::Operand value, const Type& from, const Type& to);

private:
    ir::Operand Emit(ir::Instruction in, ir::IrType result);
    void Append(const ir::Instruction& in);

    ir::Module& module_;
    Context& ctx_;
    ir::Function fn_;
    int object_count_ = 0;
};
