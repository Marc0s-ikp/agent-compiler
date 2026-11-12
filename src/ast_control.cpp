#include "ast_control.hpp"

#include <stdexcept>

#include "ast_operators.hpp"
#include "ast_lower.hpp"
#include "ir_gen.hpp"

namespace ast {

using ir::Operand;

namespace {

// A loop body with its break and continue targets in scope.
void LowerBody(IRGen& gen, const Stmt& body, const std::string& break_label,
               const std::string& continue_label) {
    gen.Ctx().PushLoop(break_label, continue_label);
    body.EmitIR(gen);
    gen.Ctx().PopLoop();
}

}  // namespace

void IfStmt::EmitIR(IRGen& gen) const {
    std::string then_label = gen.NewLabel("then");
    std::string else_label = else_ ? gen.NewLabel("else") : "";
    std::string end = gen.NewLabel("endif");
    gen.Branch(Condition(gen, *cond_), then_label, else_ ? else_label : end);
    gen.Label(then_label);
    then_->EmitIR(gen);
    if (else_) {
        gen.Jump(end);
        gen.Label(else_label);
        else_->EmitIR(gen);
    }
    gen.Label(end);
}

void WhileStmt::EmitIR(IRGen& gen) const {
    std::string top = gen.NewLabel("while");
    std::string body = gen.NewLabel("wbody");
    std::string end = gen.NewLabel("wend");
    gen.Label(top);
    gen.Branch(Condition(gen, *cond_), body, end);
    gen.Label(body);
    LowerBody(gen, *body_, end, top);
    gen.Jump(top);
    gen.Label(end);
}

void DoWhileStmt::EmitIR(IRGen& gen) const {
    std::string top = gen.NewLabel("do");
    std::string test = gen.NewLabel("dotest");
    std::string end = gen.NewLabel("doend");
    gen.Label(top);
    LowerBody(gen, *body_, end, test);
    gen.Label(test);
    gen.Branch(Condition(gen, *cond_), top, end);
    gen.Label(end);
}

// Any of init, cond and step may be missing; a missing cond is always true.
void ForStmt::EmitIR(IRGen& gen) const {
    std::string top = gen.NewLabel("for");
    std::string body = gen.NewLabel("fbody");
    std::string step = gen.NewLabel("forstep");
    std::string end = gen.NewLabel("forend");
    if (init_) init_->EmitValue(gen);
    gen.Label(top);
    if (cond_) gen.Branch(Condition(gen, *cond_), body, end);
    gen.Label(body);
    LowerBody(gen, *body_, end, step);
    gen.Label(step);
    if (step_) step_->EmitValue(gen);
    gen.Jump(top);
    gen.Label(end);
}

// The controlling value is computed once, then control jumps over the body to
// the dispatch chain, which is written after the body because the case labels
// inside it decide the chain:
//     %c = <cond>; jump dispatch; <body>; jump end;
//   dispatch: (eq, branch) per case; jump default-or-end;  end:
void SwitchStmt::EmitIR(IRGen& gen) const {
    Context& ctx = gen.Ctx();
    TypePtr type = Promote(cond_->ValueType(ctx));
    RequireIrInteger(*type);
    Operand value = ValueAs(gen, *cond_, *type);
    std::string dispatch = gen.NewLabel("swdispatch");
    std::string end = gen.NewLabel("swend");
    gen.Jump(dispatch);
    ctx.PushSwitch(end);
    body_->EmitIR(gen);
    Context::SwitchInfo info = ctx.PopSwitch();
    gen.Jump(end);
    gen.Label(dispatch);
    ir::IrType ir_type = IRGen::ToIr(*type);
    for (const auto& [case_value, label] : info.cases) {
        Operand same = gen.Compare(ir::Op::Eq, ir_type, value, Operand::Int(case_value, ir_type));
        std::string next = gen.NewLabel("swnext");
        gen.Branch(same, label, next);
        gen.Label(next);
    }
    gen.Jump(info.default_label.empty() ? end : info.default_label);
    gen.Label(end);
}

void CaseStmt::EmitIR(IRGen& gen) const {
    std::optional<long> value = value_->EvalConst(gen.Ctx());
    if (!value) throw std::runtime_error("case label is not a constant");
    gen.Label(gen.Ctx().AddCase(*value));
    stmt_->EmitIR(gen);
}

void DefaultStmt::EmitIR(IRGen& gen) const {
    gen.Label(gen.Ctx().AddDefault());
    stmt_->EmitIR(gen);
}

void BreakStmt::EmitIR(IRGen& gen) const { gen.Jump(gen.Ctx().BreakLabel()); }

void ContinueStmt::EmitIR(IRGen& gen) const { gen.Jump(gen.Ctx().ContinueLabel()); }

}  // namespace ast
