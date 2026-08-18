#pragma once

#include "common.hpp"
#include "token.hpp"
#include "value.hpp"
#include <memory>
#include <vector>
#include <string>
#include <optional>

namespace goto_lang {

// Forward declarations
struct Expr;
struct Stmt;

using ExprPtr = std::shared_ptr<Expr>;
using StmtPtr = std::shared_ptr<Stmt>;

// Visitor interface for Expressions
struct ExprVisitor;
// Visitor interface for Statements
struct StmtVisitor;

// Base Expression
struct Expr {
    SourceLocation location;
    explicit Expr(SourceLocation loc) : location(std::move(loc)) {}
    virtual ~Expr() = default;
    virtual void accept(ExprVisitor& visitor) = 0;
};

// Base Statement
struct Stmt {
    SourceLocation location;
    explicit Stmt(SourceLocation loc) : location(std::move(loc)) {}
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor& visitor) = 0;
};

// Expressions
struct LiteralExpr : public Expr {
    Value value;
    LiteralExpr(Value val, SourceLocation loc) : Expr(std::move(loc)), value(std::move(val)) {}
    void accept(ExprVisitor& visitor) override;
};

struct VariableExpr : public Expr {
    std::string name;
    VariableExpr(std::string n, SourceLocation loc) : Expr(std::move(loc)), name(std::move(n)) {}
    void accept(ExprVisitor& visitor) override;
};

struct UnaryExpr : public Expr {
    TokenType op;
    ExprPtr right;
    UnaryExpr(TokenType o, ExprPtr r, SourceLocation loc)
        : Expr(std::move(loc)), op(o), right(std::move(r)) {}
    void accept(ExprVisitor& visitor) override;
};

struct BinaryExpr : public Expr {
    ExprPtr left;
    TokenType op;
    ExprPtr right;
    BinaryExpr(ExprPtr l, TokenType o, ExprPtr r, SourceLocation loc)
        : Expr(std::move(loc)), left(std::move(l)), op(o), right(std::move(r)) {}
    void accept(ExprVisitor& visitor) override;
};

struct LogicalExpr : public Expr {
    ExprPtr left;
    TokenType op; // KwAnd or KwOr
    ExprPtr right;
    LogicalExpr(ExprPtr l, TokenType o, ExprPtr r, SourceLocation loc)
        : Expr(std::move(loc)), left(std::move(l)), op(o), right(std::move(r)) {}
    void accept(ExprVisitor& visitor) override;
};

struct CallExpr : public Expr {
    ExprPtr callee;
    std::vector<ExprPtr> arguments;
    CallExpr(ExprPtr c, std::vector<ExprPtr> args, SourceLocation loc)
        : Expr(std::move(loc)), callee(std::move(c)), arguments(std::move(args)) {}
    void accept(ExprVisitor& visitor) override;
};

struct ListExpr : public Expr {
    std::vector<ExprPtr> elements;
    ListExpr(std::vector<ExprPtr> elems, SourceLocation loc)
        : Expr(std::move(loc)), elements(std::move(elems)) {}
    void accept(ExprVisitor& visitor) override;
};

struct DictExpr : public Expr {
    std::vector<std::pair<ExprPtr, ExprPtr>> entries; // key, value
    DictExpr(std::vector<std::pair<ExprPtr, ExprPtr>> ent, SourceLocation loc)
        : Expr(std::move(loc)), entries(std::move(ent)) {}
    void accept(ExprVisitor& visitor) override;
};

struct IndexExpr : public Expr {
    ExprPtr target;
    ExprPtr index;
    IndexExpr(ExprPtr t, ExprPtr idx, SourceLocation loc)
        : Expr(std::move(loc)), target(std::move(t)), index(std::move(idx)) {}
    void accept(ExprVisitor& visitor) override;
};

struct AssignExpr : public Expr {
    std::string name;
    TokenType op; // =, +=, -=, *=, /=
    ExprPtr value;
    AssignExpr(std::string n, TokenType o, ExprPtr val, SourceLocation loc)
        : Expr(std::move(loc)), name(std::move(n)), op(o), value(std::move(val)) {}
    void accept(ExprVisitor& visitor) override;
};

struct IndexAssignExpr : public Expr {
    ExprPtr target;
    ExprPtr index;
    TokenType op; // =, +=, etc.
    ExprPtr value;
    IndexAssignExpr(ExprPtr t, ExprPtr idx, TokenType o, ExprPtr val, SourceLocation loc)
        : Expr(std::move(loc)), target(std::move(t)), index(std::move(idx)), op(o), value(std::move(val)) {}
    void accept(ExprVisitor& visitor) override;
};

// Statements
struct ExprStmt : public Stmt {
    ExprPtr expression;
    ExprStmt(ExprPtr expr, SourceLocation loc) : Stmt(std::move(loc)), expression(std::move(expr)) {}
    void accept(StmtVisitor& visitor) override;
};

struct VarDeclStmt : public Stmt {
    std::string name;
    ExprPtr initializer;
    VarDeclStmt(std::string n, ExprPtr init, SourceLocation loc)
        : Stmt(std::move(loc)), name(std::move(n)), initializer(std::move(init)) {}
    void accept(StmtVisitor& visitor) override;
};

struct BlockStmt : public Stmt {
    std::vector<StmtPtr> statements;
    BlockStmt(std::vector<StmtPtr> stmts, SourceLocation loc)
        : Stmt(std::move(loc)), statements(std::move(stmts)) {}
    void accept(StmtVisitor& visitor) override;
};

struct IfBranch {
    ExprPtr condition;
    StmtPtr body;
};

struct IfStmt : public Stmt {
    std::vector<IfBranch> branches;
    StmtPtr elseBranch; // can be nullptr
    IfStmt(std::vector<IfBranch> br, StmtPtr els, SourceLocation loc)
        : Stmt(std::move(loc)), branches(std::move(br)), elseBranch(std::move(els)) {}
    void accept(StmtVisitor& visitor) override;
};

struct WhileStmt : public Stmt {
    ExprPtr condition;
    StmtPtr body;
    WhileStmt(ExprPtr cond, StmtPtr b, SourceLocation loc)
        : Stmt(std::move(loc)), condition(std::move(cond)), body(std::move(b)) {}
    void accept(StmtVisitor& visitor) override;
};

struct ForInStmt : public Stmt {
    std::string variableName;
    ExprPtr iterable;
    StmtPtr body;
    ForInStmt(std::string var, ExprPtr iter, StmtPtr b, SourceLocation loc)
        : Stmt(std::move(loc)), variableName(std::move(var)), iterable(std::move(iter)), body(std::move(b)) {}
    void accept(StmtVisitor& visitor) override;
};

struct FnDeclStmt : public Stmt {
    std::string name;
    std::vector<std::string> params;
    StmtPtr body;
    FnDeclStmt(std::string n, std::vector<std::string> p, StmtPtr b, SourceLocation loc)
        : Stmt(std::move(loc)), name(std::move(n)), params(std::move(p)), body(std::move(b)) {}
    void accept(StmtVisitor& visitor) override;
};

struct ReturnStmt : public Stmt {
    ExprPtr value; // can be nullptr
    ReturnStmt(ExprPtr val, SourceLocation loc) : Stmt(std::move(loc)), value(std::move(val)) {}
    void accept(StmtVisitor& visitor) override;
};

struct LabelStmt : public Stmt {
    std::string name;
    LabelStmt(std::string n, SourceLocation loc) : Stmt(std::move(loc)), name(std::move(n)) {}
    void accept(StmtVisitor& visitor) override;
};

struct GotoStmt : public Stmt {
    std::string targetLabel;
    GotoStmt(std::string target, SourceLocation loc)
        : Stmt(std::move(loc)), targetLabel(std::move(target)) {}
    void accept(StmtVisitor& visitor) override;
};

struct CallLabelStmt : public Stmt {
    std::string targetLabel;
    CallLabelStmt(std::string target, SourceLocation loc)
        : Stmt(std::move(loc)), targetLabel(std::move(target)) {}
    void accept(StmtVisitor& visitor) override;
};

// Narrative Statements
struct SayStmt : public Stmt {
    std::optional<std::string> actor; // Speaker or nullopt for narrator
    ExprPtr dialogue;                 // Expression evaluating to string (or literal string)
    SayStmt(std::optional<std::string> act, ExprPtr dial, SourceLocation loc)
        : Stmt(std::move(loc)), actor(std::move(act)), dialogue(std::move(dial)) {}
    void accept(StmtVisitor& visitor) override;
};

struct SceneStmt : public Stmt {
    ExprPtr sceneName;
    std::string transition; // e.g. "fade", "dissolve", "instant"
    SceneStmt(ExprPtr name, std::string trans, SourceLocation loc)
        : Stmt(std::move(loc)), sceneName(std::move(name)), transition(std::move(trans)) {}
    void accept(StmtVisitor& visitor) override;
};

struct ShowStmt : public Stmt {
    std::string actor;
    std::string expression; // e.g. "happy", "sad", "angry"
    std::string position;   // e.g. "left", "center", "right"
    ShowStmt(std::string act, std::string expr, std::string pos, SourceLocation loc)
        : Stmt(std::move(loc)), actor(std::move(act)), expression(std::move(expr)), position(std::move(pos)) {}
    void accept(StmtVisitor& visitor) override;
};

struct HideStmt : public Stmt {
    std::string actor;
    HideStmt(std::string act, SourceLocation loc) : Stmt(std::move(loc)), actor(std::move(act)) {}
    void accept(StmtVisitor& visitor) override;
};

struct PlayAudioStmt : public Stmt {
    std::string channel; // "music", "sound", "voice"
    ExprPtr track;
    double fade{0.0};
    bool loop{true};
    PlayAudioStmt(std::string ch, ExprPtr tr, double f, bool lp, SourceLocation loc)
        : Stmt(std::move(loc)), channel(std::move(ch)), track(std::move(tr)), fade(f), loop(lp) {}
    void accept(StmtVisitor& visitor) override;
};

struct ChoiceOption {
    ExprPtr text;
    std::optional<std::string> targetLabel;
    StmtPtr body; // Optional inline block
};

struct ChoiceStmt : public Stmt {
    std::vector<ChoiceOption> options;
    ChoiceStmt(std::vector<ChoiceOption> opts, SourceLocation loc)
        : Stmt(std::move(loc)), options(std::move(opts)) {}
    void accept(StmtVisitor& visitor) override;
};

// Visitors
struct ExprVisitor {
    virtual ~ExprVisitor() = default;
    virtual void visitLiteral(LiteralExpr& expr) = 0;
    virtual void visitVariable(VariableExpr& expr) = 0;
    virtual void visitUnary(UnaryExpr& expr) = 0;
    virtual void visitBinary(BinaryExpr& expr) = 0;
    virtual void visitLogical(LogicalExpr& expr) = 0;
    virtual void visitCall(CallExpr& expr) = 0;
    virtual void visitList(ListExpr& expr) = 0;
    virtual void visitDict(DictExpr& expr) = 0;
    virtual void visitIndex(IndexExpr& expr) = 0;
    virtual void visitAssign(AssignExpr& expr) = 0;
    virtual void visitIndexAssign(IndexAssignExpr& expr) = 0;
};

struct StmtVisitor {
    virtual ~StmtVisitor() = default;
    virtual void visitExpr(ExprStmt& stmt) = 0;
    virtual void visitVarDecl(VarDeclStmt& stmt) = 0;
    virtual void visitBlock(BlockStmt& stmt) = 0;
    virtual void visitIf(IfStmt& stmt) = 0;
    virtual void visitWhile(WhileStmt& stmt) = 0;
    virtual void visitForIn(ForInStmt& stmt) = 0;
    virtual void visitFnDecl(FnDeclStmt& stmt) = 0;
    virtual void visitReturn(ReturnStmt& stmt) = 0;
    virtual void visitLabel(LabelStmt& stmt) = 0;
    virtual void visitGoto(GotoStmt& stmt) = 0;
    virtual void visitCallLabel(CallLabelStmt& stmt) = 0;
    virtual void visitSay(SayStmt& stmt) = 0;
    virtual void visitScene(SceneStmt& stmt) = 0;
    virtual void visitShow(ShowStmt& stmt) = 0;
    virtual void visitHide(HideStmt& stmt) = 0;
    virtual void visitPlayAudio(PlayAudioStmt& stmt) = 0;
    virtual void visitChoice(ChoiceStmt& stmt) = 0;
};

} // namespace goto_lang
