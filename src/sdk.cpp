#include "../include/goto/sdk.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

namespace goto_lang {

Engine::Engine()
    : interpreter_(std::make_shared<Interpreter>()) {}

Engine::Engine(std::shared_ptr<IEngineEventHandler> eventHandler)
    : interpreter_(std::make_shared<Interpreter>(std::move(eventHandler))) {}

void Engine::setEventHandler(std::shared_ptr<IEngineEventHandler> handler) {
    interpreter_->setEventHandler(std::move(handler));
}

std::shared_ptr<IEngineEventHandler> Engine::getEventHandler() const noexcept {
    return interpreter_->getEventHandler();
}

void Engine::setGlobal(const std::string& name, Value value) {
    interpreter_->setGlobal(name, std::move(value));
}

Value Engine::getGlobal(const std::string& name) const {
    return interpreter_->getGlobal(name);
}

void Engine::registerNative(const std::string& name, NativeFn fn) {
    interpreter_->setGlobal(name, Value(std::move(fn), name));
}

bool Engine::hasErrors() const noexcept {
    for (const auto& diag : diagnostics_) {
        if (diag.severity == DiagnosticSeverity::Error) return true;
    }
    return false;
}

void Engine::printDiagnostics() const {
    for (const auto& diag : diagnostics_) {
        std::cerr << diag.toString() << "\n";
    }
}

bool Engine::loadFile(const std::string& filepath) {
    diagnostics_.clear();
    std::ifstream file(filepath);
    if (!file.is_open()) {
        diagnostics_.push_back(Diagnostic{
            DiagnosticSeverity::Error,
            SourceLocation{filepath, 1, 1},
            "Could not open file: " + filepath
        });
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    Lexer lexer(source, filepath);
    std::vector<Token> tokens = lexer.scanTokens();
    if (lexer.hasErrors()) {
        diagnostics_ = lexer.diagnostics();
        return false;
    }

    Parser parser(tokens, filepath);
    loadedStatements_ = parser.parse();
    if (parser.hasErrors()) {
        diagnostics_ = parser.diagnostics();
        return false;
    }

    return true;
}

bool Engine::eval(const std::string& source, const std::string& scriptName) {
    diagnostics_.clear();
    Lexer lexer(source, scriptName);
    std::vector<Token> tokens = lexer.scanTokens();
    if (lexer.hasErrors()) {
        diagnostics_ = lexer.diagnostics();
        return false;
    }

    Parser parser(tokens, scriptName);
    std::vector<StmtPtr> statements = parser.parse();
    if (parser.hasErrors()) {
        diagnostics_ = parser.diagnostics();
        return false;
    }

    try {
        interpreter_->execute(statements);
    } catch (const GoToException& e) {
        diagnostics_.push_back(Diagnostic{
            DiagnosticSeverity::Error,
            e.location(),
            e.rawMessage()
        });
        return false;
    }

    return true;
}

void Engine::run(const std::string& startLabel) {
    if (loadedStatements_.empty()) return;
    try {
        interpreter_->executeScript(loadedStatements_, startLabel);
    } catch (const GoToException& e) {
        diagnostics_.push_back(Diagnostic{
            DiagnosticSeverity::Error,
            e.location(),
            e.rawMessage()
        });
        printDiagnostics();
    }
}

std::string Engine::saveState() const {
    return interpreter_->exportStateJson();
}

void Engine::loadState(const std::string& json) {
    interpreter_->importStateJson(json);
}

} // namespace goto_lang
