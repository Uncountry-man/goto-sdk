#pragma once

#include "common.hpp"
#include "value.hpp"
#include "ast.hpp"
#include "environment.hpp"
#include "engine_events.hpp"
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

namespace goto_lang {

struct ReturnSignal {
    Value value;
};

struct GotoSignal {
    std::string labelName;
};

class Interpreter : public ExprVisitor, public StmtVisitor {
public:
    Interpreter();
    explicit Interpreter(std::shared_ptr<IEngineEventHandler> eventHandler);

    void setEventHandler(std::shared_ptr<IEngineEventHandler> handler);
    [[nodiscard]] std::shared_ptr<IEngineEventHandler> getEventHandler() const noexcept { return eventHandler_; }

    void execute(const std::vector<StmtPtr>& statements);
    void executeScript(const std::vector<StmtPtr>& statements, const std::string& startLabel = "");
    Value evaluate(Expr& expr);
    void executeStmt(Stmt& stmt);

    [[nodiscard]] std::shared_ptr<Environment> globals() const noexcept { return globals_; }
    [[nodiscard]] std::shared_ptr<Environment> environment() const noexcept { return environment_; }

    void setGlobal(const std::string& name, Value value);
    [[nodiscard]] Value getGlobal(const std::string& name) const;

    // State Serialization (Save / Load Game)
    [[nodiscard]] std::string exportStateJson() const;
    void importStateJson(const std::string& json);

    // Visitor Overrides for Expressions
    void visitLiteral(LiteralExpr& expr) override;
    void visitVariable(VariableExpr& expr) override;
    void visitUnary(UnaryExpr& expr) override;
    void visitBinary(BinaryExpr& expr) override;
    void visitLogical(LogicalExpr& expr) override;
    void visitCall(CallExpr& expr) override;
    void visitList(ListExpr& expr) override;
    void visitDict(DictExpr& expr) override;
    void visitIndex(IndexExpr& expr) override;
    void visitAssign(AssignExpr& expr) override;
    void visitIndexAssign(IndexAssignExpr& expr) override;

    // Visitor Overrides for Statements
    void visitExpr(ExprStmt& stmt) override;
    void visitVarDecl(VarDeclStmt& stmt) override;
    void visitBlock(BlockStmt& stmt) override;
    void visitIf(IfStmt& stmt) override;
    void visitWhile(WhileStmt& stmt) override;
    void visitForIn(ForInStmt& stmt) override;
    void visitFnDecl(FnDeclStmt& stmt) override;
    void visitReturn(ReturnStmt& stmt) override;
    void visitLabel(LabelStmt& stmt) override;
    void visitGoto(GotoStmt& stmt) override;
    void visitCallLabel(CallLabelStmt& stmt) override;
    void visitSay(SayStmt& stmt) override;
    void visitScene(SceneStmt& stmt) override;
    void visitShow(ShowStmt& stmt) override;
    void visitHide(HideStmt& stmt) override;
    void visitPlayAudio(PlayAudioStmt& stmt) override;
    void visitChoice(ChoiceStmt& stmt) override;

    void executeBlock(const std::vector<StmtPtr>& statements, std::shared_ptr<Environment> env);

private:
    void scanLabels(const std::vector<StmtPtr>& statements);
    void jumpToLabel(const std::string& labelName);

    std::shared_ptr<Environment> globals_;
    std::shared_ptr<Environment> environment_;
    std::shared_ptr<IEngineEventHandler> eventHandler_;

    std::unordered_map<std::string, size_t> labelIndices_;
    std::vector<StmtPtr> topLevelStatements_;
    size_t pc_{0};
    std::vector<size_t> callStack_;
    std::string currentLabel_{"main"};

    Value lastEvaluatedValue_{nullptr};
};

} // namespace goto_lang
