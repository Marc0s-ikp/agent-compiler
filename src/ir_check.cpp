#include <map>
#include <set>

#include "ir.hpp"
#include "target.hpp"

// The IR checker (D21, 21.2). It looks at one function at a time, walking the
// instruction list in order, and reports every problem it finds.
namespace ir {

namespace {

bool IsIntOrPtr(IrType t) { return IsIntType(t) || t == IrType::Ptr; }

class FunctionChecker {
public:
    FunctionChecker(const Module& module, const Function& fn,
                    std::vector<std::string>& errors)
        : module_(module), fn_(fn), errors_(errors), defined_(fn.temps.size(), false) {}

    void Run() {
        CheckParams();
        for (index_ = 0; index_ < static_cast<int>(fn_.body.size()); ++index_) {
            CheckInstruction(fn_.body[index_]);
        }
        CheckJumpTargets();
    }

private:
    void Error(const std::string& what) {
        errors_.push_back("function '" + fn_.name + "', instruction " +
                          std::to_string(index_) + " (" + OpName(fn_.body[index_].op) +
                          "): " + what);
    }

    // ---- operands ----

    void CheckTempUse(const Operand& o) {
        if (o.temp < 0 || o.temp >= static_cast<int>(fn_.temps.size())) {
            return Error("unknown temporary %" + std::to_string(o.temp));
        }
        if (!defined_[o.temp]) Error("%" + std::to_string(o.temp) + " used before it is defined");
        if (fn_.temps[o.temp] != o.type) {
            Error("%" + std::to_string(o.temp) + " used as " + TypeName(o.type) +
                  " but defined as " + TypeName(fn_.temps[o.temp]));
        }
    }

    bool SymbolExists(const std::string& name) const {
        if (module_.external_data.count(name) > 0) return true;
        for (const DataEntry& d : module_.data) if (d.label == name) return true;
        for (const Function& f : module_.functions) if (f.name == name) return true;
        return module_.externals.count(name) > 0;
    }

    void CheckUse(const Operand& o) {
        switch (o.kind) {
            case Operand::Kind::Temp: return CheckTempUse(o);
            case Operand::Kind::Object: {
                for (const FrameObject& f : fn_.objects) if (f.name == o.name) return;
                return Error("unknown frame object $" + o.name);
            }
            case Operand::Kind::Symbol:
                if (!SymbolExists(o.name)) Error("unknown symbol @" + o.name);
                return;
            case Operand::Kind::None: return Error("missing operand");
            case Operand::Kind::Int:
                // A null pointer is 'inttoptr' of an integer, not a Ptr constant.
                if (!IsIntType(o.type)) Error("integer constant with type " + std::string(TypeName(o.type)));
                return;
            case Operand::Kind::Float:
                if (!IsFloatType(o.type)) Error("floating constant with type " + std::string(TypeName(o.type)));
                return;
        }
    }

    // Each parameter's home must be one of the function's frame objects.
    void CheckParams() {
        for (const Param& p : fn_.params) {
            bool found = false;
            for (const FrameObject& o : fn_.objects) found = found || o.name == p.object;
            if (!found) {
                errors_.push_back("function '" + fn_.name + "': parameter home $" + p.object +
                                  " is not a frame object");
            }
        }
    }

    bool HasArgs(const Instruction& in, size_t n) {
        if (in.args.size() == n) return true;
        Error("expected " + std::to_string(n) + " operands, found " + std::to_string(in.args.size()));
        return false;
    }

    void RequireType(const Operand& o, IrType want, const char* role) {
        if (o.type != want) {
            Error(std::string(role) + " has type " + TypeName(o.type) + ", expected " + TypeName(want));
        }
    }

    // ---- the result ----

    void Define(const Instruction& in, IrType result) {
        if (in.dst.kind != Operand::Kind::Temp) return Error("missing result temporary");
        int id = in.dst.temp;
        if (id < 0 || id >= static_cast<int>(fn_.temps.size())) {
            return Error("unknown result temporary %" + std::to_string(id));
        }
        if (fn_.temps[id] != result || in.dst.type != result) {
            Error("%" + std::to_string(id) + " is " + TypeName(fn_.temps[id]) +
                  " but the instruction gives " + TypeName(result));
        }
        if (defined_[id]) Error("%" + std::to_string(id) + " is defined at a second site");
        defined_[id] = true;
    }

    // ---- per kind ----

    void CheckArith(const Instruction& in) {
        if (!HasArgs(in, 2)) return;
        bool real_ok = in.op == Op::Add || in.op == Op::Sub || in.op == Op::Mul || in.op == Op::Div;
        if (!IsIntType(in.type) && !(real_ok && IsFloatType(in.type))) Error("operand type not allowed");
        RequireType(in.args[0], in.type, "left operand");
        RequireType(in.args[1], in.type, "right operand");
    }

    void CheckCompare(const Instruction& in) {
        if (!HasArgs(in, 2)) return;
        bool real = in.op >= Op::FEq;
        if (real ? !IsFloatType(in.type) : !IsIntOrPtr(in.type)) Error("operand type not allowed");
        RequireType(in.args[0], in.type, "left operand");
        RequireType(in.args[1], in.type, "right operand");
        // Pointers compare for equality or as unsigned numbers, never signed.
        if (in.type == IrType::Ptr && (in.op == Op::Lt || in.op == Op::Le)) {
            Error("pointers are compared with eq, ne, ltu or leu");
        }
    }

    // Each conversion: the source kind, the result kind, and whether the
    // result is wider (+1), narrower (-1) or any (0).
    void CheckConversion(const Instruction& in) {
        if (!HasArgs(in, 1)) return;
        IrType from = in.args[0].type, to = in.type;
        bool ok = false;
        switch (in.op) {
            case Op::SExt:
            case Op::ZExt: ok = IsIntType(from) && IsIntType(to) && SizeOf(to) > SizeOf(from); break;
            case Op::Trunc: ok = IsIntType(from) && IsIntType(to) && SizeOf(to) < SizeOf(from); break;
            case Op::IToF:
            case Op::UToF: ok = IsIntType(from) && IsFloatType(to); break;
            case Op::FToI:
            case Op::FToU: ok = IsFloatType(from) && IsIntType(to); break;
            case Op::FExt: ok = from == IrType::F32 && to == IrType::F64; break;
            case Op::FTrunc: ok = from == IrType::F64 && to == IrType::F32; break;
            case Op::PtrToInt: ok = from == IrType::Ptr && to == PointerIntType(); break;
            case Op::IntToPtr: ok = from == PointerIntType() && to == IrType::Ptr; break;
            default: break;
        }
        if (!ok) Error(std::string("cannot convert ") + TypeName(from) + " to " + TypeName(to));
    }

    void CheckRet(const Instruction& in) {
        if (in.args.empty()) {
            if (fn_.returns_value) Error("return without a value in a function that returns one");
            return;
        }
        if (!HasArgs(in, 1)) return;
        if (!fn_.returns_value) return Error("return with a value in a void function");
        RequireType(in.args[0], fn_.return_type, "returned value");
        RequireType(in.args[0], in.type, "returned value");
    }

    void CheckInstruction(const Instruction& in) {
        for (const Operand& a : in.args) CheckUse(a);
        bool has_result = true;
        IrType result = in.type;
        if (IsBinaryArith(in.op)) {
            CheckArith(in);
        } else if (IsCompare(in.op)) {
            CheckCompare(in);
            result = IrType::I32;
        } else if (IsConversion(in.op)) {
            CheckConversion(in);
        } else {
            has_result = CheckOther(in, result);
        }
        if (has_result) {
            Define(in, result);
        } else if (in.dst.kind != Operand::Kind::None) {
            Error("this instruction has no result");
        }
    }

    // Everything that is not arithmetic, a compare or a conversion. Returns
    // whether the instruction defines a temporary.
    bool CheckOther(const Instruction& in, IrType& result) {
        switch (in.op) {
            case Op::Copy:
                if (HasArgs(in, 1)) RequireType(in.args[0], in.type, "operand");
                return true;
            case Op::Const:
                if (HasArgs(in, 1)) {
                    if (!in.args[0].IsConst()) Error("operand must be a constant");
                    RequireType(in.args[0], in.type, "constant");
                }
                return true;
            case Op::PtrAdd:  // the one arithmetic on a Ptr (21.10)
                result = IrType::Ptr;
                if (in.type != IrType::Ptr) Error("ptradd works on ptr");
                if (HasArgs(in, 2)) {
                    RequireType(in.args[0], IrType::Ptr, "pointer");
                    RequireType(in.args[1], PointerIntType(), "offset");
                }
                return true;
            case Op::Neg:  // integers and floats; not Ptr
                if (HasArgs(in, 1)) RequireType(in.args[0], in.type, "operand");
                if (!IsIntType(in.type) && !IsFloatType(in.type)) Error("operand type not allowed");
                return true;
            case Op::Not:
                if (HasArgs(in, 1)) RequireType(in.args[0], in.type, "operand");
                if (!IsIntType(in.type)) Error("operand type not allowed");
                return true;
            case Op::Addr:
                result = IrType::Ptr;
                if (HasArgs(in, 1) && in.args[0].kind != Operand::Kind::Object &&
                    in.args[0].kind != Operand::Kind::Symbol) {
                    Error("operand must be a frame object or a symbol");
                }
                return true;
            case Op::Load:
                if (HasArgs(in, 1)) RequireType(in.args[0], IrType::Ptr, "address");
                return true;
            case Op::Store:
                if (HasArgs(in, 2)) {
                    RequireType(in.args[0], in.type, "stored value");
                    RequireType(in.args[1], IrType::Ptr, "address");
                }
                return false;
            case Op::MemCpy:
            case Op::MemZero: {
                size_t n = in.op == Op::MemCpy ? 2 : 1;
                if (HasArgs(in, n)) for (const Operand& a : in.args) RequireType(a, IrType::Ptr, "address");
                return false;
            }
            case Op::Label:
                if (!in.label.empty() && in.label[0] == '.') Error("label " + in.label + " is assembler syntax");
                if (!labels_.insert(in.label).second) Error("label " + in.label + " defined twice");
                return false;
            case Op::Jump:
                jumps_.push_back({in.label, index_});
                return false;
            case Op::Branch:
                if (HasArgs(in, 1) && !IsIntType(in.args[0].type)) Error("branch condition must be an integer");
                jumps_.push_back({in.label, index_});
                jumps_.push_back({in.label2, index_});
                return false;
            case Op::Ret:
                CheckRet(in);
                return false;
            case Op::Call: return CheckCall(in, result);
            default: return false;
        }
    }

    bool IsFunction(const std::string& name) const {
        for (const Function& f : module_.functions) if (f.name == name) return true;
        return module_.externals.count(name) > 0;
    }

    // The callee is a function of the module or an external; each argument has
    // the IR type of its C type; the result has the IR type of the return type.
    bool CheckCall(const Instruction& in, IrType& result) {
        if (!IsFunction(in.callee)) Error("unknown function " + in.callee);
        if (!in.ret_type) {
            Error("missing return type");
            return false;
        }
        if (in.args.size() != in.arg_types.size()) {
            Error("expected " + std::to_string(in.arg_types.size()) + " arguments, found " +
                  std::to_string(in.args.size()));
        } else {
            for (size_t i = 0; i < in.args.size(); ++i) {
                RequireType(in.args[i], IrTypeOf(*in.arg_types[i]), "argument");
            }
        }
        if (in.ret_type->IsVoid()) return false;
        result = IrTypeOf(*in.ret_type);
        if (in.type != result) {
            Error(std::string("result has type ") + TypeName(in.type) + ", expected " + TypeName(result));
        }
        return true;
    }

    void CheckJumpTargets() {
        for (const auto& [label, at] : jumps_) {
            if (!labels_.count(label)) {
                index_ = at;
                Error("label " + label + " is never defined");
            }
        }
    }

    const Module& module_;
    const Function& fn_;
    std::vector<std::string>& errors_;
    std::vector<bool> defined_;
    std::set<std::string> labels_;
    std::vector<std::pair<std::string, int>> jumps_;
    int index_ = 0;
};

}  // namespace

// Data entries: one entry per label, and a compiler-made one is internal and
// has a name no C identifier can have (so it cannot clash with a global).
void CheckData(const Module& module, std::vector<std::string>& errors) {
    std::set<std::string> seen;
    for (const DataEntry& d : module.data) {
        std::string who = "data '" + d.label + "': ";
        if (!seen.insert(d.label).second) errors.push_back(who + "defined twice");
        bool dotted = d.label.find('.') != std::string::npos;
        if (d.compiler_made && !d.internal) errors.push_back(who + "compiler-made but not internal");
        if (d.compiler_made && !dotted) errors.push_back(who + "compiler-made name must contain '.'");
        for (const DataItem& item : d.items) {
            if (item.kind == DataItem::Kind::Float && item.size != 4 && item.size != 8) {
                errors.push_back(who + "float item must be 4 or 8 bytes");
            }
            if (item.kind != DataItem::Kind::Address) continue;
            bool known = module.external_data.count(item.bytes) > 0 || module.externals.count(item.bytes) > 0;
            for (const DataEntry& e : module.data) known = known || e.label == item.bytes;
            for (const Function& f : module.functions) known = known || f.name == item.bytes;
            if (!known) errors.push_back(who + "address of unknown symbol " + item.bytes);
            if (item.size != Target().pointer_size) errors.push_back(who + "address item is not pointer-sized");
        }
        if (!d.compiler_made && dotted) errors.push_back(who + "a C-named entry cannot contain '.'");
    }
}

std::vector<std::string> CheckIR(const Module& module) {
    std::vector<std::string> errors;
    CheckData(module, errors);
    for (const Function& fn : module.functions) {
        FunctionChecker(module, fn, errors).Run();
    }
    return errors;
}

}  // namespace ir
