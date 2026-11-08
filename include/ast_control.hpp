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

private:
    ExprPtr cond_;
    StmtPtr then_, else_;
};

class WhileStmt : public Stmt {
public:
    WhileStmt(ExprPtr cond, StmtPtr body)
        : cond_(std::move(cond)), body_(std::move(body)) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;

private:
    ExprPtr cond_;
    StmtPtr body_;
};

class DoWhileStmt : public Stmt {
public:
    DoWhileStmt(StmtPtr body, ExprPtr cond)
        : body_(std::move(body)), cond_(std::move(cond)) {}
    void EmitRISC(std::ostream& os, Context& ctx) const override;

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

private:
    ExprPtr init_, cond_, step_;
    StmtPtr body_;
};

class BreakStmt : public Stmt {
public:
    void EmitRISC(std::ostream& os, Context& ctx) const override;
};

class ContinueStmt : public Stmt {
public:
    void EmitRISC(std::ostream& os, Context& ctx) const override;
};

}  // namespace ast
