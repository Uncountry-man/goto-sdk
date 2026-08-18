#include "../include/goto/lexer.hpp"
#include "../include/goto/parser.hpp"
#include <cassert>

void registerTest(std::string name, std::function<void()> fn);

void testParserBasicStatements() {
    std::string source = R"(
        let x = 10
        let name = "Alice"
        scene "castelo"
        show alice neutral left
        say alice "Bem-vindo!"
        label inicio
        goto castelo
    )";

    goto_lang::Lexer lexer(source, "<test>");
    auto tokens = lexer.scanTokens();
    assert(!lexer.hasErrors());

    goto_lang::Parser parser(tokens, "<test>");
    auto stmts = parser.parse();
    assert(!parser.hasErrors());
    assert(stmts.size() == 7);
}

void testParserControlFlowAndFunctions() {
    std::string source = R"(
        fn somar(a, b)
            return a + b
        end

        if x > 10
            let y = 1
        elif x == 10
            let y = 2
        else
            let y = 3
        end

        while y > 0
            y -= 1
        end
    )";

    goto_lang::Lexer lexer(source, "<test>");
    auto tokens = lexer.scanTokens();
    assert(!lexer.hasErrors());

    goto_lang::Parser parser(tokens, "<test>");
    auto stmts = parser.parse();
    assert(!parser.hasErrors());
    assert(stmts.size() == 3);
}

struct RegisterParserTests {
    RegisterParserTests() {
        registerTest("Parser: Basic Statements", testParserBasicStatements);
        registerTest("Parser: Control Flow & Functions", testParserControlFlowAndFunctions);
    }
} g_registerParserTests;
