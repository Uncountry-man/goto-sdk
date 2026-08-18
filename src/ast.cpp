#include "../include/goto/ast.hpp"

namespace goto_lang {

void LiteralExpr::accept(ExprVisitor& visitor) { visitor.visitLiteral(*this); }
void VariableExpr::accept(ExprVisitor& visitor) { visitor.visitVariable(*this); }
void UnaryExpr::accept(ExprVisitor& visitor) { visitor.visitUnary(*this); }
void BinaryExpr::accept(ExprVisitor& visitor) { visitor.visitBinary(*this); }
void LogicalExpr::accept(ExprVisitor& visitor) { visitor.visitLogical(*this); }
void CallExpr::accept(ExprVisitor& visitor) { visitor.visitCall(*this); }
void ListExpr::accept(ExprVisitor& visitor) { visitor.visitList(*this); }
void DictExpr::accept(ExprVisitor& visitor) { visitor.visitDict(*this); }
void IndexExpr::accept(ExprVisitor& visitor) { visitor.visitIndex(*this); }
void AssignExpr::accept(ExprVisitor& visitor) { visitor.visitAssign(*this); }
void IndexAssignExpr::accept(ExprVisitor& visitor) { visitor.visitIndexAssign(*this); }

void ExprStmt::accept(StmtVisitor& visitor) { visitor.visitExpr(*this); }
void VarDeclStmt::accept(StmtVisitor& visitor) { visitor.visitVarDecl(*this); }
void BlockStmt::accept(StmtVisitor& visitor) { visitor.visitBlock(*this); }
void IfStmt::accept(StmtVisitor& visitor) { visitor.visitIf(*this); }
void WhileStmt::accept(StmtVisitor& visitor) { visitor.visitWhile(*this); }
void ForInStmt::accept(StmtVisitor& visitor) { visitor.visitForIn(*this); }
void FnDeclStmt::accept(StmtVisitor& visitor) { visitor.visitFnDecl(*this); }
void ReturnStmt::accept(StmtVisitor& visitor) { visitor.visitReturn(*this); }
void LabelStmt::accept(StmtVisitor& visitor) { visitor.visitLabel(*this); }
void GotoStmt::accept(StmtVisitor& visitor) { visitor.visitGoto(*this); }
void CallLabelStmt::accept(StmtVisitor& visitor) { visitor.visitCallLabel(*this); }
void SayStmt::accept(StmtVisitor& visitor) { visitor.visitSay(*this); }
void SceneStmt::accept(StmtVisitor& visitor) { visitor.visitScene(*this); }
void ShowStmt::accept(StmtVisitor& visitor) { visitor.visitShow(*this); }
void HideStmt::accept(StmtVisitor& visitor) { visitor.visitHide(*this); }
void PlayAudioStmt::accept(StmtVisitor& visitor) { visitor.visitPlayAudio(*this); }
void ChoiceStmt::accept(StmtVisitor& visitor) { visitor.visitChoice(*this); }

} // namespace goto_lang
