#pragma once

#include "common.hpp"
#include "value.hpp"
#include "token.hpp"
#include "lexer.hpp"
#include "ast.hpp"
#include "parser.hpp"
#include "environment.hpp"
#include "engine_events.hpp"
#include "interpreter.hpp"

namespace goto_lang {

class Engine {
public:
    Engine();
    explicit Engine(std::shared_ptr<IEngineEventHandler> eventHandler);

    bool loadFile(const std::string& filepath);
    bool eval(const std::string& source, const std::string& scriptName = "<string>");
    void run(const std::string& startLabel = "");

    void setEventHandler(std::shared_ptr<IEngineEventHandler> handler);
    [[nodiscard]] std::shared_ptr<IEngineEventHandler> getEventHandler() const noexcept;

    void setGlobal(const std::string& name, Value value);
    [[nodiscard]] Value getGlobal(const std::string& name) const;
    void registerNative(const std::string& name, NativeFn fn);

    [[nodiscard]] const std::vector<Diagnostic>& diagnostics() const noexcept { return diagnostics_; }
    [[nodiscard]] bool hasErrors() const noexcept;
    void printDiagnostics() const;

    [[nodiscard]] std::string saveState() const;
    void loadState(const std::string& json);

    [[nodiscard]] Interpreter& interpreter() noexcept { return *interpreter_; }

private:
    std::shared_ptr<Interpreter> interpreter_;
    std::vector<StmtPtr> loadedStatements_;
    std::vector<Diagnostic> diagnostics_;
};

} // namespace goto_lang
