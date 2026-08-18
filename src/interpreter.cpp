#include "../include/goto/interpreter.hpp"
#include <iostream>
#include <sstream>

namespace goto_lang {

Interpreter::Interpreter()
    : globals_(std::make_shared<Environment>()),
      environment_(globals_),
      eventHandler_(std::make_shared<IEngineEventHandler>()) {}

Interpreter::Interpreter(std::shared_ptr<IEngineEventHandler> eventHandler)
    : globals_(std::make_shared<Environment>()),
      environment_(globals_),
      eventHandler_(std::move(eventHandler)) {
    if (!eventHandler_) {
        eventHandler_ = std::make_shared<IEngineEventHandler>();
    }
}

void Interpreter::setEventHandler(std::shared_ptr<IEngineEventHandler> handler) {
    if (handler) {
        eventHandler_ = std::move(handler);
    }
}

void Interpreter::setGlobal(const std::string& name, Value value) {
    globals_->define(name, std::move(value));
}

Value Interpreter::getGlobal(const std::string& name) const {
    return globals_->get(name);
}

std::string Interpreter::exportStateJson() const {
    auto vars = globals_->getAllVariables();
    std::string out = "{\n";
    out += "  \"current_label\": \"" + currentLabel_ + "\",\n";
    out += "  \"variables\": {\n";
    bool first = true;
    for (const auto& [k, v] : vars) {
        if (!first) out += ",\n";
        first = false;
        out += "    \"" + k + "\": " + v.toJson();
    }
    out += "\n  }\n}";
    return out;
}

void Interpreter::importStateJson(const std::string& json) {
    // Basic state loader for variables
    // In production VN engine, this restores saved dictionary states
    (void)json;
}

void Interpreter::scanLabels(const std::vector<StmtPtr>& statements) {
    labelIndices_.clear();
    for (size_t i = 0; i < statements.size(); ++i) {
        if (auto lbl = std::dynamic_pointer_cast<LabelStmt>(statements[i])) {
            labelIndices_[lbl->name] = i;
        }
    }
}

void Interpreter::jumpToLabel(const std::string& labelName) {
    auto it = labelIndices_.find(labelName);
    if (it == labelIndices_.end()) {
        throw GoToException("Label not found: '" + labelName + "'");
    }
    pc_ = it->second;
    currentLabel_ = labelName;
}

void Interpreter::execute(const std::vector<StmtPtr>& statements) {
    executeScript(statements, "");
}

void Interpreter::executeScript(const std::vector<StmtPtr>& statements, const std::string& startLabel) {
    topLevelStatements_ = statements;
    scanLabels(topLevelStatements_);
    callStack_.clear();
    pc_ = 0;

    if (!startLabel.empty()) {
        jumpToLabel(startLabel);
    }

    while (pc_ < topLevelStatements_.size()) {
        try {
            StmtPtr stmt = topLevelStatements_[pc_];
            pc_++;
            if (stmt) {
                stmt->accept(*this);
            }
        } catch (const GotoSignal& signal) {
            jumpToLabel(signal.labelName);
        } catch (const ReturnSignal&) {
            if (!callStack_.empty()) {
                pc_ = callStack_.back();
                callStack_.pop_back();
            } else {
                // Return at top-level ends script execution
                break;
            }
        }
    }
}

Value Interpreter::evaluate(Expr& expr) {
    expr.accept(*this);
    return lastEvaluatedValue_;
}

void Interpreter::executeStmt(Stmt& stmt) {
    stmt.accept(*this);
}

void Interpreter::executeBlock(const std::vector<StmtPtr>& statements, std::shared_ptr<Environment> env) {
    std::shared_ptr<Environment> previous = environment_;
    try {
        environment_ = std::move(env);
        for (const auto& stmt : statements) {
            if (stmt) {
                executeStmt(*stmt);
            }
        }
    } catch (...) {
        environment_ = previous;
        throw;
    }
    environment_ = previous;
}

// Expressions
void Interpreter::visitLiteral(LiteralExpr& expr) {
    lastEvaluatedValue_ = expr.value;
}

void Interpreter::visitVariable(VariableExpr& expr) {
    lastEvaluatedValue_ = environment_->get(expr.name);
}

void Interpreter::visitUnary(UnaryExpr& expr) {
    Value right = evaluate(*expr.right);
    if (expr.op == TokenType::Minus) {
        lastEvaluatedValue_ = -right;
        return;
    }
    if (expr.op == TokenType::KwNot) {
        lastEvaluatedValue_ = Value(!right.isTruthy());
        return;
    }
    throw GoToException("Unknown unary operator", expr.location);
}

void Interpreter::visitBinary(BinaryExpr& expr) {
    Value left = evaluate(*expr.left);
    Value right = evaluate(*expr.right);

    switch (expr.op) {
        case TokenType::Plus:
            lastEvaluatedValue_ = left + right;
            break;
        case TokenType::Minus:
            lastEvaluatedValue_ = left - right;
            break;
        case TokenType::Star:
            lastEvaluatedValue_ = left * right;
            break;
        case TokenType::Slash:
            lastEvaluatedValue_ = left / right;
            break;
        case TokenType::Percent:
            lastEvaluatedValue_ = left % right;
            break;
        case TokenType::Equal:
            lastEvaluatedValue_ = Value(left == right);
            break;
        case TokenType::NotEqual:
            lastEvaluatedValue_ = Value(left != right);
            break;
        case TokenType::Less:
            lastEvaluatedValue_ = Value(left < right);
            break;
        case TokenType::LessEqual:
            lastEvaluatedValue_ = Value(left <= right);
            break;
        case TokenType::Greater:
            lastEvaluatedValue_ = Value(left > right);
            break;
        case TokenType::GreaterEqual:
            lastEvaluatedValue_ = Value(left >= right);
            break;
        default:
            throw GoToException("Unknown binary operator", expr.location);
    }
}

void Interpreter::visitLogical(LogicalExpr& expr) {
    Value left = evaluate(*expr.left);

    if (expr.op == TokenType::KwOr) {
        if (left.isTruthy()) {
            lastEvaluatedValue_ = left;
            return;
        }
    } else if (expr.op == TokenType::KwAnd) {
        if (!left.isTruthy()) {
            lastEvaluatedValue_ = left;
            return;
        }
    }
    lastEvaluatedValue_ = evaluate(*expr.right);
}

void Interpreter::visitCall(CallExpr& expr) {
    Value callee = evaluate(*expr.callee);

    std::vector<Value> args;
    args.reserve(expr.arguments.size());
    for (const auto& arg : expr.arguments) {
        args.push_back(evaluate(*arg));
    }

    if (callee.isNativeFunction()) {
        const auto& nativeFn = callee.asNativeFunction();
        lastEvaluatedValue_ = nativeFn(args, *environment_);
        return;
    }

    if (callee.isFunction()) {
        const auto& fnData = callee.asFunction();
        if (args.size() != fnData.params.size()) {
            throw GoToException("Function '" + fnData.name + "' expected " +
                                std::to_string(fnData.params.size()) + " arguments, but got " +
                                std::to_string(args.size()), expr.location);
        }

        auto fnEnv = std::make_shared<Environment>(fnData.closure ? fnData.closure : globals_);
        for (size_t i = 0; i < fnData.params.size(); ++i) {
            fnEnv->define(fnData.params[i], args[i]);
        }

        try {
            if (fnData.body) {
                if (auto block = std::dynamic_pointer_cast<BlockStmt>(fnData.body)) {
                    executeBlock(block->statements, fnEnv);
                } else {
                    std::shared_ptr<Environment> prev = environment_;
                    environment_ = fnEnv;
                    executeStmt(*fnData.body);
                    environment_ = prev;
                }
            }
            lastEvaluatedValue_ = Value(nullptr);
        } catch (const ReturnSignal& ret) {
            lastEvaluatedValue_ = ret.value;
        }
        return;
    }

    throw GoToException("Can only call functions, got " + callee.typeName(), expr.location);
}

void Interpreter::visitList(ListExpr& expr) {
    std::vector<Value> elems;
    elems.reserve(expr.elements.size());
    for (const auto& elem : expr.elements) {
        elems.push_back(evaluate(*elem));
    }
    lastEvaluatedValue_ = Value::makeList(std::move(elems));
}

void Interpreter::visitDict(DictExpr& expr) {
    std::unordered_map<std::string, Value> map;
    for (const auto& [k, v] : expr.entries) {
        Value keyVal = evaluate(*k);
        Value valVal = evaluate(*v);
        map[keyVal.toString()] = valVal;
    }
    lastEvaluatedValue_ = Value::makeDict(std::move(map));
}

void Interpreter::visitIndex(IndexExpr& expr) {
    Value target = evaluate(*expr.target);
    Value index = evaluate(*expr.index);
    lastEvaluatedValue_ = target.getSubscript(index);
}

void Interpreter::visitAssign(AssignExpr& expr) {
    Value val = evaluate(*expr.value);

    if (expr.op == TokenType::Assign) {
        environment_->assign(expr.name, val);
        lastEvaluatedValue_ = val;
        return;
    }

    Value current = environment_->get(expr.name);
    Value result;
    switch (expr.op) {
        case TokenType::PlusAssign:
            result = current + val;
            break;
        case TokenType::MinusAssign:
            result = current - val;
            break;
        case TokenType::StarAssign:
            result = current * val;
            break;
        case TokenType::SlashAssign:
            result = current / val;
            break;
        default:
            throw GoToException("Unsupported assignment operator", expr.location);
    }
    environment_->assign(expr.name, result);
    lastEvaluatedValue_ = result;
}

void Interpreter::visitIndexAssign(IndexAssignExpr& expr) {
    Value target = evaluate(*expr.target);
    Value index = evaluate(*expr.index);
    Value val = evaluate(*expr.value);

    if (expr.op == TokenType::Assign) {
        target.setSubscript(index, val);
        lastEvaluatedValue_ = val;
        return;
    }

    Value current = target.getSubscript(index);
    Value result;
    switch (expr.op) {
        case TokenType::PlusAssign:
            result = current + val;
            break;
        case TokenType::MinusAssign:
            result = current - val;
            break;
        case TokenType::StarAssign:
            result = current * val;
            break;
        case TokenType::SlashAssign:
            result = current / val;
            break;
        default:
            throw GoToException("Unsupported assignment operator", expr.location);
    }
    target.setSubscript(index, result);
    lastEvaluatedValue_ = result;
}

// Statements
void Interpreter::visitExpr(ExprStmt& stmt) {
    evaluate(*stmt.expression);
}

void Interpreter::visitVarDecl(VarDeclStmt& stmt) {
    Value val = stmt.initializer ? evaluate(*stmt.initializer) : Value(nullptr);
    environment_->define(stmt.name, val);
}

void Interpreter::visitBlock(BlockStmt& stmt) {
    executeBlock(stmt.statements, std::make_shared<Environment>(environment_));
}

void Interpreter::visitIf(IfStmt& stmt) {
    for (const auto& branch : stmt.branches) {
        Value cond = evaluate(*branch.condition);
        if (cond.isTruthy()) {
            executeStmt(*branch.body);
            return;
        }
    }
    if (stmt.elseBranch) {
        executeStmt(*stmt.elseBranch);
    }
}

void Interpreter::visitWhile(WhileStmt& stmt) {
    while (evaluate(*stmt.condition).isTruthy()) {
        executeStmt(*stmt.body);
    }
}

void Interpreter::visitForIn(ForInStmt& stmt) {
    Value iter = evaluate(*stmt.iterable);

    if (iter.isList()) {
        auto list = iter.asList();
        for (const auto& item : *list) {
            environment_->define(stmt.variableName, item);
            executeStmt(*stmt.body);
        }
        return;
    }

    if (iter.isDict()) {
        auto dict = iter.asDict();
        for (const auto& [k, v] : *dict) {
            environment_->define(stmt.variableName, Value(k));
            executeStmt(*stmt.body);
        }
        return;
    }

    if (iter.isString()) {
        const std::string& str = iter.asString();
        for (char c : str) {
            environment_->define(stmt.variableName, Value(std::string(1, c)));
            executeStmt(*stmt.body);
        }
        return;
    }

    throw GoToException("Cannot iterate over type " + iter.typeName(), stmt.location);
}

void Interpreter::visitFnDecl(FnDeclStmt& stmt) {
    FunctionData fnData;
    fnData.name = stmt.name;
    fnData.params = stmt.params;
    fnData.body = stmt.body;
    fnData.closure = environment_;
    environment_->define(stmt.name, Value(std::move(fnData)));
}

void Interpreter::visitReturn(ReturnStmt& stmt) {
    Value val = stmt.value ? evaluate(*stmt.value) : Value(nullptr);
    throw ReturnSignal{val};
}

void Interpreter::visitLabel(LabelStmt& stmt) {
    currentLabel_ = stmt.name;
}

void Interpreter::visitGoto(GotoStmt& stmt) {
    throw GotoSignal{stmt.targetLabel};
}

void Interpreter::visitCallLabel(CallLabelStmt& stmt) {
    callStack_.push_back(pc_);
    throw GotoSignal{stmt.targetLabel};
}

void Interpreter::visitSay(SayStmt& stmt) {
    Value dialVal = evaluate(*stmt.dialogue);
    SayEvent ev;
    ev.speaker = stmt.actor;
    ev.text = dialVal.toString();
    ev.location = stmt.location;
    eventHandler_->onSay(ev);
}

void Interpreter::visitScene(SceneStmt& stmt) {
    Value sceneVal = evaluate(*stmt.sceneName);
    SceneEvent ev;
    ev.sceneName = sceneVal.toString();
    ev.transition = stmt.transition;
    ev.location = stmt.location;
    eventHandler_->onScene(ev);
}

void Interpreter::visitShow(ShowStmt& stmt) {
    ShowEvent ev;
    ev.actor = stmt.actor;
    ev.expression = stmt.expression;
    ev.position = stmt.position;
    ev.location = stmt.location;
    eventHandler_->onShow(ev);
}

void Interpreter::visitHide(HideStmt& stmt) {
    HideEvent ev;
    ev.actor = stmt.actor;
    ev.location = stmt.location;
    eventHandler_->onHide(ev);
}

void Interpreter::visitPlayAudio(PlayAudioStmt& stmt) {
    Value trackVal = evaluate(*stmt.track);
    AudioEvent ev;
    ev.channel = stmt.channel;
    ev.track = trackVal.toString();
    ev.fade = stmt.fade;
    ev.loop = stmt.loop;
    ev.location = stmt.location;
    eventHandler_->onAudio(ev);
}

void Interpreter::visitChoice(ChoiceStmt& stmt) {
    ChoiceEvent ev;
    ev.location = stmt.location;

    for (size_t i = 0; i < stmt.options.size(); ++i) {
        const auto& opt = stmt.options[i];
        Value textVal = evaluate(*opt.text);
        ChoiceItem item;
        item.index = static_cast<int>(i);
        item.text = textVal.toString();
        item.targetLabel = opt.targetLabel;
        ev.options.push_back(std::move(item));
    }

    int chosenIndex = eventHandler_->onChoice(ev);
    if (chosenIndex >= 0 && chosenIndex < static_cast<int>(stmt.options.size())) {
        const auto& chosen = stmt.options[chosenIndex];
        if (chosen.targetLabel) {
            throw GotoSignal{*chosen.targetLabel};
        } else if (chosen.body) {
            executeStmt(*chosen.body);
        }
    }
}

} // namespace goto_lang
