#include "ast_control.hpp"

#include <sstream>
#include <stdexcept>

#include "ast_operators.hpp"
#include "emit.hpp"

namespace ast {

namespace {

void EmitBody(std::ostream& os, Context& ctx, const Stmt& body,
              const std::string& break_label,
              const std::string& continue_label) {
    ctx.PushLoop(break_label, continue_label);
    body.EmitRISC(os, ctx);
    ctx.PopLoop();
}

}  // namespace

void IfStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    std::string else_label = ctx.NewLabel("else");
    std::string end = ctx.NewLabel("endif");
    EmitCondition(os, ctx, *cond_);
    EmitBranchIfZero(os, else_label);
    then_->EmitRISC(os, ctx);
    if (else_) EmitJump(os, end);
    os << else_label << ":\n";
    if (else_) {
        else_->EmitRISC(os, ctx);
        os << end << ":\n";
    }
}

void WhileStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    std::string top = ctx.NewLabel("while");
    std::string end = ctx.NewLabel("wend");
    os << top << ":\n";
    EmitCondition(os, ctx, *cond_);
    EmitBranchIfZero(os, end);
    EmitBody(os, ctx, *body_, end, top);
    EmitJump(os, top);
    os << end << ":\n";
}

void DoWhileStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    std::string top = ctx.NewLabel("do");
    std::string test = ctx.NewLabel("dotest");
    std::string end = ctx.NewLabel("doend");
    os << top << ":\n";
    EmitBody(os, ctx, *body_, end, test);
    os << test << ":\n";
    EmitCondition(os, ctx, *cond_);
    EmitBranchIfNonZero(os, top);
    os << end << ":\n";
}

void ForStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    std::string top = ctx.NewLabel("for");
    std::string step = ctx.NewLabel("forstep");
    std::string end = ctx.NewLabel("forend");
    if (init_) init_->EmitRISC(os, ctx);
    os << top << ":\n";
    if (cond_) {
        EmitCondition(os, ctx, *cond_);
        EmitBranchIfZero(os, end);
    }
    EmitBody(os, ctx, *body_, end, step);
    os << step << ":\n";
    if (step_) step_->EmitRISC(os, ctx);
    EmitJump(os, top);
    os << end << ":\n";
}

// The controlling value stays in a0 while the cases are compared: the
// dispatch is straight-line code that uses only t0, so it is evaluated once.
// Each comparison skips over a long-range jump, like EmitBranchIfZero.
void SwitchStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    RequireInteger(cond_->ValueType(ctx));
    cond_->EmitRISC(os, ctx);
    std::string end = ctx.NewLabel("swend");
    std::ostringstream body;
    ctx.PushSwitch(end);
    body_->EmitRISC(body, ctx);
    Context::SwitchInfo info = ctx.PopSwitch();
    for (const auto& [value, label] : info.cases) {
        os << "\tli t0, " << value << "\n\tbne a0, t0, 1f\n";
        EmitJump(os, label);
        os << "1:\n";
    }
    EmitJump(os, info.default_label.empty() ? end : info.default_label);
    os << body.str() << end << ":\n";
}

void CaseStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    std::optional<long> value = value_->EvalConst(ctx);
    if (!value) throw std::runtime_error("case label is not a constant");
    os << ctx.AddCase(*value) << ":\n";
    stmt_->EmitRISC(os, ctx);
}

void DefaultStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    os << ctx.AddDefault() << ":\n";
    stmt_->EmitRISC(os, ctx);
}

void BreakStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    EmitJump(os, ctx.BreakLabel());
}

void ContinueStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    EmitJump(os, ctx.ContinueLabel());
}

}  // namespace ast
