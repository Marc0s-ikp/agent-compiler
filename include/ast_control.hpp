#pragma once
#include "ast_node.hpp"

namespace ast {

class IfStmt : public Stmt {
public:
    // 'else_stmt' may be null.
    IfStmt(ExprPtr cond, StmtPtr then_stmt, StmtPtr else_stmt)
        : cond_(std::move(cond)),
          then_(std::move(then_stmt)),
          else_(std::move(else_stmt)) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;
    void EmitIR(IRGen& gen) const override;

private:
    ExprPtr cond_;
    StmtPtr then_, else_;
};

class WhileStmt : public Stmt {
public:
    WhileStmt(ExprPtr cond, StmtPtr body)
        : cond_(std::move(cond)), body_(std::move(body)) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;
    void EmitIR(IRGen& gen) const override;

private:
    ExprPtr cond_;
    StmtPtr body_;
};

class DoWhileStmt : public Stmt {
public:
    DoWhileStmt(StmtPtr body, ExprPtr cond)
        : body_(std::move(body)), cond_(std::move(cond)) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;
    void EmitIR(IRGen& gen) const override;

private:
    StmtPtr body_;
    ExprPtr cond_;
};

// for (init; cond; step) body. Each of init, cond and step may be null.
class ForStmt : public Stmt {
public:
    ForStmt(ExprPtr init, ExprPtr cond, ExprPtr step, StmtPtr body)
        : init_(std::move(init)),
          cond_(std::move(cond)),
          step_(std::move(step)),
          body_(std::move(body)) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;
    void EmitIR(IRGen& gen) const override;

private:
    ExprPtr init_, cond_, step_;
    StmtPtr body_;
};

// switch (cond) body. The body is generated into a buffer first, because the
// case labels it contains decide the dispatch code that precedes it.
class SwitchStmt : public Stmt {
public:
    SwitchStmt(ExprPtr cond, StmtPtr body)
        : cond_(std::move(cond)), body_(std::move(body)) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;
    void EmitIR(IRGen& gen) const override;

private:
    ExprPtr cond_;
    StmtPtr body_;
};

// 'case value: stmt' and 'default: stmt'. A label does nothing at run time;
// the dispatch jumps to it.
class CaseStmt : public Stmt {
public:
    CaseStmt(ExprPtr value, StmtPtr stmt)
        : value_(std::move(value)), stmt_(std::move(stmt)) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;
    void EmitIR(IRGen& gen) const override;

private:
    ExprPtr value_;
    StmtPtr stmt_;
};

class DefaultStmt : public Stmt {
public:
    explicit DefaultStmt(StmtPtr stmt) : stmt_(std::move(stmt)) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;
    void EmitIR(IRGen& gen) const override;

private:
    StmtPtr stmt_;
};

class BreakStmt : public Stmt {
public:
    void EmitRISC(std::ostream& os, Context& ctx) const override;
    void EmitIR(IRGen& gen) const override;
};

class ContinueStmt : public Stmt {
public:
    void EmitRISC(std::ostream& os, Context& ctx) const override;
    void EmitIR(IRGen& gen) const override;
};

}  // namespace ast
