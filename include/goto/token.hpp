#pragma once

#include "common.hpp"
#include "value.hpp"
#include <string>
#include <string_view>

namespace goto_lang {

enum class TokenType {
    // Narrative Keywords
    KwSay,
    KwScene,
    KwShow,
    KwHide,
    KwPlay,
    KwChoice,
    KwOption,
    KwLabel,
    KwGoto,
    KwCall,

    // Control Flow & Structure Keywords
    KwIf,
    KwElif,
    KwElse,
    KwWhile,
    KwFor,
    KwIn,
    KwFn,
    KwReturn,
    KwEnd,
    KwLet,
    KwVar,

    // Logical & Value Keywords
    KwTrue,
    KwFalse,
    KwNil,
    KwAnd,
    KwOr,
    KwNot,

    // Literals & Identifiers
    Identifier,
    Number,
    String,

    // Operators
    Plus,           // +
    Minus,          // -
    Star,           // *
    Slash,          // /
    Percent,        // %
    Assign,         // =
    PlusAssign,     // +=
    MinusAssign,    // -=
    StarAssign,     // *=
    SlashAssign,    // /=
    Equal,          // ==
    NotEqual,       // !=
    Less,           // <
    LessEqual,      // <=
    Greater,        // >
    GreaterEqual,   // >=

    // Delimiters & Punctuation
    Colon,          // :
    Comma,          // ,
    Dot,            // .
    Arrow,          // ->
    LParen,         // (
    RParen,         // )
    LBracket,       // [
    RBracket,       // ]
    LBrace,         // {
    RBrace,         // }

    // Special
    Newline,
    EndOfFile
};

struct Token {
    TokenType type{TokenType::EndOfFile};
    std::string lexeme;
    Value literal{nullptr};
    SourceLocation location;

    [[nodiscard]] std::string toString() const;
};

std::string_view tokenTypeToString(TokenType type);

} // namespace goto_lang
