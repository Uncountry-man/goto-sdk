#pragma once

#include "common.hpp"
#include "value.hpp"
#include <string>
#include <vector>
#include <functional>
#include <optional>

namespace goto_lang {

struct SayEvent {
    std::optional<std::string> speaker;
    std::string text;
    SourceLocation location;
};

struct SceneEvent {
    std::string sceneName;
    std::string transition;
    SourceLocation location;
};

struct ShowEvent {
    std::string actor;
    std::string expression;
    std::string position;
    SourceLocation location;
};

struct HideEvent {
    std::string actor;
    SourceLocation location;
};

struct AudioEvent {
    std::string channel; // "music", "sound", "voice"
    std::string track;
    double fade{0.0};
    bool loop{true};
    SourceLocation location;
};

struct ChoiceItem {
    int index{0};
    std::string text;
    std::optional<std::string> targetLabel;
};

struct ChoiceEvent {
    std::vector<ChoiceItem> options;
    SourceLocation location;
};

class IEngineEventHandler {
public:
    virtual ~IEngineEventHandler() = default;

    virtual void onSay(const SayEvent& event) {
        if (event.speaker) {
            std::cout << "\n[" << *event.speaker << "]: " << event.text << "\n";
        } else {
            std::cout << "\n* " << event.text << " *\n";
        }
    }

    virtual void onScene(const SceneEvent& event) {
        std::cout << "\n--- [SCENE: " << event.sceneName << " (" << event.transition << ")] ---\n";
    }

    virtual void onShow(const ShowEvent& event) {
        std::cout << "  (Show " << event.actor << " [" << event.expression << "] at " << event.position << ")\n";
    }

    virtual void onHide(const HideEvent& event) {
        std::cout << "  (Hide " << event.actor << ")\n";
    }

    virtual void onAudio(const AudioEvent& event) {
        std::cout << "  (Audio " << event.channel << " -> '" << event.track << "')\n";
    }

    virtual int onChoice(const ChoiceEvent& event) {
        std::cout << "\n=== Escolhas ===\n";
        for (const auto& opt : event.options) {
            std::cout << " [" << (opt.index + 1) << "] " << opt.text << "\n";
        }
        std::cout << "Escolha uma opção: ";
        int choice = 1;
        std::string inputStr;
        if (std::getline(std::cin, inputStr)) {
            try {
                choice = std::stoi(inputStr);
            } catch (...) {
                choice = 1;
            }
        }
        if (choice < 1 || choice > static_cast<int>(event.options.size())) {
            choice = 1;
        }
        return choice - 1; // 0-indexed
    }
};

} // namespace goto_lang
