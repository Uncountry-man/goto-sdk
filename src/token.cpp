#include "../include/goto/token.hpp"

namespace goto_lang {

std::string_view tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::KwSay: return "say";
        case TokenType::KwScene: return "scene";
        case TokenType::KwShow: return "show";
        case TokenType::KwHide: return "hide";
        case TokenType::KwPlay: return "play";
        case TokenType::KwChoice: return "choice";
        case TokenType::KwOption: return "option";
        case TokenType::KwLabel: return "label";
        case TokenType::KwGoto: return "goto";
        case TokenType::KwCall: return "call";

        case TokenType::KwIf: return "if";
        case TokenType::KwElif: return "elif";
        case TokenType::KwElse: return "else";
        case TokenType::KwWhile: return "while";
        case TokenType::KwFor: return "for";
        case TokenType::KwIn: return "in";
        case TokenType::KwFn: return "fn";
        case TokenType::KwReturn: return "return";
        case TokenType::KwEnd: return "end";
        case TokenType::KwLet: return "let";
        case TokenType::KwVar: return "var";

        case TokenType::KwTrue: return "true";
        case TokenType::KwFalse: return "false";
        case TokenType::KwNil: return "nil";
        case TokenType::KwAnd: return "and";
        case TokenType::KwOr: return "or";
        case TokenType::KwNot: return "not";

        case TokenType::Identifier: return "Identifier";
        case TokenType::Number: return "Number";
        case TokenType::String: return "String";

        case TokenType::Plus: return "+";
        case TokenType::Minus: return "-";
        case TokenType::Star: return "*";
        case TokenType::Slash: return "/";
        case TokenType::Percent: return "%";
        case TokenType::Assign: return "=";
        case TokenType::PlusAssign: return "+=";
        case TokenType::MinusAssign: return "-=";
        case TokenType::StarAssign: return "*=";
        case TokenType::SlashAssign: return "/=";
        case TokenType::Equal: return "==";
        case TokenType::NotEqual: return "!=";
        case TokenType::Less: return "<";
        case TokenType::LessEqual: return "<=";
        case TokenType::Greater: return ">";
        case TokenType::GreaterEqual: return ">=";

        case TokenType::Colon: return ":";
        case TokenType::Comma: return ",";
        case TokenType::Dot: return ".";
        case TokenType::Arrow: return "->";
        case TokenType::LParen: return "(";
        case TokenType::RParen: return ")";
        case TokenType::LBracket: return "[";
        case TokenType::RBracket: return "]";
        case TokenType::LBrace: return "{";
        case TokenType::RBrace: return "}";

        case TokenType::Newline: return "Newline";
        case TokenType::EndOfFile: return "EOF";
    }
    return "Unknown";
}

std::string Token::toString() const {
    std::string res = "[";
    res += tokenTypeToString(type);
    res += " '" + lexeme + "'";
    if (!literal.isNil()) {
        res += " " + literal.toRepr();
    }
    res += " @" + location.toString() + "]";
    return res;
}

} // namespace goto_lang
