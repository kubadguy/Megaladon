// src/types/value.h
#pragma once

#include <string>
#include <vector>
#include <variant> // C++17 for std::variant
#include <memory>  // For std::shared_ptr

// Forward declare MegaladonCallable (defined in interpreter.h)
class MegaladonCallable;
class Interpreter; // Forward declare Interpreter for callMethod

// Represents any value in the Megaladon language
class MegaladonValue {
public:
    enum ValueType {
        VOID,    // For statements that return nothing, or uninitialized variables
        NUMBER,
        BOOLEAN,
        STRING,
        LIST,
        FUNCTION, // User-defined functions and built-in callables
        INVALID  // For error states or uninitialized values if not using VOID for that
    };

private:
    // Use std::variant to hold one of several types safely and efficiently.
    // For lists and strings, use std::vector and std::string directly.
    // For functions, use a smart pointer to the callable object.
    std::variant<std::monostate, double, bool, std::string, std::vector<MegaladonValue>, std::shared_ptr<MegaladonCallable>> data;
    ValueType type;

public:
    // Constructors
    MegaladonValue() : data(std::monostate{}), type(VOID) {} // Default constructor for VOID
    MegaladonValue(double val) : data(val), type(NUMBER) {}
    MegaladonValue(bool val) : data(val), type(BOOLEAN) {}
    MegaladonValue(std::string val) : data(std::move(val)), type(STRING) {}
    MegaladonValue(std::vector<MegaladonValue> val) : data(std::move(val)), type(LIST) {}
    MegaladonValue(std::shared_ptr<MegaladonCallable> val) : data(std::move(val)), type(FUNCTION) {}
    MegaladonValue(ValueType type) : type(type) { // For specific VOID or INVALID initialization
        if (type != VOID && type != INVALID) {
            // Error: Constructor used incorrectly for non-monostate types
        }
    }


    // Type checkers
    bool isVoid() const { return type == VOID; }
    bool isNumber() const { return type == NUMBER; }
    bool isBoolean() const { return type == BOOLEAN; }
    bool isString() const { return type == STRING; }
    bool isList() const { return type == LIST; }
    bool isFunction() const { return type == FUNCTION; }
    bool isInvalid() const { return type == INVALID; }

    // Accessors (with safety checks or exceptions)
    double asNumber() const;
    bool asBoolean() const;
    const std::string& asString() const;
    std::string& asStringMutable(); // For modifying string in place for methods
    const std::vector<MegaladonValue>& asList() const;
    std::vector<MegaladonValue>& asListMutable(); // For modifying list in place for methods
    MegaladonCallable* asFunction() const; // Returns raw pointer, ownership managed by shared_ptr in data

    // Operators
    MegaladonValue operator+(const MegaladonValue& other) const;
    MegaladonValue operator-(const MegaladonValue& other) const;
    MegaladonValue operator*(const MegaladonValue& other) const;
    MegaladonValue operator/(const MegaladonValue& other) const;
    MegaladonValue operator%(const MegaladonValue& other) const;

    bool operator==(const MegaladonValue& other) const;
    bool operator!=(const MegaladonValue& other) const;
    bool operator<(const MegaladonValue& other) const;
    bool operator<=(const MegaladonValue& other) const;
    bool operator>(const MegaladonValue& other) const;
    bool operator>=(const MegaladonValue& other) const;

    // Truthiness concept for control flow
    bool isTruthy() const;

    // Convert to string for printing/debugging
    std::string toString() const;

    // Method calling mechanism for objects (strings, lists)
    MegaladonValue callMethod(Interpreter& interpreter, const std::string& methodName, const std::vector<MegaladonValue>& args);

private:
    // Helper for operator overloading to report type errors
    void checkNumericOperand(const MegaladonValue& operand) const;
    void checkNumericOperands(const MegaladonValue& other) const;
};