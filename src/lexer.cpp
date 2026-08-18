#include "../include/goto/lexer.hpp"
#include <cctype>
#include <sstream>

namespace goto_lang {

const std::unordered_map<std::string, TokenType> Lexer::keywords_ = {
    {"say",     TokenType::KwSay},
    {"scene",   TokenType::KwScene},
    {"show",    TokenType::KwShow},
    {"hide",    TokenType::KwHide},
    {"play",    TokenType::KwPlay},
    {"choice",  TokenType::KwChoice},
    {"option",  TokenType::KwOption},
    {"label",   TokenType::KwLabel},
    {"goto",    TokenType::KwGoto},
    {"call",    TokenType::KwCall},

    {"if",      TokenType::KwIf},
    {"elif",    TokenType::KwElif},
    {"else",    TokenType::KwElse},
    {"while",   TokenType::KwWhile},
    {"for",     TokenType::KwFor},
    {"in",      TokenType::KwIn},
    {"fn",      TokenType::KwFn},
    {"return",  TokenType::KwReturn},
    {"end",     TokenType::KwEnd},
    {"let",     TokenType::KwLet},
    {"var",     TokenType::KwVar},

    {"true",    TokenType::KwTrue},
    {"false",   TokenType::KwFalse},
    {"nil",     TokenType::KwNil},
    {"null",    TokenType::KwNil},
    {"and",     TokenType::KwAnd},
    {"or",      TokenType::KwOr},
    {"not",     TokenType::KwNot}
};

Lexer::Lexer(std::string source, std::string filename)
    : source_(std::move(source)), filename_(std::move(filename)) {}

bool Lexer::hasErrors() const noexcept {
    for (const auto& diag : diagnostics_) {
        if (diag.severity == DiagnosticSeverity::Error) return true;
    }
    return false;
}

std::vector<Token> Lexer::scanTokens() {
    tokens_.clear();
    diagnostics_.clear();
    start_ = 0;
    current_ = 0;
    line_ = 1;
    column_ = 1;

    while (!isAtEnd()) {
        start_ = current_;
        tokenStartColumn_ = column_;
        scanToken();
    }

    // Append Newline if needed before EOF
    if (!tokens_.empty() && tokens_.back().type != TokenType::Newline) {
        tokens_.push_back(Token{TokenType::Newline, "\n", Value(nullptr), SourceLocation{filename_, line_, column_}});
    }

    tokens_.push_back(Token{TokenType::EndOfFile, "", Value(nullptr), SourceLocation{filename_, line_, column_}});
    return tokens_;
}

bool Lexer::isAtEnd() const noexcept {
    return current_ >= source_.size();
}

char Lexer::advance() {
    char c = source_[current_++];
    column_++;
    return c;
}

char Lexer::peek() const noexcept {
    if (isAtEnd()) return '\0';
    return source_[current_];
}

char Lexer::peekNext() const noexcept {
    if (current_ + 1 >= source_.size()) return '\0';
    return source_[current_ + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source_[current_] != expected) return false;
    current_++;
    column_++;
    return true;
}

void Lexer::addToken(TokenType type) {
    addToken(type, Value(nullptr));
}

void Lexer::addToken(TokenType type, Value literal) {
    std::string text = source_.substr(start_, current_ - start_);
    tokens_.push_back(Token{type, std::move(text), std::move(literal), SourceLocation{filename_, line_, tokenStartColumn_}});
}

void Lexer::addError(const std::string& message) {
    diagnostics_.push_back(Diagnostic{
        DiagnosticSeverity::Error,
        SourceLocation{filename_, line_, tokenStartColumn_},
        message
    });
}

void Lexer::scanToken() {
    char c = advance();
    switch (c) {
        // Single character tokens
        case '(': addToken(TokenType::LParen); break;
        case ')': addToken(TokenType::RParen); break;
        case '[': addToken(TokenType::LBracket); break;
        case ']': addToken(TokenType::RBracket); break;
        case '{': addToken(TokenType::LBrace); break;
        case '}': addToken(TokenType::RBrace); break;
        case ',': addToken(TokenType::Comma); break;
        case '.': addToken(TokenType::Dot); break;
        case ':': addToken(TokenType::Colon); break;
        case '%': addToken(TokenType::Percent); break;

        // One or two character operators
        case '+':
            addToken(match('=') ? TokenType::PlusAssign : TokenType::Plus);
            break;
        case '-':
            if (match('>')) {
                addToken(TokenType::Arrow);
            } else if (match('=')) {
                addToken(TokenType::MinusAssign);
            } else {
                addToken(TokenType::Minus);
            }
            break;
        case '*':
            addToken(match('=') ? TokenType::StarAssign : TokenType::Star);
            break;
        case '/':
            if (match('/')) {
                // Single line comment //
                while (peek() != '\n' && !isAtEnd()) advance();
            } else if (match('*')) {
                blockComment();
            } else if (match('=')) {
                addToken(TokenType::SlashAssign);
            } else {
                addToken(TokenType::Slash);
            }
            break;
        case '#':
            // Single line comment #
            while (peek() != '\n' && !isAtEnd()) advance();
            break;
        case '=':
            addToken(match('=') ? TokenType::Equal : TokenType::Assign);
            break;
        case '!':
            if (match('=')) {
                addToken(TokenType::NotEqual);
            } else {
                addError("Unexpected character '!' (did you mean 'not' or '!= '?)");
            }
            break;
        case '<':
            addToken(match('=') ? TokenType::LessEqual : TokenType::Less);
            break;
        case '>':
            addToken(match('=') ? TokenType::GreaterEqual : TokenType::Greater);
            break;

        // Whitespace and newlines
        case ' ':
        case '\r':
        case '\t':
            // Ignore whitespace
            break;

        case ';':
            // Semicolon acts like newline separator
            if (!tokens_.empty() && tokens_.back().type != TokenType::Newline) {
                addToken(TokenType::Newline);
            }
            break;

        case '\n':
            line_++;
            column_ = 1;
            // Prevent consecutive redundant newlines
            if (!tokens_.empty() && tokens_.back().type != TokenType::Newline) {
                addToken(TokenType::Newline);
            }
            break;

        // Literals
        case '"':
        case '\'':
            string(c);
            break;

        default:
            if (std::isdigit(static_cast<unsigned char>(c))) {
                number();
            } else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                identifier();
            } else {
                addError("Unexpected character '" + std::string(1, c) + "'");
            }
            break;
    }
}

void Lexer::blockComment() {
    int depth = 1;
    while (depth > 0 && !isAtEnd()) {
        if (peek() == '/' && peekNext() == '*') {
            advance();
            advance();
            depth++;
        } else if (peek() == '*' && peekNext() == '/') {
            advance();
            advance();
            depth--;
        } else {
            if (peek() == '\n') {
                line_++;
                column_ = 0;
            }
            advance();
        }
    }
    if (depth > 0) {
        addError("Unterminated block comment");
    }
}

void Lexer::string(char quoteChar) {
    std::string value;
    while (peek() != quoteChar && !isAtEnd()) {
        if (peek() == '\n') {
            line_++;
            column_ = 0;
        }
        if (peek() == '\\') {
            advance(); // Skip '\\'
            if (isAtEnd()) break;
            char esc = advance();
            switch (esc) {
                case 'n': value.push_back('\n'); break;
                case 't': value.push_back('\t'); break;
                case 'r': value.push_back('\r'); break;
                case '\\': value.push_back('\\'); break;
                case '"': value.push_back('"'); break;
                case '\'': value.push_back('\''); break;
                default:
                    value.push_back(esc);
                    break;
            }
        } else {
            value.push_back(advance());
        }
    }

    if (isAtEnd()) {
        addError("Unterminated string literal");
        return;
    }

    // Closing quote
    advance();
    addToken(TokenType::String, Value(value));
}

void Lexer::number() {
    while (std::isdigit(static_cast<unsigned char>(peek()))) advance();

    // Look for fractional part
    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peekNext()))) {
        advance(); // consume '.'
        while (std::isdigit(static_cast<unsigned char>(peek()))) advance();
    }

    std::string text = source_.substr(start_, current_ - start_);
    try {
        double val = std::stod(text);
        addToken(TokenType::Number, Value(val));
    } catch (...) {
        addError("Invalid number format: " + text);
    }
}

void Lexer::identifier() {
    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') advance();

    std::string text = source_.substr(start_, current_ - start_);
    auto it = keywords_.find(text);
    if (it != keywords_.end()) {
        TokenType type = it->second;
        if (type == TokenType::KwTrue) {
            addToken(type, Value(true));
        } else if (type == TokenType::KwFalse) {
            addToken(type, Value(false));
        } else if (type == TokenType::KwNil) {
            addToken(type, Value(nullptr));
        } else {
            addToken(type);
        }
    } else {
        addToken(TokenType::Identifier);
    }
}

} // namespace goto_lang
