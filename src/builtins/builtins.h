// src/builtins/builtins.h
#pragma once

#include "../types/value.h"
#include "../environment/environment.h"
#include <memory>
#include <string>
#include <vector>

// Forward declaration of Interpreter and MegaladonCallable (defined in interpreter.h)
// This is critical to avoid circular dependencies.
class Interpreter;
class MegaladonCallable;

// Base class for all built-in functions
class MegaladonBuiltin : public MegaladonCallable {
protected:
    std::string name;
    int _arity;
public:
    MegaladonBuiltin(std::string name, int arity) : name(std::move(name)), _arity(arity) {}
    int arity() const override { return _arity; }
    std::string toString() const override { return "[Built-in Function " + name + "]"; }
};

// Declarations of individual built-in functions (implementation in core_functions.cpp)
class PrintBuiltin : public MegaladonBuiltin {
public:
    PrintBuiltin() : MegaladonBuiltin("print", 1) {}
    MegaladonValue call(Interpreter& interpreter, const std::vector<MegaladonValue>& arguments) override;
};

class InputBuiltin : public MegaladonBuiltin {
public:
    InputBuiltin() : MegaladonBuiltin("input", 0) {}
    MegaladonValue call(Interpreter& interpreter, const std::vector<MegaladonValue>& arguments) override;
};

class LenBuiltin : public MegaladonBuiltin {
public:
    LenBuiltin() : MegaladonBuiltin("len", 1) {}
    MegaladonValue call(Interpreter& interpreter, const std::vector<MegaladonValue>& arguments) override;
};

// Global function to register all built-ins
void registerBuiltins(std::shared_ptr<Environment>& env);