#pragma once

#include "common.hpp"
#include "token.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace goto_lang {

class Lexer {
public:
    explicit Lexer(std::string source, std::string filename = "<script>");

    [[nodiscard]] std::vector<Token> scanTokens();
    [[nodiscard]] const std::vector<Diagnostic>& diagnostics() const noexcept { return diagnostics_; }
    [[nodiscard]] bool hasErrors() const noexcept;

private:
    void scanToken();
    void identifier();
    void number();
    void string(char quoteChar);
    void blockComment();

    [[nodiscard]] bool isAtEnd() const noexcept;
    char advance();
    [[nodiscard]] char peek() const noexcept;
    [[nodiscard]] char peekNext() const noexcept;
    bool match(char expected);

    void addToken(TokenType type);
    void addToken(TokenType type, Value literal);
    void addError(const std::string& message);

    std::string source_;
    std::string filename_;
    std::vector<Token> tokens_;
    std::vector<Diagnostic> diagnostics_;

    size_t start_{0};
    size_t current_{0};
    int line_{1};
    int column_{1};
    int tokenStartColumn_{1};

    static const std::unordered_map<std::string, TokenType> keywords_;
};

} // namespace goto_lang
