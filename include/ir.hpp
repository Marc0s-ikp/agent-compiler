#pragma once
#include <ostream>
#include <set>
#include <string>
#include <vector>

#include "type.hpp"

// The linear IR between the AST and code generation (D21, 21.2). Nothing in
// this file knows about C types or about any machine, apart from the size of
// a Ptr, which comes from Target().
namespace ir {

// Machine-level types. Signedness belongs to the operation, not the type.
enum class IrType { I8, I16, I32, I64, F32, F64, Ptr };

int SizeOf(IrType type);  // bytes; Ptr is Target().pointer_size
bool IsIntType(IrType type);    // I8 to I64
bool IsFloatType(IrType type);  // F32, F64
const char* TypeName(IrType type);
// The IR type of a scalar C type (integer or pointer); throws "not in IR yet"
// for the others. The one place C types meet IR types (21.5).
IrType IrTypeOf(const Type& type);
// The integer type as wide as a pointer (I32 on RV32): the offset of ptradd
// and the result of ptrtoint (21.10).
IrType PointerIntType();

// An instruction argument. A temporary is typed and numbered per function; a
// frame object ('$name') and a symbol ('@name') are both addresses (Ptr).
struct Operand {
    enum class Kind { None, Temp, Int, Float, Object, Symbol };
    Kind kind = Kind::None;
    IrType type = IrType::I32;
    int temp = 0;        // Temp
    long int_value = 0;  // Int
    double float_value = 0;  // Float
    std::string name;    // Object, Symbol

    static Operand Temp(int id, IrType type);
    static Operand Int(long value, IrType type);
    static Operand Float(double value, IrType type);
    static Operand Object(const std::string& name);
    static Operand Symbol(const std::string& name);
    bool IsTemp() const { return kind == Kind::Temp; }
    bool IsConst() const { return kind == Kind::Int || kind == Kind::Float; }
};

enum class Op {
    // values
    Copy, Const, Addr,
    // arithmetic
    Add, Sub, Mul, Div, DivU, Rem, RemU, And, Or, Xor, Shl, Shr, Sar, Neg, Not,
    // ptradd ptr, offset: Ptr + a byte count of PointerIntType() -> Ptr (21.10)
    PtrAdd,
    // compare: the operands have 'type', the result is I32 0 or 1
    Eq, Ne, Lt, LtU, Le, LeU, FEq, FLt, FLe,
    // conversion: 'type' is the result type, the source is the operand's
    SExt, ZExt, Trunc, IToF, UToF, FToI, FToU, FExt, FTrunc,
    // Ptr <-> PointerIntType()
    PtrToInt, IntToPtr,
    // memory
    Load, Store, MemCpy, MemZero,
    // control
    Label, Jump, Branch, Ret,
    // call: 'callee', args = the values, 'arg_types' their C types, 'ret_type'
    // the C return type; the result (if not void) is 'dst' with 'type'
    Call,
};

const char* OpName(Op op);
bool IsBinaryArith(Op op);  // Add to Sar
bool IsCompare(Op op);      // Eq to FLe
bool IsConversion(Op op);   // SExt to IntToPtr

// One instruction. 'type' is the type the operation works on: the operands'
// and the result's for arithmetic, the operands' for a compare, the result's
// for a conversion and a load, the stored value's for a store.
struct Instruction {
    Op op = Op::Ret;
    IrType type = IrType::I32;
    Operand dst;                // a Temp, or None
    std::vector<Operand> args;
    std::string label;          // Label; Jump target; Branch if nonzero
    std::string label2;         // Branch if zero
    long offset = 0;            // Addr: byte offset
    long size = 0, align = 0;   // MemCpy, MemZero
    // Call (21.5): the C types are what the target's classifier needs
    std::string callee;
    std::vector<TypePtr> arg_types;
    TypePtr ret_type;
};

struct FrameObject {
    std::string name;
    int size, align;
};

// A parameter: the frame object that is its home, and its C type. The backend
// fills the home in the prologue, from the classifier's pieces (21.5).
struct Param {
    std::string object;
    TypePtr type;
};

struct Function {
    std::string name;
    std::vector<Param> params;
    TypePtr c_return;           // the C return type (void included)
    bool is_global = true;
    bool returns_value = false;
    IrType return_type = IrType::I32;
    std::vector<FrameObject> objects;
    std::vector<IrType> temps;  // the type of %0, %1, ...
    std::vector<Instruction> body;
};

// A global, string or float-pool object that the backend writes (21.2a).
struct DataItem {
    enum class Kind { Bytes, Int, Float, Address, Zero };
    Kind kind = Kind::Zero;
    std::string bytes;   // Bytes; Address: the label
    int size = 0;        // Int, Float: 1, 2, 4 or 8; Zero: byte count
    long offset = 0;     // Address: byte offset from the label
    long long value = 0; // Int value, Float bits
};

struct DataEntry {
    std::string label;
    int size = 0, align = 1;
    bool read_only = false;
    bool internal = false;       // no .globl: 'static', or compiler-made
    // Made by the compiler (string, float constant, static local). The name is
    // neutral and contains a '.', which no C name does; the backend decides how
    // it is spelled (RISC-V: a local ".L" label). Implies 'internal'.
    bool compiler_made = false;
    std::vector<DataItem> items;  // empty and not read-only: zero-filled
};

struct Module {
    std::vector<Function> functions;
    std::vector<DataEntry> data;
    std::set<std::string> externals;  // functions defined elsewhere
    std::set<std::string> external_data;  // extern objects: valid symbols, no entry
};

// --emit-ir: the module as text.
void Print(const Module& module, std::ostream& os);

// The IR checker (21.2). Returns one message per problem; empty means the
// module is well formed. Runs on every --ir and --emit-ir compile.
std::vector<std::string> CheckIR(const Module& module);

}  // namespace ir
