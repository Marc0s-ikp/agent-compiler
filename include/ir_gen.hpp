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
    ir::Operand AddFrameObject(const std::string& name, const Type& type);

    // ---- instructions ----
    ir::Operand Binary(ir::Op op, ir::IrType type, ir::Operand a, ir::Operand b);
    ir::Operand Compare(ir::Op op, ir::IrType type, ir::Operand a, ir::Operand b);
    ir::Operand Unary(ir::Op op, ir::IrType type, ir::Operand a);
    // A temporary with no instruction yet: the result of '&&', '||' and '?:'
    // gets its value from Move at two sites (21.2).
    ir::Operand NewTemp(ir::IrType type);
    void Move(ir::Operand dst, ir::Operand src);  // 'const' for a constant, else 'copy'
    ir::Operand AddressOf(ir::Operand object_or_symbol, long offset = 0);
    ir::Operand Load(ir::IrType type, ir::Operand address);
    void Store(ir::IrType type, ir::Operand value, ir::Operand address);
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
