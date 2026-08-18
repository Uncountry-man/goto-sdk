#pragma once

#include "common.hpp"
#include "value.hpp"
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

namespace goto_lang {

class Environment : public std::enable_shared_from_this<Environment> {
public:
    Environment();
    explicit Environment(std::shared_ptr<Environment> enclosing);

    void define(const std::string& name, Value value);
    bool assign(const std::string& name, const Value& value);
    [[nodiscard]] Value get(const std::string& name) const;
    [[nodiscard]] bool has(const std::string& name) const;

    [[nodiscard]] std::shared_ptr<Environment> enclosing() const noexcept { return enclosing_; }
    [[nodiscard]] const std::unordered_map<std::string, Value>& localVariables() const noexcept { return values_; }

    [[nodiscard]] std::unordered_map<std::string, Value> getAllVariables() const;
    void loadVariables(const std::unordered_map<std::string, Value>& vars);

    void registerBuiltins();

private:
    std::shared_ptr<Environment> enclosing_{nullptr};
    std::unordered_map<std::string, Value> values_;
};

} // namespace goto_lang
