#pragma once

#include "common.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <variant>
#include <ostream>

namespace goto_lang {

class Environment;
struct Stmt;
struct FnDeclStmt;

enum class ValueType {
    Nil,
    Bool,
    Number,
    String,
    List,
    Dict,
    Function,
    NativeFunction,
    LabelRef
};

class Value;

using ListPtr = std::shared_ptr<std::vector<Value>>;
using DictPtr = std::shared_ptr<std::unordered_map<std::string, Value>>;
using NativeFn = std::function<Value(const std::vector<Value>&, Environment&)>;

struct FunctionData {
    std::string name;
    std::vector<std::string> params;
    std::shared_ptr<Stmt> body;
    std::shared_ptr<Environment> closure;
};

class Value {
public:
    Value();                                  // Nil
    Value(std::nullptr_t);                    // Nil
    Value(bool b);                            // Bool
    Value(int n);                             // Number (int)
    Value(int64_t n);                         // Number (int64)
    Value(double n);                          // Number (double)
    Value(const char* s);                     // String
    Value(std::string s);                     // String
    Value(ListPtr list);                      // List
    Value(DictPtr dict);                      // Dict
    Value(FunctionData fn);                   // Function
    Value(NativeFn nativeFn, std::string name = "<native>"); // NativeFunction
    
    static Value makeList(std::vector<Value> elements = {});
    static Value makeDict(std::unordered_map<std::string, Value> map = {});
    static Value makeLabelRef(std::string labelName);

    [[nodiscard]] ValueType type() const noexcept { return type_; }
    [[nodiscard]] std::string typeName() const;

    [[nodiscard]] bool isNil() const noexcept { return type_ == ValueType::Nil; }
    [[nodiscard]] bool isBool() const noexcept { return type_ == ValueType::Bool; }
    [[nodiscard]] bool isNumber() const noexcept { return type_ == ValueType::Number; }
    [[nodiscard]] bool isString() const noexcept { return type_ == ValueType::String; }
    [[nodiscard]] bool isList() const noexcept { return type_ == ValueType::List; }
    [[nodiscard]] bool isDict() const noexcept { return type_ == ValueType::Dict; }
    [[nodiscard]] bool isFunction() const noexcept { return type_ == ValueType::Function; }
    [[nodiscard]] bool isNativeFunction() const noexcept { return type_ == ValueType::NativeFunction; }
    [[nodiscard]] bool isCallable() const noexcept { return isFunction() || isNativeFunction(); }
    [[nodiscard]] bool isLabelRef() const noexcept { return type_ == ValueType::LabelRef; }

    [[nodiscard]] bool asBool() const;
    [[nodiscard]] double asNumber() const;
    [[nodiscard]] const std::string& asString() const;
    [[nodiscard]] ListPtr asList() const;
    [[nodiscard]] DictPtr asDict() const;
    [[nodiscard]] const FunctionData& asFunction() const;
    [[nodiscard]] const NativeFn& asNativeFunction() const;
    [[nodiscard]] const std::string& asLabelRef() const;

    [[nodiscard]] bool isTruthy() const;
    [[nodiscard]] std::string toString() const;
    [[nodiscard]] std::string toRepr() const;
    [[nodiscard]] std::string toJson() const;

    // List/Dict/String Subscript access
    [[nodiscard]] Value getSubscript(const Value& keyOrIndex) const;
    void setSubscript(const Value& keyOrIndex, const Value& val);

    // Operator overloads
    Value operator+(const Value& other) const;
    Value operator-(const Value& other) const;
    Value operator*(const Value& other) const;
    Value operator/(const Value& other) const;
    Value operator%(const Value& other) const;
    Value operator-() const; // Unary negate

    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const { return !(*this == other); }
    bool operator<(const Value& other) const;
    bool operator<=(const Value& other) const;
    bool operator>(const Value& other) const;
    bool operator>=(const Value& other) const;

    friend std::ostream& operator<<(std::ostream& os, const Value& val) {
        return os << val.toString();
    }

private:
    ValueType type_{ValueType::Nil};

    struct NativeWrapper {
        NativeFn fn;
        std::string name;
    };

    std::variant<
        std::monostate,
        bool,
        double,
        std::string,
        ListPtr,
        DictPtr,
        std::shared_ptr<FunctionData>,
        std::shared_ptr<NativeWrapper>
    > data_;
};

} // namespace goto_lang
