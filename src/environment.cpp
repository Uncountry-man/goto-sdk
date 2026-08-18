#include "../include/goto/environment.hpp"
#include <iostream>
#include <chrono>
#include <random>
#include <cmath>

namespace goto_lang {

Environment::Environment() : enclosing_(nullptr) {
    registerBuiltins();
}

Environment::Environment(std::shared_ptr<Environment> enclosing)
    : enclosing_(std::move(enclosing)) {}

void Environment::define(const std::string& name, Value value) {
    values_[name] = std::move(value);
}

bool Environment::assign(const std::string& name, const Value& value) {
    auto it = values_.find(name);
    if (it != values_.end()) {
        it->second = value;
        return true;
    }
    if (enclosing_) {
        return enclosing_->assign(name, value);
    }
    // If not found in any scope, define in current scope (dynamic variable declaration on assignment)
    values_[name] = value;
    return true;
}

Value Environment::get(const std::string& name) const {
    auto it = values_.find(name);
    if (it != values_.end()) {
        return it->second;
    }
    if (enclosing_) {
        return enclosing_->get(name);
    }
    throw GoToException("Undefined variable '" + name + "'");
}

bool Environment::has(const std::string& name) const {
    if (values_.find(name) != values_.end()) return true;
    if (enclosing_) return enclosing_->has(name);
    return false;
}

std::unordered_map<std::string, Value> Environment::getAllVariables() const {
    std::unordered_map<std::string, Value> all;
    if (enclosing_) {
        all = enclosing_->getAllVariables();
    }
    for (const auto& [k, v] : values_) {
        // Skip built-in functions when dumping user variables
        if (!v.isNativeFunction()) {
            all[k] = v;
        }
    }
    return all;
}

void Environment::loadVariables(const std::unordered_map<std::string, Value>& vars) {
    for (const auto& [k, v] : vars) {
        values_[k] = v;
    }
}

void Environment::registerBuiltins() {
    // print(...)
    define("print", Value([](const std::vector<Value>& args, Environment&) -> Value {
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) std::cout << " ";
            std::cout << args[i].toString();
        }
        return Value(nullptr);
    }, "print"));

    // println(...)
    define("println", Value([](const std::vector<Value>& args, Environment&) -> Value {
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) std::cout << " ";
            std::cout << args[i].toString();
        }
        std::cout << "\n";
        return Value(nullptr);
    }, "println"));

    // input(prompt)
    define("input", Value([](const std::vector<Value>& args, Environment&) -> Value {
        if (!args.empty()) {
            std::cout << args[0].toString();
        }
        std::string line;
        std::getline(std::cin, line);
        return Value(line);
    }, "input"));

    // len(obj)
    define("len", Value([](const std::vector<Value>& args, Environment&) -> Value {
        if (args.empty()) throw GoToException("len() takes exactly 1 argument (0 given)");
        const auto& obj = args[0];
        if (obj.isList()) {
            return Value(static_cast<double>(obj.asList()->size()));
        }
        if (obj.isDict()) {
            return Value(static_cast<double>(obj.asDict()->size()));
        }
        if (obj.isString()) {
            return Value(static_cast<double>(obj.asString().size()));
        }
        throw GoToException("len() unsupported on type " + obj.typeName());
    }, "len"));

    // push(list, val)
    define("push", Value([](const std::vector<Value>& args, Environment&) -> Value {
        if (args.size() < 2) throw GoToException("push(list, value) requires 2 arguments");
        if (!args[0].isList()) throw GoToException("push() requires a list as first argument");
        args[0].asList()->push_back(args[1]);
        return args[0];
    }, "push"));

    // pop(list)
    define("pop", Value([](const std::vector<Value>& args, Environment&) -> Value {
        if (args.empty()) throw GoToException("pop(list) requires 1 argument");
        if (!args[0].isList()) throw GoToException("pop() requires a list as first argument");
        auto list = args[0].asList();
        if (list->empty()) throw GoToException("pop() called on empty list");
        Value last = list->back();
        list->pop_back();
        return last;
    }, "pop"));

    // keys(dict)
    define("keys", Value([](const std::vector<Value>& args, Environment&) -> Value {
        if (args.empty() || !args[0].isDict()) throw GoToException("keys(dict) requires a dictionary");
        const auto& dict = *args[0].asDict();
        std::vector<Value> result;
        result.reserve(dict.size());
        for (const auto& [k, v] : dict) {
            result.push_back(Value(k));
        }
        return Value::makeList(result);
    }, "keys"));

    // values(dict)
    define("values", Value([](const std::vector<Value>& args, Environment&) -> Value {
        if (args.empty() || !args[0].isDict()) throw GoToException("values(dict) requires a dictionary");
        const auto& dict = *args[0].asDict();
        std::vector<Value> result;
        result.reserve(dict.size());
        for (const auto& [k, v] : dict) {
            result.push_back(v);
        }
        return Value::makeList(result);
    }, "values"));

    // has(dict, key)
    define("has", Value([](const std::vector<Value>& args, Environment&) -> Value {
        if (args.size() < 2 || !args[0].isDict()) throw GoToException("has(dict, key) requires a dictionary and a key");
        const auto& dict = *args[0].asDict();
        return Value(dict.find(args[1].toString()) != dict.end());
    }, "has"));

    // remove(dict, key)
    define("remove", Value([](const std::vector<Value>& args, Environment&) -> Value {
        if (args.size() < 2 || !args[0].isDict()) throw GoToException("remove(dict, key) requires a dictionary and a key");
        auto dict = args[0].asDict();
        dict->erase(args[1].toString());
        return args[0];
    }, "remove"));

    // str(val)
    define("str", Value([](const std::vector<Value>& args, Environment&) -> Value {
        if (args.empty()) return Value("");
        return Value(args[0].toString());
    }, "str"));

    // num(val)
    define("num", Value([](const std::vector<Value>& args, Environment&) -> Value {
        if (args.empty()) return Value(0.0);
        if (args[0].isNumber()) return args[0];
        if (args[0].isBool()) return Value(args[0].asBool() ? 1.0 : 0.0);
        if (args[0].isString()) {
            try {
                return Value(std::stod(args[0].asString()));
            } catch (...) {
                return Value(0.0);
            }
        }
        return Value(0.0);
    }, "num"));

    // int(val)
    define("int", Value([](const std::vector<Value>& args, Environment&) -> Value {
        if (args.empty()) return Value(0);
        if (args[0].isNumber()) return Value(std::floor(args[0].asNumber()));
        if (args[0].isString()) {
            try {
                return Value(std::stoll(args[0].asString()));
            } catch (...) {
                return Value(0);
            }
        }
        return Value(0);
    }, "int"));

    // type(val)
    define("type", Value([](const std::vector<Value>& args, Environment&) -> Value {
        if (args.empty()) return Value("nil");
        return Value(args[0].typeName());
    }, "type"));

    // random(min, max)
    define("random", Value([](const std::vector<Value>& args, Environment&) -> Value {
        static std::mt19937 rng(std::random_device{}());
        if (args.size() == 1) {
            double max = args[0].isNumber() ? args[0].asNumber() : 1.0;
            std::uniform_real_distribution<double> dist(0.0, max);
            return Value(dist(rng));
        } else if (args.size() >= 2) {
            double min = args[0].isNumber() ? args[0].asNumber() : 0.0;
            double max = args[1].isNumber() ? args[1].asNumber() : 1.0;
            std::uniform_real_distribution<double> dist(min, max);
            return Value(dist(rng));
        }
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return Value(dist(rng));
    }, "random"));

    // clock()
    define("clock", Value([](const std::vector<Value>&, Environment&) -> Value {
        auto now = std::chrono::steady_clock::now().time_since_epoch();
        double seconds = std::chrono::duration<double>(now).count();
        return Value(seconds);
    }, "clock"));
}

} // namespace goto_lang
