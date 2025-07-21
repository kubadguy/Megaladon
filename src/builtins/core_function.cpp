#include "builtins.h"
#include "../types/value.h"
#include "../interpreter/interpreter.h" // For Interpreter access in call methods
#include <iostream>
#include <string>
#include <cmath> // For std::fmod

// --- PrintBuiltin ---
MegaladonValue PrintBuiltin::call(Interpreter& interpreter, const std::vector<MegaladonValue>& arguments) {
    // Suppress unused parameter warning for 'interpreter'
    (void)interpreter;

    if (arguments.empty()) {
        std::cout << "void\n"; // Changed from VOID to 'void' string
    } else {
        std::cout << arguments[0].toString() << "\n";
    }
    return MegaladonValue(); // VOID
}

// --- InputBuiltin ---
MegaladonValue InputBuiltin::call(Interpreter& interpreter, const std::vector<MegaladonValue>& arguments) {
    // Suppress unused parameter warnings
    (void)interpreter;
    // You might want to use arguments[0] as a prompt if provided
    if (arguments.size() > 0 && arguments[0].isString()) {
        std::cout << arguments[0].asString();
    }

    std::string line;
    std::getline(std::cin, line);
    return MegaladonValue(line);
}

// --- LenBuiltin ---
MegaladonValue LenBuiltin::call(Interpreter& interpreter, const std::vector<MegaladonValue>& arguments) {
    // Suppress unused parameter warning for 'interpreter'
    (void)interpreter;

    if (arguments.size() != 1) {
        throw std::runtime_error("MegaladonError: len() expects 1 argument.");
    }

    const MegaladonValue& arg = arguments[0];
    if (arg.isString()) {
        return MegaladonValue(static_cast<double>(arg.asString().length()));
    } else if (arg.isList()) {
        return MegaladonValue(static_cast<double>(arg.asList().size()));
    } else {
        throw std::runtime_error("MegaladonError: len() argument must be a string or a list.");
    }
}

// --- Register Built-ins ---
void registerBuiltins(std::shared_ptr<Environment>& env) {
    env->define("print", MegaladonValue(std::make_shared<PrintBuiltin>()));
    env->define("input", MegaladonValue(std::make_shared<InputBuiltin>()));
    env->define("len", MegaladonValue(std::make_shared<LenBuiltin>()));
    // Add other built-in functions here
}