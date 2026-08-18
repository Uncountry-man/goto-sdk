#pragma once

#include "common.hpp"
#include "token.hpp"
#include "ast.hpp"
#include <vector>
#include <memory>
#include <string>

namespace goto_lang {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens, std::string filename = "<script>");

    [[nodiscard]] std::vector<StmtPtr> parse();
    [[nodiscard]] const std::vector<Diagnostic>& diagnostics() const noexcept { return diagnostics_; }
    [[nodiscard]] bool hasErrors() const noexcept;

private:
    // Statement parsing
    StmtPtr declaration();
    StmtPtr varDeclaration();
    StmtPtr fnDeclaration();
    StmtPtr statement();
    StmtPtr ifStatement();
    StmtPtr whileStatement();
    StmtPtr forStatement();
    StmtPtr returnStatement();
    StmtPtr labelStatement();
    StmtPtr gotoStatement();
    StmtPtr callLabelStatement();
    StmtPtr sayStatement();
    StmtPtr sceneStatement();
    StmtPtr showStatement();
    StmtPtr hideStatement();
    StmtPtr playAudioStatement();
    StmtPtr choiceStatement();
    StmtPtr exprOrAssignStatement();
    std::vector<StmtPtr> blockUntil(const std::vector<TokenType>& terminators);

    // Expression parsing (Precedence Climbing)
    ExprPtr expression();
    ExprPtr assignment();
    ExprPtr logicalOr();
    ExprPtr logicalAnd();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr callOrSubscript();
    ExprPtr primary();

    // Helpers
    bool match(const std::vector<TokenType>& types);
    bool check(TokenType type) const;
    Token advance();
    [[nodiscard]] bool isAtEnd() const;
    Token peek() const;
    Token previous() const;
    Token consume(TokenType type, const std::string& message);
    void skipNewlines();
    void synchronize();
    void addError(const Token& token, const std::string& message);

    std::vector<Token> tokens_;
    std::string filename_;
    std::vector<Diagnostic> diagnostics_;
    size_t current_{0};
};

} // namespace goto_lang
