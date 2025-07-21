// src/environment/environment.h
#pragma once

#include <string>
#include <map>
#include <memory> // For std::shared_ptr
#include "../types/value.h"

// Represents a scope for variables
class Environment {
public:
    // Constructor for global environment (no enclosing)
    Environment() : enclosing(nullptr) {}

    // Constructor for local environments (with enclosing scope)
    Environment(std::shared_ptr<Environment> enclosing) : enclosing(std::move(enclosing)) {}

    // Define a new variable in the current scope
    void define(const std::string& name, const MegaladonValue& value);

    // Get the value of a variable, searching enclosing scopes
    MegaladonValue get(const std::string& name);

    // Assign a new value to an existing variable, searching enclosing scopes
    void assign(const std::string& name, const MegaladonValue& value);

private:
    std::map<std::string, MegaladonValue> values;
    std::shared_ptr<Environment> enclosing; // Pointer to the parent environment
};