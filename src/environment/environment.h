#pragma once

#include <string>
#include <map>
#include <memory> // For std::shared_ptr, std::enable_shared_from_this
#include "../types/value.h" // For MegaladonValue
#include "../lexer/token.h" // For Token

class Environment : public std::enable_shared_from_this<Environment> {
public:
    Environment();
    Environment(std::shared_ptr<Environment> enclosing);

    void define(const std::string& name, const MegaladonValue& value);
    MegaladonValue get(const Token& name);
    void assign(const Token& name, const MegaladonValue& value);

    // For local variable resolution
    MegaladonValue getAt(int distance, const std::string& name);
    void assignAt(int distance, const Token& name, const MegaladonValue& value);

    std::shared_ptr<Environment> ancestor(int distance);

private:
    std::map<std::string, MegaladonValue> values;
    std::shared_ptr<Environment> enclosing; // Pointer to the parent environment
};