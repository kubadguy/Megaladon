#pragma once

#include <string>
#include <vector>
#include <variant>
#include <memory>    // For std::shared_ptr
#include <stdexcept> // For std::runtime_error
#include <sstream>   // For std::stringstream
#include <cmath>     // For std::fmod
#include <iomanip>   // For std::fixed, std::setprecision

// Forward declarations for circular dependencies
class Interpreter;
class MegaladonCallable; // Forward declare MegaladonCallable because MegaladonValue uses it

// --- MegaladonValue Definition ---
// Define a variant to hold different types of values
enum ValueType {
    VOID,
    NUMBER,
    BOOLEAN,
    STRING,
    LIST,
    FUNCTION, // For user-defined functions and built-in callables
    INVALID   // For error states or uninitialized values
};

class MegaladonValue {
public:
    // Use std::variant to hold different types of data
    // std::monostate is for VOID type
    std::variant<std::monostate, double, bool, std::string, std::vector<MegaladonValue>, std::shared_ptr<MegaladonCallable>> data;
    ValueType type;

    // Constructors
    MegaladonValue() : data(std::monostate{}), type(VOID) {} // Default constructor for VOID
    MegaladonValue(ValueType type) : type(type) { // For specific VOID or INVALID initialization
        if (type == NUMBER) data = 0.0;
        else if (type == BOOLEAN) data = false;
        else if (type == STRING) data = "";
        else if (type == LIST) data = std::vector<MegaladonValue>();
        else if (type == FUNCTION) data = std::shared_ptr<MegaladonCallable>();
    }

    MegaladonValue(double val) : data(val), type(NUMBER) {}
    MegaladonValue(bool val) : data(val), type(BOOLEAN) {}
    MegaladonValue(std::string val) : data(std::move(val)), type(STRING) {}
    MegaladonValue(std::vector<MegaladonValue> val) : data(std::move(val)), type(LIST) {}
    MegaladonValue(std::shared_ptr<MegaladonCallable> val) : data(std::move(val)), type(FUNCTION) {}

    // Type checking methods
    bool isVoid() const { return type == VOID; }
    bool isNumber() const { return type == NUMBER; }
    bool isBoolean() const { return type == BOOLEAN; }
    bool isString() const { return type == STRING; }
    bool isList() const { return type == LIST; }
    bool isFunction() const { return type == FUNCTION; }
    bool isInvalid() const { return type == INVALID; }

    // Value conversion methods (with checks for safety)
    double asNumber() const {
        if (type == NUMBER) return std::get<double>(data);
        // Handle error or conversion failure appropriately
        throw std::runtime_error("MegaladonError: Value is not a number.");
    }

    bool asBoolean() const {
        if (type == BOOLEAN) return std::get<bool>(data);
        throw std::runtime_error("MegaladonError: Value is not a boolean.");
    }

    const std::string& asString() const {
        if (type == STRING) return std::get<std::string>(data);
        throw std::runtime_error("MegaladonError: Value is not a string.");
    }

    const std::vector<MegaladonValue>& asList() const {
        if (type == LIST) return std::get<std::vector<MegaladonValue>>(data);
        throw std::runtime_error("MegaladonError: Value is not a list.");
    }

    // For modifying list in place for methods
    // NOTE: This should only be called on a non-const MegaladonValue
    std::vector<MegaladonValue>& asListMutable() {
        if (type == LIST) return std::get<std::vector<MegaladonValue>>(data);
        throw std::runtime_error("MegaladonError: Value is not a list or cannot be modified.");
    }

    std::shared_ptr<MegaladonCallable> asCallable() const {
        if (type == FUNCTION) return std::get<std::shared_ptr<MegaladonCallable>>(data);
        throw std::runtime_error("MegaladonError: Value is not a callable function.");
    }

    // String representation for debugging and 'print' function
    std::string toString() const; // Implemented in value.cpp
};

// Equality operator (for comparing MegaladonValues)
bool operator==(const MegaladonValue& lhs, const MegaladonValue& rhs);
bool operator!=(const MegaladonValue& lhs, const MegaladonValue& rhs);

// --- MegaladonCallable Definition ---
// Define MegaladonCallable AFTER MegaladonValue, as it uses MegaladonValue directly
class MegaladonCallable {
public:
    // Pure virtual functions that derived classes must implement
    virtual int arity() const = 0;
    virtual std::string toString() const = 0;
    virtual MegaladonValue call(Interpreter& interpreter, const std::vector<MegaladonValue>& arguments) = 0;

    // A virtual destructor is crucial for proper polymorphism with shared_ptr
    virtual ~MegaladonCallable() = default;
};
// --- End MegaladonCallable Definition ---