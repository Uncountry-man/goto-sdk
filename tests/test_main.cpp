#include <iostream>
#include <vector>
#include <functional>
#include <string>

struct TestCase {
    std::string name;
    std::function<void()> fn;
};

std::vector<TestCase>& getTestRegistry() {
    static std::vector<TestCase> registry;
    return registry;
}

void registerTest(std::string name, std::function<void()> fn) {
    getTestRegistry().push_back(TestCase{std::move(name), std::move(fn)});
}

#define ASSERT_TRUE(cond) \
    if (!(cond)) { \
        std::cerr << "Assertion FAILED: " #cond " at " << __FILE__ << ":" << __LINE__ << "\n"; \
        throw std::runtime_error("Assertion failed: " #cond); \
    }

#define ASSERT_EQUAL(a, b) \
    if ((a) != (b)) { \
        std::cerr << "Assertion FAILED: " #a " == " #b " at " << __FILE__ << ":" << __LINE__ << "\n"; \
        throw std::runtime_error("Assertion failed: " #a " != " #b); \
    }

int main() {
    std::cout << "==========================================\n";
    std::cout << "       GoTo SDK - Suíte de Testes\n";
    std::cout << "==========================================\n\n";

    int passed = 0;
    int failed = 0;

    for (const auto& test : getTestRegistry()) {
        std::cout << "[EXECUTANDO] " << test.name << "... ";
        try {
            test.fn();
            std::cout << "PASSOU\n";
            passed++;
        } catch (const std::exception& e) {
            std::cout << "FALHOU (" << e.what() << ")\n";
            failed++;
        } catch (...) {
            std::cout << "FALHOU (Excecao desconhecida)\n";
            failed++;
        }
    }

    std::cout << "\n------------------------------------------\n";
    std::cout << "Total de testes: " << (passed + failed) << "\n";
    std::cout << "Passaram: " << passed << "\n";
    std::cout << "Falharam: " << failed << "\n";
    std::cout << "------------------------------------------\n";

    return (failed == 0) ? 0 : 1;
}
