// src/environment/environment.cpp
#include "environment.h"
#include "../util/error.h" // For runtime error reporting
#include <stdexcept>
#include <utility> // For std::move

void Environment::define(const std::string& name, const MegaladonValue& value) {
    // For now, allow redefinition in same scope (like Python), or disallow (like Java)
    // Here, it will overwrite if it exists, otherwise insert.
    values[name] = value;
}

MegaladonValue Environment::get(const std::string& name) {
    if (values.count(name)) {
        return values[name];
    }
    // If not found in current scope, check enclosing scope
    if (enclosing) {
        return enclosing->get(name);
    }
    throw std::runtime_error("Undefined variable '" + name + "'.");
}

void Environment::assign(const std::string& name, const MegaladonValue& value) {
    if (values.count(name)) {
        values[name] = value;
        return;
    }
    // If not found in current scope, try to assign in enclosing scope
    if (enclosing) {
        enclosing->assign(name, value);
        return;
    }
    throw std::runtime_error("Undefined variable '" + name + "'.");
}