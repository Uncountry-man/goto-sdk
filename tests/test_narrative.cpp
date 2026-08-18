#include "../include/goto/sdk.hpp"
#include <cassert>

void registerTest(std::string name, std::function<void()> fn);

class MockEventHandler : public goto_lang::IEngineEventHandler {
public:
    std::vector<std::string> says;
    std::vector<std::string> scenes;
    std::vector<std::string> shows;

    void onSay(const goto_lang::SayEvent& event) override {
        says.push_back((event.speaker ? (*event.speaker + ": ") : "") + event.text);
    }

    void onScene(const goto_lang::SceneEvent& event) override {
        scenes.push_back(event.sceneName);
    }

    void onShow(const goto_lang::ShowEvent& event) override {
        shows.push_back(event.actor + "_" + event.expression);
    }

    int onChoice(const goto_lang::ChoiceEvent&) override {
        return 0; // Select first option by default
    }
};

void testNarrativeCommandsAndGoto() {
    auto handler = std::make_shared<MockEventHandler>();
    goto_lang::Engine engine(handler);

    std::string script = R"(
        scene "quarto"
        show alice happy
        say alice "Ola!"

        goto destino

        say alice "Isto nunca deve ser dito"

        label destino
        scene "jardim"
        say "Voce chegou ao jardim."
    )";

    bool ok = engine.eval(script, "<test_narrative>");
    if (!ok) engine.printDiagnostics();
    assert(ok);

    assert(handler->scenes.size() == 2);
    assert(handler->scenes[0] == "quarto");
    assert(handler->scenes[1] == "jardim");

    assert(handler->says.size() == 2);
    assert(handler->says[0] == "alice: Ola!");
    assert(handler->says[1] == "Voce chegou ao jardim.");
}

struct RegisterNarrativeTests {
    RegisterNarrativeTests() {
        registerTest("Narrative: Scene, Show, Say & Goto Jumps", testNarrativeCommandsAndGoto);
    }
} g_registerNarrativeTests;
