#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <unordered_map>
#include <variant>
#include <iostream>
#include <sstream>
#include <functional>
#include <optional>
#include <stdexcept>
#include <cstdint>
#include <utility>

namespace goto_lang {

struct SourceLocation {
    std::string file{"<anonymous>"};
    int line{1};
    int column{1};

    [[nodiscard]] std::string toString() const {
        return file + ":" + std::to_string(line) + ":" + std::to_string(column);
    }
};

enum class DiagnosticSeverity {
    Info,
    Warning,
    Error
};

struct Diagnostic {
    DiagnosticSeverity severity{DiagnosticSeverity::Error};
    SourceLocation location;
    std::string message;

    [[nodiscard]] std::string toString() const {
        std::string prefix;
        switch (severity) {
            case DiagnosticSeverity::Info:    prefix = "[INFO] "; break;
            case DiagnosticSeverity::Warning: prefix = "[WARNING] "; break;
            case DiagnosticSeverity::Error:   prefix = "[ERROR] "; break;
        }
        return prefix + location.toString() + " - " + message;
    }
};

class GoToException : public std::runtime_error {
public:
    GoToException(std::string message, SourceLocation loc = {})
        : std::runtime_error(loc.line > 0 ? (loc.toString() + ": " + message) : message),
          location_(std::move(loc)), rawMessage_(std::move(message)) {}

    [[nodiscard]] const SourceLocation& location() const noexcept { return location_; }
    [[nodiscard]] const std::string& rawMessage() const noexcept { return rawMessage_; }

private:
    SourceLocation location_;
    std::string rawMessage_;
};

} // namespace goto_lang
