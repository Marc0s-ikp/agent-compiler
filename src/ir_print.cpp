#include <sstream>

#include "ir.hpp"

namespace ir {

namespace {

std::string OperandText(const Operand& o) {
    switch (o.kind) {
        case Operand::Kind::Temp: return "%" + std::to_string(o.temp);
        case Operand::Kind::Int: return std::to_string(o.int_value);
        case Operand::Kind::Float: {
            std::ostringstream s;
            s << o.float_value;
            return s.str();
        }
        case Operand::Kind::Object: return "$" + o.name;
        case Operand::Kind::Symbol: return "@" + o.name;
        case Operand::Kind::None: return "_";
    }
    return "?";
}

std::string ArgList(const std::vector<Operand>& args) {
    std::string text;
    for (size_t i = 0; i < args.size(); ++i) {
        text += (i ? ", " : "") + OperandText(args[i]);
    }
    return text;
}

// The part after '= ' (or the whole line when there is no result).
std::string Body(const Instruction& in) {
    std::string name = OpName(in.op);
    std::string type = TypeName(in.type);
    switch (in.op) {
        case Op::Label: return in.label + ":";
        case Op::Jump: return name + " " + in.label;
        case Op::Branch:
            return name + " " + ArgList(in.args) + ", " + in.label + ", " + in.label2;
        case Op::Addr:
            return name + " " + ArgList(in.args) +
                   (in.offset ? " + " + std::to_string(in.offset) : "");
        case Op::MemCpy:
        case Op::MemZero:
            return name + " " + ArgList(in.args) + ", " + std::to_string(in.size) +
                   ", align " + std::to_string(in.align);
        case Op::Ret: return in.args.empty() ? name : name + " " + type + " " + ArgList(in.args);
        default: break;
    }
    if (IsConversion(in.op)) {
        return name + " " + TypeName(in.args[0].type) + " " + ArgList(in.args) +
               " to " + type;
    }
    return name + " " + type + " " + ArgList(in.args);
}

void PrintFunction(const Function& fn, std::ostream& os) {
    os << (fn.is_global ? "func " : "static func ") << fn.name << "() -> "
       << (fn.returns_value ? TypeName(fn.return_type) : "void") << " {\n";
    for (const FrameObject& o : fn.objects) {
        os << "  frame $" << o.name << " size " << o.size << " align " << o.align << "\n";
    }
    for (const Instruction& in : fn.body) {
        if (in.op == Op::Label) {
            os << Body(in) << "\n";
        } else if (in.dst.kind == Operand::Kind::Temp) {
            os << "  " << OperandText(in.dst) << " = " << Body(in) << "\n";
        } else {
            os << "  " << Body(in) << "\n";
        }
    }
    os << "}\n";
}

void PrintData(const DataEntry& d, std::ostream& os) {
    os << "data @" << d.label << " size " << d.size << " align " << d.align
       << (d.read_only ? " rodata" : "") << (d.internal ? " internal" : "") << "\n";
}

}  // namespace

void Print(const Module& module, std::ostream& os) {
    for (const DataEntry& d : module.data) PrintData(d, os);
    for (const Function& fn : module.functions) PrintFunction(fn, os);
}

}  // namespace ir
