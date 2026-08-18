#include "../include/goto/sdk.hpp"
#include <cassert>

void registerTest(std::string name, std::function<void()> fn);

void testControlFlowAndFunctions() {
    goto_lang::Engine engine;

    std::string script = R"(
        let total = 0
        let i = 1
        while i <= 5
            total += i
            i += 1
        end

        fn dobro(x)
            return x * 2
        end

        let res = dobro(total)
    )";

    bool ok = engine.eval(script, "<test>");
    if (!ok) engine.printDiagnostics();
    assert(ok);

    assert(engine.getGlobal("total").asNumber() == 15);
    assert(engine.getGlobal("res").asNumber() == 30);
}

void testForInLoop() {
    goto_lang::Engine engine;

    std::string script = R"(
        let items = [10, 20, 30]
        let soma = 0
        for x in items
            soma += x
        end
    )";

    bool ok = engine.eval(script, "<test>");
    if (!ok) engine.printDiagnostics();
    assert(ok);

    assert(engine.getGlobal("soma").asNumber() == 60);
}

struct RegisterControlFlowTests {
    RegisterControlFlowTests() {
        registerTest("ControlFlow: While, For-In & Functions", testControlFlowAndFunctions);
        registerTest("ControlFlow: For-In Iteration", testForInLoop);
    }
} g_registerControlFlowTests;
