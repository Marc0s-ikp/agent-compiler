#include "ast_control.hpp"

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

void BreakStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    EmitJump(os, ctx.BreakLabel());
}

void ContinueStmt::EmitRISC(std::ostream& os, Context& ctx) const {
    EmitJump(os, ctx.ContinueLabel());
}

}  // namespace ast
