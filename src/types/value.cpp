// src/types/value.cpp
#include "value.h"
#include "../interpreter/interpreter.h" // For MegaladonCallable and Interpreter
#include "../util/error.h" // For runtime error reporting

#include <stdexcept>
#include <string>
#include <algorithm> // For std::transform for string methods (if not in string_methods.cpp)
#include <cctype>    // For ::tolower, ::toupper

// Include built-in method implementations
// These are functions that take `const std::vector<MegaladonValue>& args`
// The first arg in `args` is always the `this` object (the string/list itself).
extern MegaladonValue string_len(const std::vector<MegaladonValue>& args);
extern MegaladonValue string_substring(const std::vector<MegaladonValue>& args);
extern MegaladonValue string_to_lower(const std::vector<MegaladonValue>& args);
extern MegaladonValue string_to_upper(const std::vector<MegaladonValue>& args);
extern MegaladonValue string_trim(const std::vector<MegaladonValue>& args);
extern MegaladonValue string_starts_with(const std::vector<MegaladonValue>& args);
extern MegaladonValue string_ends_with(const std::vector<MegaladonValue>& args);
extern MegaladonValue string_contains(const std::vector<MegaladonValue>& args);
extern MegaladonValue string_replace(const std::vector<MegaladonValue>& args);
extern MegaladonValue string_split(const std::vector<MegaladonValue>& args);
extern MegaladonValue string_index_of(const std::vector<MegaladonValue>& args);
extern MegaladonValue string_to_list(const std::vector<MegaladonValue>& args);
extern MegaladonValue string_count_vowels(const std::vector<MegaladonValue>& args);


extern MegaladonValue list_len(const std::vector<MegaladonValue>& args);
extern MegaladonValue list_add(const std::vector<MegaladonValue>& args);
extern MegaladonValue list_remove_at(const std::vector<MegaladonValue>& args);
extern MegaladonValue list_get(const std::vector<MegaladonValue>& args); // Used for explicit .get()
extern MegaladonValue list_set(const std::vector<MegaladonValue>& args); // Used for explicit .set()
extern MegaladonValue list_contains(const std::vector<MegaladonValue>& args);
extern MegaladonValue list_insert_at(const std::vector<MegaladonValue>& args);
extern MegaladonValue list_pop(const std::vector<MegaladonValue>& args);
extern MegaladonValue list_clear(const std::vector<MegaladonValue>& args);
extern MegaladonValue list_join(const std::vector<MegaladonValue>& args);
extern MegaladonValue list_reverse(const std::vector<MegaladonValue>& args);
extern MegaladonValue list_slice(const std::vector<MegaladonValue>& args);
extern MegaladonValue list_remove(const std::vector<MegaladonValue>& args);
extern MegaladonValue list_sort(const std::vector<MegaladonValue>& args);
extern MegaladonValue list_filter(Interpreter& interpreter, const std::vector<MegaladonValue>& args);
extern MegaladonValue list_map(Interpreter& interpreter, const std::vector<MegaladonValue>& args);


// --- Accessors ---
double MegaladonValue::asNumber() const {
    if (type == NUMBER) return std::get<double>(data);
    throw std::runtime_error("MegaladonValue is not a number.");
}

bool MegaladonValue::asBoolean() const {
    if (type == BOOLEAN) return std::get<bool>(data);
    throw std::runtime_error("MegaladonValue is not a boolean.");
}

const std::string& MegaladonValue::asString() const {
    if (type == STRING) return std::get<std::string>(data);
    throw std::runtime_error("MegaladonValue is not a string.");
}

std::string& MegaladonValue::asStringMutable() {
    if (type == STRING) return std::get<std::string>(data);
    throw std::runtime_error("MegaladonValue is not a string (mutable access requested).");
}

const std::vector<MegaladonValue>& MegaladonValue::asList() const {
    if (type == LIST) return std::get<std::vector<MegaladonValue>>(data);
    throw std::runtime_error("MegaladonValue is not a list.");
}

std::vector<MegaladonValue>& MegaladonValue::asListMutable() {
    if (type == LIST) return std::get<std::vector<MegaladonValue>>(data);
    throw std::runtime_error("MegaladonValue is not a list (mutable access requested).");
}

MegaladonCallable* MegaladonValue::asFunction() const {
    if (type == FUNCTION) return std::get<std::shared_ptr<MegaladonCallable>>(data).get();
    throw std::runtime_error("MegaladonValue is not a function.");
}


// --- Operators ---

void MegaladonValue::checkNumericOperand(const MegaladonValue& operand) const {
    if (!operand.isNumber()) {
        throw std::runtime_error("Operand must be a number for this operation.");
    }
}

void MegaladonValue::checkNumericOperands(const MegaladonValue& other) const {
    if (!isNumber() || !other.isNumber()) {
        throw std::runtime_error("Operands must be numbers for this operation.");
    }
}

MegaladonValue MegaladonValue::operator+(const MegaladonValue& other) const {
    if (isNumber() && other.isNumber()) {
        return MegaladonValue(asNumber() + other.asNumber());
    } else if (isString() && other.isString()) {
        return MegaladonValue(asString() + other.asString());
    } else if (isList() && other.isList()) {
        std::vector<MegaladonValue> newList = asList();
        newList.insert(newList.end(), other.asList().begin(), other.asList().end());
        return MegaladonValue(newList);
    }
    throw std::runtime_error("Invalid operands for '+' operator.");
}

MegaladonValue MegaladonValue::operator-(const MegaladonValue& other) const {
    checkNumericOperands(other);
    return MegaladonValue(asNumber() - other.asNumber());
}

MegaladonValue MegaladonValue::operator*(const MegaladonValue& other) const {
    checkNumericOperands(other);
    return MegaladonValue(asNumber() * other.asNumber());
}

MegaladonValue MegaladonValue::operator/(const MegaladonValue& other) const {
    checkNumericOperands(other);
    if (other.asNumber() == 0) {
        throw std::runtime_error("Division by zero.");
    }
    return MegaladonValue(asNumber() / other.asNumber());
}

MegaladonValue MegaladonValue::operator%(const MegaladonValue& other) const {
    checkNumericOperands(other);
    if (other.asNumber() == 0) {
        throw std::runtime_error("Modulo by zero.");
    }
    // Modulo for doubles isn't standard, typically requires fmod.
    // For simplicity, convert to int if values are effectively integers.
    // A robust solution would involve checking for integer-like numbers.
    if (fmod(asNumber(), 1) == 0 && fmod(other.asNumber(), 1) == 0) {
        return MegaladonValue(static_cast<double>(static_cast<long long>(asNumber()) % static_cast<long long>(other.asNumber())));
    }
    throw std::runtime_error("Modulo operator requires integer-like numbers.");
}


bool MegaladonValue::operator==(const MegaladonValue& other) const {
    if (type != other.type) {
        // Special case: VOID == VOID, but VOID is not equal to anything else.
        if (type == VOID && other.type == VOID) return true;
        return false;
    }

    switch (type) {
        case VOID:    return true; // void == void
        case NUMBER:  return asNumber() == other.asNumber();
        case BOOLEAN: return asBoolean() == other.asBoolean();
        case STRING:  return asString() == other.asString();
        case LIST: {
            // Compare list contents element by element
            const auto& this_list = asList();
            const auto& other_list = other.asList();
            if (this_list.size() != other_list.size()) return false;
            for (size_t i = 0; i < this_list.size(); ++i) {
                if (!(this_list[i] == other_list[i])) return false;
            }
            return true;
        }
        case FUNCTION:
            return asFunction() == other.asFunction(); // Compare raw pointers of shared_ptr
        case INVALID: return false; // Invalid is never equal to anything, even another invalid
    }
    return false; // Should be unreachable
}

bool MegaladonValue::operator!=(const MegaladonValue& other) const {
    return !(*this == other);
}

bool MegaladonValue::operator<(const MegaladonValue& other) const {
    if (isNumber() && other.isNumber()) {
        return asNumber() < other.asNumber();
    } else if (isString() && other.isString()) {
        return asString() < other.asString();
    }
    throw std::runtime_error("Invalid operands for '<' operator.");
}

bool MegaladonValue::operator<=(const MegaladonValue& other) const {
    if (isNumber() && other.isNumber()) {
        return asNumber() <= other.asNumber();
    } else if (isString() && other.isString()) {
        return asString() <= other.asString();
    }
    throw std::runtime_error("Invalid operands for '<=' operator.");
}

bool MegaladonValue::operator>(const MegaladonValue& other) const {
    if (isNumber() && other.isNumber()) {
        return asNumber() > other.asNumber();
    } else if (isString() && other.isString()) {
        return asString() > other.asString();
    }
    throw std::runtime_error("Invalid operands for '>' operator.");
}

bool MegaladonValue::operator>=(const MegaladonValue& other) const {
    if (isNumber() && other.isNumber()) {
        return asNumber() >= other.asNumber();
    } else if (isString() && other.isString()) {
        return asString() >= other.asString();
    }
    throw std::runtime_error("Invalid operands for '>=' operator.");
}


// --- Truthiness ---
bool MegaladonValue::isTruthy() const {
    switch (type) {
        case VOID:    return false;
        case NUMBER:  return asNumber() != 0.0;
        case BOOLEAN: return asBoolean();
        case STRING:  return !asString().empty();
        case LIST:    return !asList().empty();
        case FUNCTION: return true; // Functions are always "truthy"
        case INVALID: return false; // Invalid values are not truthy
    }
    return false; // Should be unreachable
}

// --- ToString ---
std::string MegaladonValue::toString() const {
    switch (type) {
        case VOID:    return "void";
        case NUMBER:  {
            std::string s = std::to_string(asNumber());
            // Remove trailing zeros and decimal point if it's an integer
            s.erase(s.find_last_not_of('0') + 1, std::string::npos);
            if (s.back() == '.') {
                s.pop_back();
            }
            return s;
        }
        case BOOLEAN: return asBoolean() ? "true" : "false";
        case STRING:  return asString();
        case LIST: {
            std::string s = "[";
            const auto& list = asList();
            for (size_t i = 0; i < list.size(); ++i) {
                s += list[i].toString();
                if (i < list.size() - 1) {
                    s += ", ";
                }
            }
            s += "]";
            return s;
        }
        case FUNCTION: return asFunction()->toString();
        case INVALID: return "<invalid>";
    }
    return "<unknown type>"; // Should be unreachable
}

// --- Method Calling ---
MegaladonValue MegaladonValue::callMethod(Interpreter& interpreter, const std::string& methodName, const std::vector<MegaladonValue>& args_from_call_expr) {
    // The 'this' object (the string or list itself) is the first argument to the method function
    std::vector<MegaladonValue> method_args;
    method_args.push_back(*this); // Add the 'this' object
    method_args.insert(method_args.end(), args_from_call_expr.begin(), args_from_call_expr.end());

    if (isString()) {
        if (methodName == "len") return string_len(method_args);
        else if (methodName == "substring") return string_substring(method_args);
        else if (methodName == "to_lower") return string_to_lower(method_args);
        else if (methodName == "to_upper") return string_to_upper(method_args);
        else if (methodName == "trim") return string_trim(method_args);
        else if (methodName == "starts_with") return string_starts_with(method_args);
        else if (methodName == "ends_with") return string_ends_with(method_args);
        else if (methodName == "contains") return string_contains(method_args);
        else if (methodName == "replace") return string_replace(method_args);
        else if (methodName == "split") return string_split(method_args);
        else if (methodName == "index_of") return string_index_of(method_args);
        else if (methodName == "to_list") return string_to_list(method_args);
        else if (methodName == "count_vowels") return string_count_vowels(method_args);
        else {
            ReportRuntimeError("Unknown string method: " + methodName);
            return MegaladonValue(MegaladonValue::INVALID);
        }
    } else if (isList()) {
        if (methodName == "len") return list_len(method_args);
        else if (methodName == "add") return list_add(method_args);
        else if (methodName == "remove_at") return list_remove_at(method_args);
        else if (methodName == "get") return list_get(method_args);
        else if (methodName == "set") return list_set(method_args);
        else if (methodName == "contains") return list_contains(method_args);
        else if (methodName == "insert_at") return list_insert_at(method_args);
        else if (methodName == "pop") return list_pop(method_args);
        else if (methodName == "clear") return list_clear(method_args);
        else if (methodName == "join") return list_join(method_args);
        else if (methodName == "reverse") return list_reverse(method_args);
        else if (methodName == "slice") return list_slice(method_args);
        else if (methodName == "remove") return list_remove(method_args);
        else if (methodName == "sort") return list_sort(method_args);
        else if (methodName == "filter") return list_filter(interpreter, method_args); // Pass interpreter for higher-order
        else if (methodName == "map") return list_map(interpreter, method_args); // Pass interpreter for higher-order
        else {
            ReportRuntimeError("Unknown list method: " + methodName);
            return MegaladonValue(MegaladonValue::INVALID);
        }
    }
    // Should not reach here if callMethod is only called on string/list types
    ReportRuntimeError("Cannot call method '" + methodName + "' on this type.");
    return MegaladonValue(MegaladonValue::INVALID);
}