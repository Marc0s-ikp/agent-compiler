#pragma once
#include <ostream>
#include <set>
#include <string>
#include <vector>

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
    // compare: the operands have 'type', the result is I32 0 or 1
    Eq, Ne, Lt, LtU, Le, LeU, FEq, FLt, FLe,
    // conversion: 'type' is the result type, the source is the operand's
    SExt, ZExt, Trunc, IToF, UToF, FToI, FToU, FExt, FTrunc,
    // memory
    Load, Store, MemCpy, MemZero,
    // control
    Label, Jump, Branch, Ret,
};

const char* OpName(Op op);
bool IsBinaryArith(Op op);  // Add to Sar
bool IsCompare(Op op);      // Eq to FLe
bool IsConversion(Op op);   // SExt to FTrunc

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
};

struct FrameObject {
    std::string name;
    int size, align;
};

struct Function {
    std::string name;
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
    int size = 0;        // Int, Float; Zero: byte count; Address: offset
    long long value = 0; // Int value, Float bits
};

struct DataEntry {
    std::string label;
    int size = 0, align = 1;
    bool read_only = false;
    bool internal = false;
    std::vector<DataItem> items;  // empty and not read-only: zero-filled
};

struct Module {
    std::vector<Function> functions;
    std::vector<DataEntry> data;
    std::set<std::string> externals;  // functions defined elsewhere
};

// --emit-ir: the module as text.
void Print(const Module& module, std::ostream& os);

// The IR checker (21.2). Returns one message per problem; empty means the
// module is well formed. Runs on every --ir and --emit-ir compile.
std::vector<std::string> CheckIR(const Module& module);

}  // namespace ir
