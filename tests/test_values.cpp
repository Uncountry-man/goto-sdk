#include "../include/goto/value.hpp"
#include <cassert>

void registerTest(std::string name, std::function<void()> fn);

void testValueOperations() {
    // Number arithmetic
    goto_lang::Value a(10.0);
    goto_lang::Value b(5.0);
    assert((a + b).asNumber() == 15.0);
    assert((a - b).asNumber() == 5.0);
    assert((a * b).asNumber() == 50.0);
    assert((a / b).asNumber() == 2.0);

    // String concatenation
    goto_lang::Value str1("Ola ");
    goto_lang::Value str2("Mundo");
    assert((str1 + str2).asString() == "Ola Mundo");

    // List operations
    auto listVal = goto_lang::Value::makeList({goto_lang::Value(1), goto_lang::Value(2), goto_lang::Value("tres")});
    assert(listVal.isList());
    assert(listVal.getSubscript(goto_lang::Value(0)).asNumber() == 1);
    assert(listVal.getSubscript(goto_lang::Value(2)).asString() == "tres");

    // Dict operations
    auto dictVal = goto_lang::Value::makeDict({
        {"nome", goto_lang::Value("Alice")},
        {"hp", goto_lang::Value(100)}
    });
    assert(dictVal.isDict());
    assert(dictVal.getSubscript(goto_lang::Value("nome")).asString() == "Alice");
    assert(dictVal.getSubscript(goto_lang::Value("hp")).asNumber() == 100);
}

struct RegisterValueTests {
    RegisterValueTests() {
        registerTest("Values: Dynamic Types, Lists & Dicts", testValueOperations);
    }
} g_registerValueTests;
