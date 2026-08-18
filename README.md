# GoTo Language SDK (C++20)

O **GoTo SDK** é um conjunto completo de ferramentas, compilador/interpretador e biblioteca runtime em C++20 para a linguagem de script e narrativa **GoTo**. 

Projetada com foco em legibilidade para roteiristas e desenvolvedores de **Visual Novels** e jogos narrativos, a linguagem GoTo combina a simplicidade de sintaxe em blocos (estilo Ren'Py/Python/Lua) com controle explícito de fluxo via **`label`** e **`goto`**, tipagem dinâmica de alta performance e suporte nativo a estruturas de dados como listas e dicionários.

---

## ⚡ Comandos Rápidos no Terminal

Você pode executar tudo usando apenas o comando **`goto`** (ou `.\goto.bat` no PowerShell):

| Comando | O que faz |
| :--- | :--- |
| **`goto`** | Inicia o console interativo **REPL** da GoTo |
| **`goto game`** | Roda a demonstração de combate e lógica de jogo |
| **`goto demo`** | Roda a demonstração de **Visual Novel** interativa |
| **`goto test`** | Roda toda a suíte de testes unitários |
| **`goto build`** | Compila o SDK e o executável |
| **`goto check arquivo.goto`** | Valida a sintaxe de um script sem executar |
| **`goto meu_script.goto`** | Executa qualquer arquivo `.goto` |

---

## 🚀 Principais Recursos

- **Sintaxe Intuitiva em Blocos**: Blocos delimitados por `end` (`if ... end`, `while ... end`, `for ... in ... end`, `fn ... end`).
- **Comandos Narrativos Nativos**:
  - `scene "<cena>"` - Define o cenário/plano de fundo e transições.
  - `show <personagem> <expressão> [posição]` - Posiciona o sprite do personagem na tela.
  - `hide <personagem>` - Remove o personagem da cena.
  - `say <personagem> "<texto>"` ou `say "<texto>"` - Diálogos com speaker ou narrador.
  - `choice ... option "<texto>" goto <label> ... end` - Sistema de escolhas e ramificações.
  - `play music/sound "<arquivo>"` - Controle de trilha sonora e efeitos sonoros.
- **Navegação Não-Linear**: Suporte de primeira classe a `label <nome>` e `goto <nome>`, além de chamadas de subrotina `call <label>` e `return`.
- **Tipagem Dinâmica & Estruturas de Dados**:
  - Números, Strings, Booleanos (`true`, `false`), `nil`.
  - Listas dinâmicas: `[1, 2, "item"]`, indexação `lista[0]`, funções `push`, `pop`, `len`.
  - Dicionários chave-valor: `{"hp": 100, "nome": "Alice"}`, indexação `dict["chave"]`, funções `keys`, `values`, `has`, `remove`.
- **Arquitetura Embeddable em C++**: Fácil integração em qualquer engine de jogos (SDL, Raylib, SFML, Unreal Engine, Godot via GDExtension ou engine própria) através de callbacks (`IEngineEventHandler`).
- **CLI Completa & REPL**: Ferramenta de linha de comando `goto.exe` com checagem estática, visualização de tokens e modo interativo REPL.

---

## 📁 Estrutura do Repositório

```
SDK_GoTo/
├── CMakeLists.txt              # Configuração CMake do projeto (C++20)
├── build.ps1                   # Script de compilação automatizada (MSVC/CMake)
├── run.ps1                     # Script para executar scripts .goto rapidamente
├── README.md                   # Documentação completa
├── include/
│   └── goto/
│       ├── common.hpp          # Tipos comuns, diagnóstico e exceções
│       ├── value.hpp           # Sistema de valores dinâmicos
│       ├── token.hpp           # Enumeração de tokens e palavras-chave
│       ├── lexer.hpp           # Analisador léxico
│       ├── ast.hpp             # Árvore sintática abstrata (AST)
│       ├── parser.hpp          # Analisador sintático descendente recursivo
│       ├── environment.hpp     # Tabela de símbolos e escopo de variáveis
│       ├── engine_events.hpp   # Eventos de narrativa e callbacks para engines
│       ├── interpreter.hpp     # Interpretador e motor de execução
│       └── sdk.hpp             # Interface C++ simplificada da Engine
├── src/
│   ├── value.cpp
│   ├── token.cpp
│   ├── lexer.cpp
│   ├── ast.cpp
│   ├── parser.cpp
│   ├── environment.cpp
│   ├── interpreter.cpp
│   ├── sdk.cpp
│   └── main.cpp                # Ponto de entrada da CLI goto.exe
├── samples/
│   ├── visual_novel_demo.goto  # Demonstração completa de Visual Novel interativa
│   └── game_logic.goto         # Exemplo de lógica de combate, funções e dados
└── tests/
    ├── test_main.cpp           # Executor da suíte de testes
    ├── test_lexer.cpp
    ├── test_parser.cpp
    ├── test_values.cpp
    ├── test_control_flow.cpp
    └── test_narrative.cpp
```

---

## 🛠️ Como Compilar

### Opção 1: Usando o Script PowerShell (Recomendado)
No terminal na raiz do projeto:
```powershell
.\build.ps1 -RunTests
```

### Opção 2: Usando CMake Manualmente
```bash
cmake -B build
cmake --build build --config Release
```

---

## 🎮 Como Usar a CLI `goto`

### 1. Executar um Script
```bash
.\build\Debug\goto.exe samples\visual_novel_demo.goto
```
Ou com o helper `run.ps1`:
```powershell
.\run.ps1 samples\visual_novel_demo.goto
```

### 2. Verificar Sintaxe sem Executar
```bash
.\build\Debug\goto.exe --check samples\visual_novel_demo.goto
```

### 3. Modo Interativo REPL
```bash
.\build\Debug\goto.exe --repl
```

---

## 📖 Guia de Sintaxe da GoTo

### Diálogos e Cenas
```goto
scene "quarto" fade
show alice happy center

say alice "Bom dia! Tudo bem com você?"
say "O vento sopra lá fora..."
```

### Rótulos e Salto de Fluxo (`goto`)
```goto
label inicio
say "Você está diante de uma porta misteriosa."
goto abrir_porta

label abrir_porta
say "A porta se abre lentamente..."
```

### Menu de Escolhas (`choice`)
```goto
choice
    option "Entrar na floresta" goto rota_floresta
    option "Voltar para a cidade" goto rota_cidade
    option "Descansar aqui"
        say "Você descansou um pouco."
        goto rota_floresta
end
```

### Variáveis, Listas e Dicionários
```goto
let ouro = 100
let inventario = ["espada", "escudo", "poção"]
let status = {
    "vida": 100,
    "mana": 50
}

status["vida"] -= 20
push(inventario, "anel_magico")
```

### Funções e Estruturas de Repetição
```goto
fn calcular_dano(base, bonus)
    return base * 1.5 + bonus
end

let dano = calcular_dano(10, 5)

let i = 0
while i < 3
    println("Contagem: " + str(i))
    i += 1
end

for item in inventario
    println("Item: " + item)
end
```

---

## 🔌 Exemplo de Integração em C++

```cpp
#include "goto/sdk.hpp"
#include <iostream>

class MinhaGameEngine : public goto_lang::IEngineEventHandler {
public:
    void onSay(const goto_lang::SayEvent& ev) override {
        // Renderizar caixa de diálogo na UI da sua engine
        std::cout << (ev.speaker ? *ev.speaker : "Narrador") << ": " << ev.text << "\n";
    }

    void onScene(const goto_lang::SceneEvent& ev) override {
        // Carregar textura de fundo da cena
    }
};

int main() {
    auto handler = std::make_shared<MinhaGameEngine>();
    goto_lang::Engine engine(handler);

    if (engine.loadFile("script.goto")) {
        engine.run();
    }
    return 0;
}
```
