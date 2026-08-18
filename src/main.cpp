#include "../include/goto/sdk.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

void printUsage() {
    std::cout << "GoTo Language SDK - CLI Tool (v1.0.0)\n";
    std::cout << "Uso: goto [opcoes] [arquivo.goto]\n\n";
    std::cout << "Opcoes:\n";
    std::cout << "  -h, --help        Mostra esta mensagem de ajuda\n";
    std::cout << "  -v, --version     Mostra a versao da linguagem GoTo\n";
    std::cout << "  -c, --check       Verifica erros de sintaxe sem executar\n";
    std::cout << "  -t, --tokens      Exibe o fluxo de tokens gerado pelo Lexer\n";
    std::cout << "  -r, --repl        Inicia o console interativo REPL\n";
    std::cout << "\nExemplo:\n";
    std::cout << "  goto samples/visual_novel_demo.goto\n";
}

void runRepl() {
    std::cout << "==========================================\n";
    std::cout << "   GoTo Interactive REPL (v1.0.0)\n";
    std::cout << "   Digite 'exit' ou 'quit' para sair\n";
    std::cout << "==========================================\n\n";

    goto_lang::Engine engine;
    std::string line;

    while (true) {
        std::cout << "goto> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;
        if (line.empty()) continue;

        if (!engine.eval(line, "<repl>")) {
            engine.printDiagnostics();
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        runRepl();
        return 0;
    }

    std::string firstArg = argv[1];
    if (firstArg == "-h" || firstArg == "--help") {
        printUsage();
        return 0;
    }

    if (firstArg == "-v" || firstArg == "--version") {
        std::cout << "GoTo Language SDK & Runtime v1.0.0 (C++20)\n";
        return 0;
    }

    if (firstArg == "-r" || firstArg == "--repl") {
        runRepl();
        return 0;
    }

    if (firstArg == "-t" || firstArg == "--tokens") {
        if (argc < 3) {
            std::cerr << "Erro: Nenhum arquivo especificado para exibir tokens.\n";
            return 1;
        }
        std::string filename = argv[2];
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Erro: Nao foi possivel abrir o arquivo " << filename << "\n";
            return 1;
        }
        std::stringstream ss;
        ss << file.rdbuf();
        goto_lang::Lexer lexer(ss.str(), filename);
        auto tokens = lexer.scanTokens();
        std::cout << "--- Tokens (" << filename << ") ---\n";
        for (const auto& tok : tokens) {
            std::cout << tok.toString() << "\n";
        }
        if (lexer.hasErrors()) {
            for (const auto& diag : lexer.diagnostics()) {
                std::cerr << diag.toString() << "\n";
            }
            return 1;
        }
        return 0;
    }

    if (firstArg == "-c" || firstArg == "--check") {
        if (argc < 3) {
            std::cerr << "Erro: Nenhum arquivo especificado para checagem.\n";
            return 1;
        }
        std::string filename = argv[2];
        goto_lang::Engine engine;
        if (!engine.loadFile(filename)) {
            engine.printDiagnostics();
            std::cerr << "\n[FALHA] Foram encontrados erros de sintaxe no arquivo.\n";
            return 1;
        }
        std::cout << "[SUCESSO] Sintaxe do arquivo '" << filename << "' verificada com sucesso!\n";
        return 0;
    }

    std::string scriptPath = firstArg;
    goto_lang::Engine engine;
    if (!engine.loadFile(scriptPath)) {
        engine.printDiagnostics();
        return 1;
    }

    engine.run();
    return 0;
}
