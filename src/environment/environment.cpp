#include "environment.h"
#include "../util/error.h" // Assuming MegaladonError is defined here

Environment::Environment() : enclosing(nullptr) {}

Environment::Environment(std::shared_ptr<Environment> enclosing) : enclosing(std::move(enclosing)) {}

void Environment::define(const std::string& name, const MegaladonValue& value) {
    values[name] = value;
}

MegaladonValue Environment::get(const Token& name) {
    if (values.count(name.lexeme)) {
        return values.at(name.lexeme);
    }

    if (enclosing != nullptr) {
        return enclosing->get(name);
    }

    throw MegaladonError(name, "Undefined variable '" + name.lexeme + "'.");
}

void Environment::assign(const Token& name, const MegaladonValue& value) {
    if (values.count(name.lexeme)) {
        values[name.lexeme] = value;
        return;
    }

    if (enclosing != nullptr) {
        enclosing->assign(name, value);
        return;
    }

    throw MegaladonError(name, "Undefined variable '" + name.lexeme + "'.");
}

MegaladonValue Environment::getAt(int distance, const std::string& name) {
    return ancestor(distance)->values.at(name);
}

void Environment::assignAt(int distance, const Token& name, const MegaladonValue& value) {
    ancestor(distance)->values[name.lexeme] = value;
}

std::shared_ptr<Environment> Environment::ancestor(int distance) {
    std::shared_ptr<Environment> environment = shared_from_this(); // Get shared_ptr to current object
    for (int i = 0; i < distance; ++i) {
        if (!environment->enclosing) {
            // This should ideally not happen if resolution is correct
            throw std::runtime_error("MegaladonError: Internal error: Attempted to access an ancestor environment beyond scope.");
        }
        environment = environment->enclosing;
    }
    return environment;
}