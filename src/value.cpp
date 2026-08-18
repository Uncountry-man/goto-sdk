#include "../include/goto/value.hpp"
#include <cmath>
#include <iomanip>
#include <sstream>

namespace goto_lang {

Value::Value() : type_(ValueType::Nil), data_(std::monostate{}) {}
Value::Value(std::nullptr_t) : type_(ValueType::Nil), data_(std::monostate{}) {}
Value::Value(bool b) : type_(ValueType::Bool), data_(b) {}
Value::Value(int n) : type_(ValueType::Number), data_(static_cast<double>(n)) {}
Value::Value(int64_t n) : type_(ValueType::Number), data_(static_cast<double>(n)) {}
Value::Value(double n) : type_(ValueType::Number), data_(n) {}
Value::Value(const char* s) : type_(ValueType::String), data_(std::string(s ? s : "")) {}
Value::Value(std::string s) : type_(ValueType::String), data_(std::move(s)) {}
Value::Value(ListPtr list) : type_(ValueType::List), data_(list ? list : std::make_shared<std::vector<Value>>()) {}
Value::Value(DictPtr dict) : type_(ValueType::Dict), data_(dict ? dict : std::make_shared<std::unordered_map<std::string, Value>>()) {}

Value::Value(FunctionData fn)
    : type_(ValueType::Function),
      data_(std::make_shared<FunctionData>(std::move(fn))) {}

Value::Value(NativeFn nativeFn, std::string name)
    : type_(ValueType::NativeFunction),
      data_(std::make_shared<NativeWrapper>(NativeWrapper{std::move(nativeFn), std::move(name)})) {}

Value Value::makeList(std::vector<Value> elements) {
    return Value(std::make_shared<std::vector<Value>>(std::move(elements)));
}

Value Value::makeDict(std::unordered_map<std::string, Value> map) {
    return Value(std::make_shared<std::unordered_map<std::string, Value>>(std::move(map)));
}

Value Value::makeLabelRef(std::string labelName) {
    Value val;
    val.type_ = ValueType::LabelRef;
    val.data_ = std::move(labelName);
    return val;
}

std::string Value::typeName() const {
    switch (type_) {
        case ValueType::Nil: return "nil";
        case ValueType::Bool: return "bool";
        case ValueType::Number: return "number";
        case ValueType::String: return "string";
        case ValueType::List: return "list";
        case ValueType::Dict: return "dict";
        case ValueType::Function: return "function";
        case ValueType::NativeFunction: return "native_function";
        case ValueType::LabelRef: return "label";
    }
    return "unknown";
}

bool Value::asBool() const {
    if (type_ == ValueType::Bool) return std::get<bool>(data_);
    throw GoToException("Type error: Expected bool, got " + typeName());
}

double Value::asNumber() const {
    if (type_ == ValueType::Number) return std::get<double>(data_);
    throw GoToException("Type error: Expected number, got " + typeName());
}

const std::string& Value::asString() const {
    if (type_ == ValueType::String) return std::get<std::string>(data_);
    throw GoToException("Type error: Expected string, got " + typeName());
}

ListPtr Value::asList() const {
    if (type_ == ValueType::List) return std::get<ListPtr>(data_);
    throw GoToException("Type error: Expected list, got " + typeName());
}

DictPtr Value::asDict() const {
    if (type_ == ValueType::Dict) return std::get<DictPtr>(data_);
    throw GoToException("Type error: Expected dict, got " + typeName());
}

const FunctionData& Value::asFunction() const {
    if (type_ == ValueType::Function) return *std::get<std::shared_ptr<FunctionData>>(data_);
    throw GoToException("Type error: Expected user function, got " + typeName());
}

const NativeFn& Value::asNativeFunction() const {
    if (type_ == ValueType::NativeFunction) return std::get<std::shared_ptr<NativeWrapper>>(data_)->fn;
    throw GoToException("Type error: Expected native function, got " + typeName());
}

const std::string& Value::asLabelRef() const {
    if (type_ == ValueType::LabelRef) return std::get<std::string>(data_);
    throw GoToException("Type error: Expected label reference, got " + typeName());
}

bool Value::isTruthy() const {
    switch (type_) {
        case ValueType::Nil:
            return false;
        case ValueType::Bool:
            return std::get<bool>(data_);
        case ValueType::Number:
            return std::get<double>(data_) != 0.0;
        case ValueType::String:
            return !std::get<std::string>(data_).empty();
        case ValueType::List:
            return !std::get<ListPtr>(data_)->empty();
        case ValueType::Dict:
            return !std::get<DictPtr>(data_)->empty();
        case ValueType::Function:
        case ValueType::NativeFunction:
        case ValueType::LabelRef:
            return true;
    }
    return false;
}

std::string Value::toString() const {
    switch (type_) {
        case ValueType::Nil: return "nil";
        case ValueType::Bool: return std::get<bool>(data_) ? "true" : "false";
        case ValueType::Number: {
            double n = std::get<double>(data_);
            if (std::floor(n) == n && !std::isinf(n) && !std::isnan(n)) {
                return std::to_string(static_cast<int64_t>(n));
            }
            std::ostringstream ss;
            ss << n;
            return ss.str();
        }
        case ValueType::String: return std::get<std::string>(data_);
        case ValueType::List: {
            const auto& list = *std::get<ListPtr>(data_);
            std::string out = "[";
            for (size_t i = 0; i < list.size(); ++i) {
                if (i > 0) out += ", ";
                out += list[i].toRepr();
            }
            out += "]";
            return out;
        }
        case ValueType::Dict: {
            const auto& dict = *std::get<DictPtr>(data_);
            std::string out = "{";
            bool first = true;
            for (const auto& [k, v] : dict) {
                if (!first) out += ", ";
                first = false;
                out += "\"" + k + "\": " + v.toRepr();
            }
            out += "}";
            return out;
        }
        case ValueType::Function: {
            const auto& fn = *std::get<std::shared_ptr<FunctionData>>(data_);
            return "<fn " + (fn.name.empty() ? "anonymous" : fn.name) + ">";
        }
        case ValueType::NativeFunction: {
            const auto& wrapper = *std::get<std::shared_ptr<NativeWrapper>>(data_);
            return "<native fn " + wrapper.name + ">";
        }
        case ValueType::LabelRef: {
            return "<label " + std::get<std::string>(data_) + ">";
        }
    }
    return "nil";
}

std::string Value::toRepr() const {
    if (type_ == ValueType::String) {
        std::string s = std::get<std::string>(data_);
        std::string escaped = "\"";
        for (char c : s) {
            if (c == '"') escaped += "\\\"";
            else if (c == '\\') escaped += "\\\\";
            else if (c == '\n') escaped += "\\n";
            else if (c == '\t') escaped += "\\t";
            else escaped += c;
        }
        escaped += "\"";
        return escaped;
    }
    return toString();
}

std::string Value::toJson() const {
    switch (type_) {
        case ValueType::Nil: return "null";
        case ValueType::Bool: return std::get<bool>(data_) ? "true" : "false";
        case ValueType::Number: {
            double n = std::get<double>(data_);
            if (std::floor(n) == n && !std::isinf(n) && !std::isnan(n)) {
                return std::to_string(static_cast<int64_t>(n));
            }
            std::ostringstream ss;
            ss << n;
            return ss.str();
        }
        case ValueType::String: return toRepr();
        case ValueType::List: {
            const auto& list = *std::get<ListPtr>(data_);
            std::string out = "[";
            for (size_t i = 0; i < list.size(); ++i) {
                if (i > 0) out += ",";
                out += list[i].toJson();
            }
            out += "]";
            return out;
        }
        case ValueType::Dict: {
            const auto& dict = *std::get<DictPtr>(data_);
            std::string out = "{";
            bool first = true;
            for (const auto& [k, v] : dict) {
                if (!first) out += ",";
                first = false;
                out += "\"" + k + "\":" + v.toJson();
            }
            out += "}";
            return out;
        }
        default:
            return "\"" + toString() + "\"";
    }
}

Value Value::getSubscript(const Value& keyOrIndex) const {
    if (type_ == ValueType::List) {
        const auto& list = *std::get<ListPtr>(data_);
        if (!keyOrIndex.isNumber()) {
            throw GoToException("List index must be a number, got " + keyOrIndex.typeName());
        }
        int idx = static_cast<int>(keyOrIndex.asNumber());
        if (idx < 0) idx = static_cast<int>(list.size()) + idx;
        if (idx < 0 || idx >= static_cast<int>(list.size())) {
            throw GoToException("List index out of bounds: " + std::to_string(idx) + " (size " + std::to_string(list.size()) + ")");
        }
        return list[static_cast<size_t>(idx)];
    }

    if (type_ == ValueType::Dict) {
        const auto& dict = *std::get<DictPtr>(data_);
        std::string key = keyOrIndex.toString();
        auto it = dict.find(key);
        if (it == dict.end()) {
            return Value(); // Nil if not found
        }
        return it->second;
    }

    if (type_ == ValueType::String) {
        const auto& str = std::get<std::string>(data_);
        if (!keyOrIndex.isNumber()) {
            throw GoToException("String index must be a number, got " + keyOrIndex.typeName());
        }
        int idx = static_cast<int>(keyOrIndex.asNumber());
        if (idx < 0) idx = static_cast<int>(str.size()) + idx;
        if (idx < 0 || idx >= static_cast<int>(str.size())) {
            throw GoToException("String index out of bounds: " + std::to_string(idx));
        }
        return Value(std::string(1, str[static_cast<size_t>(idx)]));
    }

    throw GoToException("Cannot index value of type " + typeName());
}

void Value::setSubscript(const Value& keyOrIndex, const Value& val) {
    if (type_ == ValueType::List) {
        auto& list = *std::get<ListPtr>(data_);
        if (!keyOrIndex.isNumber()) {
            throw GoToException("List index must be a number, got " + keyOrIndex.typeName());
        }
        int idx = static_cast<int>(keyOrIndex.asNumber());
        if (idx < 0) idx = static_cast<int>(list.size()) + idx;
        if (idx < 0 || idx >= static_cast<int>(list.size())) {
            throw GoToException("List index out of bounds: " + std::to_string(idx));
        }
        list[static_cast<size_t>(idx)] = val;
        return;
    }

    if (type_ == ValueType::Dict) {
        auto& dict = *std::get<DictPtr>(data_);
        std::string key = keyOrIndex.toString();
        dict[key] = val;
        return;
    }

    throw GoToException("Cannot assign index on value of type " + typeName());
}

Value Value::operator+(const Value& other) const {
    if (isNumber() && other.isNumber()) {
        return Value(asNumber() + other.asNumber());
    }
    if (isString() || other.isString()) {
        return Value(toString() + other.toString());
    }
    if (isList() && other.isList()) {
        auto combined = std::make_shared<std::vector<Value>>(*asList());
        const auto& otherList = *other.asList();
        combined->insert(combined->end(), otherList.begin(), otherList.end());
        return Value(combined);
    }
    throw GoToException("Invalid operator '+' between " + typeName() + " and " + other.typeName());
}

Value Value::operator-(const Value& other) const {
    if (isNumber() && other.isNumber()) {
        return Value(asNumber() - other.asNumber());
    }
    throw GoToException("Invalid operator '-' between " + typeName() + " and " + other.typeName());
}

Value Value::operator*(const Value& other) const {
    if (isNumber() && other.isNumber()) {
        return Value(asNumber() * other.asNumber());
    }
    if (isString() && other.isNumber()) {
        int times = static_cast<int>(other.asNumber());
        std::string result;
        for (int i = 0; i < times; ++i) result += asString();
        return Value(result);
    }
    throw GoToException("Invalid operator '*' between " + typeName() + " and " + other.typeName());
}

Value Value::operator/(const Value& other) const {
    if (isNumber() && other.isNumber()) {
        double divisor = other.asNumber();
        if (divisor == 0.0) {
            throw GoToException("Division by zero");
        }
        return Value(asNumber() / divisor);
    }
    throw GoToException("Invalid operator '/' between " + typeName() + " and " + other.typeName());
}

Value Value::operator%(const Value& other) const {
    if (isNumber() && other.isNumber()) {
        double divisor = other.asNumber();
        if (divisor == 0.0) {
            throw GoToException("Modulo by zero");
        }
        return Value(std::fmod(asNumber(), divisor));
    }
    throw GoToException("Invalid operator '%' between " + typeName() + " and " + other.typeName());
}

Value Value::operator-() const {
    if (isNumber()) {
        return Value(-asNumber());
    }
    throw GoToException("Cannot negate non-number type " + typeName());
}

bool Value::operator==(const Value& other) const {
    if (type_ != other.type_) return false;
    switch (type_) {
        case ValueType::Nil: return true;
        case ValueType::Bool: return std::get<bool>(data_) == std::get<bool>(other.data_);
        case ValueType::Number: return std::get<double>(data_) == std::get<double>(other.data_);
        case ValueType::String: return std::get<std::string>(data_) == std::get<std::string>(other.data_);
        case ValueType::List: return std::get<ListPtr>(data_) == std::get<ListPtr>(other.data_);
        case ValueType::Dict: return std::get<DictPtr>(data_) == std::get<DictPtr>(other.data_);
        case ValueType::Function: return std::get<std::shared_ptr<FunctionData>>(data_) == std::get<std::shared_ptr<FunctionData>>(other.data_);
        case ValueType::NativeFunction: return std::get<std::shared_ptr<NativeWrapper>>(data_) == std::get<std::shared_ptr<NativeWrapper>>(other.data_);
        case ValueType::LabelRef: return std::get<std::string>(data_) == std::get<std::string>(other.data_);
    }
    return false;
}

bool Value::operator<(const Value& other) const {
    if (isNumber() && other.isNumber()) {
        return asNumber() < other.asNumber();
    }
    if (isString() && other.isString()) {
        return asString() < other.asString();
    }
    throw GoToException("Invalid comparison '<' between " + typeName() + " and " + other.typeName());
}

bool Value::operator<=(const Value& other) const {
    if (isNumber() && other.isNumber()) {
        return asNumber() <= other.asNumber();
    }
    if (isString() && other.isString()) {
        return asString() <= other.asString();
    }
    throw GoToException("Invalid comparison '<=' between " + typeName() + " and " + other.typeName());
}

bool Value::operator>(const Value& other) const {
    if (isNumber() && other.isNumber()) {
        return asNumber() > other.asNumber();
    }
    if (isString() && other.isString()) {
        return asString() > other.asString();
    }
    throw GoToException("Invalid comparison '>' between " + typeName() + " and " + other.typeName());
}

bool Value::operator>=(const Value& other) const {
    if (isNumber() && other.isNumber()) {
        return asNumber() >= other.asNumber();
    }
    if (isString() && other.isString()) {
        return asString() >= other.asString();
    }
    throw GoToException("Invalid comparison '>=' between " + typeName() + " and " + other.typeName());
}

} // namespace goto_lang
