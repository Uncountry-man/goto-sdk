#include "../include/goto/lexer.hpp"
#include <iostream>
#include <cassert>

void registerTest(std::string name, std::function<void()> fn);

void testLexerKeywordsAndTokens() {
    std::string source = R"(
        scene "quarto"
        show alice happy
        say alice "Ola mundo"
        let hp = 100
        if hp > 50
            goto floresta
        end
    )";

    goto_lang::Lexer lexer(source, "<test>");
    auto tokens = lexer.scanTokens();

    assert(!lexer.hasErrors());
    assert(tokens.size() > 5);
}

void testLexerOperatorsAndComments() {
    std::string source = R"(
        # Single line comment
        let x = 10 + 20 * 3
        // Another comment
        /* Multi-line
           comment */
        x += 5
        let flag = (x == 35) and (not false)
    )";

    goto_lang::Lexer lexer(source, "<test>");
    auto tokens = lexer.scanTokens();

    assert(!lexer.hasErrors());
}

struct RegisterLexerTests {
    RegisterLexerTests() {
        registerTest("Lexer: Keywords & Tokens", testLexerKeywordsAndTokens);
        registerTest("Lexer: Operators & Comments", testLexerOperatorsAndComments);
    }
} g_registerLexerTests;
