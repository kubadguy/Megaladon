#pragma once

#include "../types/value.h" // Ensures MegaladonCallable is fully defined BEFORE MegaladonBuiltin
#include <string>
#include <vector>
#include <memory> // For std::shared_ptr

// Forward declarations to avoid circular dependencies if needed
class Interpreter;
class Environment; // For registerBuiltins

class MegaladonBuiltin : public MegaladonCallable {
public:
    MegaladonBuiltin(const std::string& name, int arity) : name(name), _arity(arity) {}

    int arity() const override { return _arity; }
    std::string toString() const override { return "[Built-in Function " + name + "]"; }

protected:
    std::string name;
    int _arity;
};

// Specific built-in functions
class PrintBuiltin : public MegaladonBuiltin {
public:
    PrintBuiltin() : MegaladonBuiltin("print", 1) {}
    MegaladonValue call(Interpreter& interpreter, const std::vector<MegaladonValue>& arguments) override;
};

class InputBuiltin : public MegaladonBuiltin {
public:
    InputBuiltin() : MegaladonBuiltin("input", 0) {} // Assuming input takes no args directly, or one prompt arg
    MegaladonValue call(Interpreter& interpreter, const std::vector<MegaladonValue>& arguments) override;
};

class LenBuiltin : public MegaladonBuiltin {
public:
    LenBuiltin() : MegaladonBuiltin("len", 1) {}
    MegaladonValue call(Interpreter& interpreter, const std::vector<MegaladonValue>& arguments) override;
};

// Forward declaration for the registration function
void registerBuiltins(std::shared_ptr<Environment>& env);