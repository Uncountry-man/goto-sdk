#include "../include/goto/parser.hpp"
#include <algorithm>

namespace goto_lang {

Parser::Parser(std::vector<Token> tokens, std::string filename)
    : tokens_(std::move(tokens)), filename_(std::move(filename)) {}

bool Parser::hasErrors() const noexcept {
    for (const auto& diag : diagnostics_) {
        if (diag.severity == DiagnosticSeverity::Error) return true;
    }
    return false;
}

std::vector<StmtPtr> Parser::parse() {
    std::vector<StmtPtr> statements;
    diagnostics_.clear();
    current_ = 0;

    skipNewlines();
    while (!isAtEnd()) {
        try {
            StmtPtr stmt = declaration();
            if (stmt) {
                statements.push_back(stmt);
            }
        } catch (const GoToException& e) {
            diagnostics_.push_back(Diagnostic{
                DiagnosticSeverity::Error,
                e.location(),
                e.rawMessage()
            });
            synchronize();
        }
        skipNewlines();
    }
    return statements;
}

StmtPtr Parser::declaration() {
    skipNewlines();
    if (isAtEnd()) return nullptr;

    if (match({TokenType::KwLet, TokenType::KwVar})) {
        return varDeclaration();
    }
    if (match({TokenType::KwFn})) {
        return fnDeclaration();
    }
    return statement();
}

StmtPtr Parser::varDeclaration() {
    Token nameToken = consume(TokenType::Identifier, "Expected variable name after 'let'/'var'");
    ExprPtr initializer = nullptr;
    if (match({TokenType::Assign})) {
        initializer = expression();
    }
    return std::make_shared<VarDeclStmt>(nameToken.lexeme, initializer, nameToken.location);
}

StmtPtr Parser::fnDeclaration() {
    Token nameToken = consume(TokenType::Identifier, "Expected function name after 'fn'");
    consume(TokenType::LParen, "Expected '(' after function name");

    std::vector<std::string> params;
    if (!check(TokenType::RParen)) {
        do {
            Token p = consume(TokenType::Identifier, "Expected parameter name");
            params.push_back(p.lexeme);
        } while (match({TokenType::Comma}));
    }
    consume(TokenType::RParen, "Expected ')' after parameters");

    skipNewlines();
    std::vector<StmtPtr> body = blockUntil({TokenType::KwEnd});
    consume(TokenType::KwEnd, "Expected 'end' to close function '" + nameToken.lexeme + "'");

    return std::make_shared<FnDeclStmt>(
        nameToken.lexeme,
        std::move(params),
        std::make_shared<BlockStmt>(std::move(body), nameToken.location),
        nameToken.location
    );
}

StmtPtr Parser::statement() {
    if (match({TokenType::KwIf})) return ifStatement();
    if (match({TokenType::KwWhile})) return whileStatement();
    if (match({TokenType::KwFor})) return forStatement();
    if (match({TokenType::KwReturn})) return returnStatement();
    if (match({TokenType::KwLabel})) return labelStatement();
    if (match({TokenType::KwGoto})) return gotoStatement();
    if (match({TokenType::KwCall})) return callLabelStatement();
    if (match({TokenType::KwSay})) return sayStatement();
    if (match({TokenType::KwScene})) return sceneStatement();
    if (match({TokenType::KwShow})) return showStatement();
    if (match({TokenType::KwHide})) return hideStatement();
    if (match({TokenType::KwPlay})) return playAudioStatement();
    if (match({TokenType::KwChoice})) return choiceStatement();

    return exprOrAssignStatement();
}

StmtPtr Parser::ifStatement() {
    SourceLocation loc = previous().location;
    ExprPtr condition = expression();
    skipNewlines();

    std::vector<IfBranch> branches;
    std::vector<StmtPtr> ifBody = blockUntil({TokenType::KwElif, TokenType::KwElse, TokenType::KwEnd});
    branches.push_back(IfBranch{condition, std::make_shared<BlockStmt>(std::move(ifBody), loc)});

    while (match({TokenType::KwElif})) {
        SourceLocation elifLoc = previous().location;
        ExprPtr elifCond = expression();
        skipNewlines();
        std::vector<StmtPtr> elifBody = blockUntil({TokenType::KwElif, TokenType::KwElse, TokenType::KwEnd});
        branches.push_back(IfBranch{elifCond, std::make_shared<BlockStmt>(std::move(elifBody), elifLoc)});
    }

    StmtPtr elseBody = nullptr;
    if (match({TokenType::KwElse})) {
        SourceLocation elseLoc = previous().location;
        skipNewlines();
        std::vector<StmtPtr> elseStmts = blockUntil({TokenType::KwEnd});
        elseBody = std::make_shared<BlockStmt>(std::move(elseStmts), elseLoc);
    }

    consume(TokenType::KwEnd, "Expected 'end' to close 'if' block");
    return std::make_shared<IfStmt>(std::move(branches), elseBody, loc);
}

StmtPtr Parser::whileStatement() {
    SourceLocation loc = previous().location;
    ExprPtr condition = expression();
    skipNewlines();

    std::vector<StmtPtr> body = blockUntil({TokenType::KwEnd});
    consume(TokenType::KwEnd, "Expected 'end' to close 'while' loop");

    return std::make_shared<WhileStmt>(
        condition,
        std::make_shared<BlockStmt>(std::move(body), loc),
        loc
    );
}

StmtPtr Parser::forStatement() {
    SourceLocation loc = previous().location;
    Token varToken = consume(TokenType::Identifier, "Expected variable name after 'for'");
    consume(TokenType::KwIn, "Expected 'in' after variable in 'for' loop");
    ExprPtr iterable = expression();
    skipNewlines();

    std::vector<StmtPtr> body = blockUntil({TokenType::KwEnd});
    consume(TokenType::KwEnd, "Expected 'end' to close 'for' loop");

    return std::make_shared<ForInStmt>(
        varToken.lexeme,
        iterable,
        std::make_shared<BlockStmt>(std::move(body), loc),
        loc
    );
}

StmtPtr Parser::returnStatement() {
    SourceLocation loc = previous().location;
    ExprPtr value = nullptr;
    if (!check(TokenType::Newline) && !check(TokenType::EndOfFile) && !check(TokenType::KwEnd) && !check(TokenType::KwElif) && !check(TokenType::KwElse)) {
        value = expression();
    }
    return std::make_shared<ReturnStmt>(value, loc);
}

StmtPtr Parser::labelStatement() {
    SourceLocation loc = previous().location;
    Token name = consume(TokenType::Identifier, "Expected label identifier name");
    match({TokenType::Colon}); // Optional colon: 'label start:'
    return std::make_shared<LabelStmt>(name.lexeme, loc);
}

StmtPtr Parser::gotoStatement() {
    SourceLocation loc = previous().location;
    Token name = consume(TokenType::Identifier, "Expected target label identifier after 'goto'");
    return std::make_shared<GotoStmt>(name.lexeme, loc);
}

StmtPtr Parser::callLabelStatement() {
    SourceLocation loc = previous().location;
    Token name = consume(TokenType::Identifier, "Expected target label identifier after 'call'");
    return std::make_shared<CallLabelStmt>(name.lexeme, loc);
}

StmtPtr Parser::sayStatement() {
    SourceLocation loc = previous().location;
    std::optional<std::string> actor = std::nullopt;

    // Check if next token is actor identifier (and dialogue follows it)
    if (check(TokenType::Identifier)) {
        // Lookahead to see if next token is an expression or string
        TokenType nextType = (current_ + 1 < tokens_.size()) ? tokens_[current_ + 1].type : TokenType::EndOfFile;
        if (nextType == TokenType::String || nextType == TokenType::LParen || nextType == TokenType::Identifier || nextType == TokenType::Number) {
            Token actorTok = advance();
            actor = actorTok.lexeme;
        }
    }

    ExprPtr dialogue = expression();
    return std::make_shared<SayStmt>(actor, dialogue, loc);
}

StmtPtr Parser::sceneStatement() {
    SourceLocation loc = previous().location;
    ExprPtr sceneName = expression();
    std::string transition = "fade";
    if (check(TokenType::Identifier)) {
        transition = advance().lexeme;
    }
    return std::make_shared<SceneStmt>(sceneName, transition, loc);
}

StmtPtr Parser::showStatement() {
    SourceLocation loc = previous().location;
    Token actor = consume(TokenType::Identifier, "Expected character name after 'show'");
    std::string expression = "default";
    std::string position = "center";

    if (check(TokenType::Identifier) || check(TokenType::String)) {
        Token exprTok = advance();
        expression = exprTok.lexeme;
    }

    if (check(TokenType::Identifier) || check(TokenType::String)) {
        Token posTok = advance();
        position = posTok.lexeme;
    }

    return std::make_shared<ShowStmt>(actor.lexeme, expression, position, loc);
}

StmtPtr Parser::hideStatement() {
    SourceLocation loc = previous().location;
    Token actor = consume(TokenType::Identifier, "Expected character name after 'hide'");
    return std::make_shared<HideStmt>(actor.lexeme, loc);
}

StmtPtr Parser::playAudioStatement() {
    SourceLocation loc = previous().location;
    std::string channel = "music";
    if (check(TokenType::Identifier)) {
        channel = advance().lexeme;
    }
    ExprPtr track = expression();
    return std::make_shared<PlayAudioStmt>(channel, track, 0.0, true, loc);
}

StmtPtr Parser::choiceStatement() {
    SourceLocation loc = previous().location;
    skipNewlines();

    std::vector<ChoiceOption> options;

    while (match({TokenType::KwOption})) {
        SourceLocation optLoc = previous().location;
        ExprPtr optText = expression();
        std::optional<std::string> targetLabel = std::nullopt;
        StmtPtr body = nullptr;

        if (match({TokenType::KwGoto})) {
            Token target = consume(TokenType::Identifier, "Expected label after 'goto' in option");
            targetLabel = target.lexeme;
        } else {
            // Block of statements for this option until next option or end
            skipNewlines();
            std::vector<StmtPtr> optStmts = blockUntil({TokenType::KwOption, TokenType::KwEnd});
            if (!optStmts.empty()) {
                body = std::make_shared<BlockStmt>(std::move(optStmts), optLoc);
            }
        }
        options.push_back(ChoiceOption{optText, targetLabel, body});
        skipNewlines();
    }

    consume(TokenType::KwEnd, "Expected 'end' to close 'choice' block");
    return std::make_shared<ChoiceStmt>(std::move(options), loc);
}

StmtPtr Parser::exprOrAssignStatement() {
    SourceLocation loc = peek().location;
    ExprPtr expr = expression();
    return std::make_shared<ExprStmt>(expr, loc);
}

std::vector<StmtPtr> Parser::blockUntil(const std::vector<TokenType>& terminators) {
    std::vector<StmtPtr> statements;
    while (!isAtEnd()) {
        skipNewlines();
        if (isAtEnd()) break;
        bool isTerminator = false;
        for (TokenType term : terminators) {
            if (check(term)) {
                isTerminator = true;
                break;
            }
        }
        if (isTerminator) break;

        StmtPtr stmt = declaration();
        if (stmt) {
            statements.push_back(stmt);
        }
    }
    return statements;
}

// Expressions
ExprPtr Parser::expression() {
    return assignment();
}

ExprPtr Parser::assignment() {
    ExprPtr expr = logicalOr();

    if (match({TokenType::Assign, TokenType::PlusAssign, TokenType::MinusAssign, TokenType::StarAssign, TokenType::SlashAssign})) {
        Token opToken = previous();
        ExprPtr value = assignment();

        if (auto varExpr = std::dynamic_pointer_cast<VariableExpr>(expr)) {
            return std::make_shared<AssignExpr>(varExpr->name, opToken.type, value, opToken.location);
        }
        if (auto idxExpr = std::dynamic_pointer_cast<IndexExpr>(expr)) {
            return std::make_shared<IndexAssignExpr>(idxExpr->target, idxExpr->index, opToken.type, value, opToken.location);
        }
        throw GoToException("Invalid assignment target", opToken.location);
    }
    return expr;
}

ExprPtr Parser::logicalOr() {
    ExprPtr expr = logicalAnd();
    while (match({TokenType::KwOr})) {
        TokenType op = previous().type;
        SourceLocation loc = previous().location;
        ExprPtr right = logicalAnd();
        expr = std::make_shared<LogicalExpr>(expr, op, right, loc);
    }
    return expr;
}

ExprPtr Parser::logicalAnd() {
    ExprPtr expr = equality();
    while (match({TokenType::KwAnd})) {
        TokenType op = previous().type;
        SourceLocation loc = previous().location;
        ExprPtr right = equality();
        expr = std::make_shared<LogicalExpr>(expr, op, right, loc);
    }
    return expr;
}

ExprPtr Parser::equality() {
    ExprPtr expr = comparison();
    while (match({TokenType::Equal, TokenType::NotEqual})) {
        TokenType op = previous().type;
        SourceLocation loc = previous().location;
        ExprPtr right = comparison();
        expr = std::make_shared<BinaryExpr>(expr, op, right, loc);
    }
    return expr;
}

ExprPtr Parser::comparison() {
    ExprPtr expr = term();
    while (match({TokenType::Greater, TokenType::GreaterEqual, TokenType::Less, TokenType::LessEqual})) {
        TokenType op = previous().type;
        SourceLocation loc = previous().location;
        ExprPtr right = term();
        expr = std::make_shared<BinaryExpr>(expr, op, right, loc);
    }
    return expr;
}

ExprPtr Parser::term() {
    ExprPtr expr = factor();
    while (match({TokenType::Plus, TokenType::Minus})) {
        TokenType op = previous().type;
        SourceLocation loc = previous().location;
        ExprPtr right = factor();
        expr = std::make_shared<BinaryExpr>(expr, op, right, loc);
    }
    return expr;
}

ExprPtr Parser::factor() {
    ExprPtr expr = unary();
    while (match({TokenType::Star, TokenType::Slash, TokenType::Percent})) {
        TokenType op = previous().type;
        SourceLocation loc = previous().location;
        ExprPtr right = unary();
        expr = std::make_shared<BinaryExpr>(expr, op, right, loc);
    }
    return expr;
}

ExprPtr Parser::unary() {
    if (match({TokenType::KwNot, TokenType::Minus})) {
        TokenType op = previous().type;
        SourceLocation loc = previous().location;
        ExprPtr right = unary();
        return std::make_shared<UnaryExpr>(op, right, loc);
    }
    return callOrSubscript();
}

ExprPtr Parser::callOrSubscript() {
    ExprPtr expr = primary();

    while (true) {
        if (match({TokenType::LParen})) {
            SourceLocation loc = previous().location;
            std::vector<ExprPtr> args;
            if (!check(TokenType::RParen)) {
                do {
                    args.push_back(expression());
                } while (match({TokenType::Comma}));
            }
            consume(TokenType::RParen, "Expected ')' after arguments");
            expr = std::make_shared<CallExpr>(expr, std::move(args), loc);
        } else if (match({TokenType::LBracket})) {
            SourceLocation loc = previous().location;
            ExprPtr index = expression();
            consume(TokenType::RBracket, "Expected ']' after index");
            expr = std::make_shared<IndexExpr>(expr, index, loc);
        } else {
            break;
        }
    }
    return expr;
}

ExprPtr Parser::primary() {
    if (match({TokenType::KwNil})) return std::make_shared<LiteralExpr>(Value(nullptr), previous().location);
    if (match({TokenType::KwTrue})) return std::make_shared<LiteralExpr>(Value(true), previous().location);
    if (match({TokenType::KwFalse})) return std::make_shared<LiteralExpr>(Value(false), previous().location);

    if (match({TokenType::Number, TokenType::String})) {
        return std::make_shared<LiteralExpr>(previous().literal, previous().location);
    }

    if (match({TokenType::Identifier})) {
        return std::make_shared<VariableExpr>(previous().lexeme, previous().location);
    }

    // List literal: [1, 2, 3]
    if (match({TokenType::LBracket})) {
        SourceLocation loc = previous().location;
        std::vector<ExprPtr> elements;
        skipNewlines();
        if (!check(TokenType::RBracket)) {
            do {
                skipNewlines();
                elements.push_back(expression());
                skipNewlines();
            } while (match({TokenType::Comma}));
        }
        skipNewlines();
        consume(TokenType::RBracket, "Expected ']' to close list");
        return std::make_shared<ListExpr>(std::move(elements), loc);
    }

    // Dict literal: {"key": value, "key2": value2}
    if (match({TokenType::LBrace})) {
        SourceLocation loc = previous().location;
        std::vector<std::pair<ExprPtr, ExprPtr>> entries;
        skipNewlines();
        if (!check(TokenType::RBrace)) {
            do {
                skipNewlines();
                ExprPtr key = expression();
                consume(TokenType::Colon, "Expected ':' after dictionary key");
                ExprPtr val = expression();
                entries.push_back({key, val});
                skipNewlines();
            } while (match({TokenType::Comma}));
        }
        skipNewlines();
        consume(TokenType::RBrace, "Expected '}' to close dictionary");
        return std::make_shared<DictExpr>(std::move(entries), loc);
    }

    // Parenthesized expression: (expr)
    if (match({TokenType::LParen})) {
        ExprPtr expr = expression();
        consume(TokenType::RParen, "Expected ')' after expression");
        return expr;
    }

    Token tok = peek();
    throw GoToException("Unexpected token '" + tok.lexeme + "' in expression", tok.location);
}

// Helpers
bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

Token Parser::advance() {
    if (!isAtEnd()) current_++;
    return previous();
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EndOfFile;
}

Token Parser::peek() const {
    return tokens_[current_];
}

Token Parser::previous() const {
    return tokens_[current_ - 1];
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw GoToException(message, peek().location);
}

void Parser::skipNewlines() {
    while (!isAtEnd() && peek().type == TokenType::Newline) {
        advance();
    }
}

void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::Newline) return;
        switch (peek().type) {
            case TokenType::KwFn:
            case TokenType::KwVar:
            case TokenType::KwLet:
            case TokenType::KwFor:
            case TokenType::KwIf:
            case TokenType::KwWhile:
            case TokenType::KwReturn:
            case TokenType::KwSay:
            case TokenType::KwScene:
            case TokenType::KwShow:
            case TokenType::KwHide:
            case TokenType::KwLabel:
            case TokenType::KwGoto:
            case TokenType::KwChoice:
                return;
            default:
                break;
        }
        advance();
    }
}

void Parser::addError(const Token& token, const std::string& message) {
    diagnostics_.push_back(Diagnostic{
        DiagnosticSeverity::Error,
        token.location,
        message
    });
}

} // namespace goto_lang
