// src/builtins/core_functions.cpp
#include "builtins.h"
#include "../interpreter/interpreter.h" // For Interpreter context
#include "../util/error.h" // For runtime error reporting

#include <iostream>
#include <string>
#include <chrono> // For future time functions
#include <random> // For future random functions

// --- Built-in Function Implementations ---

MegaladonValue PrintBuiltin::call(Interpreter& interpreter, const std::vector<MegaladonValue>& arguments) {
    std::cout << arguments[0].toString() << std::endl;
    return MegaladonValue(); // Void
}

MegaladonValue InputBuiltin::call(Interpreter& interpreter, const std::vector<MegaladonValue>& arguments) {
    std::string line;
    std::getline(std::cin, line);
    return MegaladonValue(line);
}

MegaladonValue LenBuiltin::call(Interpreter& interpreter, const std::vector<MegaladonValue>& arguments) {
    if (arguments.size() != 1) {
        ReportRuntimeError("Built-in 'len' expects 1 argument.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
    const MegaladonValue& arg = arguments[0];
    if (arg.isString()) {
        return MegaladonValue(static_cast<double>(arg.asString().length()));
    } else if (arg.isList()) {
        return MegaladonValue(static_cast<double>(arg.asList().size()));
    } else {
        ReportRuntimeError("Built-in 'len' argument must be a string or a list.");
        return MegaladonValue(MegaladonValue::INVALID);
    }
}


// --- Register All Built-ins ---
void registerBuiltins(std::shared_ptr<Environment>& env) {
    // Use std::shared_ptr for MegaladonBuiltin instances
    env->define("print", MegaladonValue(std::make_shared<PrintBuiltin>()));
    env->define("input", MegaladonValue(std::make_shared<InputBuiltin>()));
    env->define("len", MegaladonValue(std::make_shared<LenBuiltin>()));

    // string_methods.cpp and list_methods.cpp contain implementations
    // that are called via MegaladonValue::callMethod.
    // They are not directly registered in the global environment, but associated
    // with the MegaladonValue types themselves.
}