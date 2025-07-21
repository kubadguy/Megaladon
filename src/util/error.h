#pragma once

#include <string>
#include <stdexcept> // For std::runtime_error
#include "../lexer/token.h" // For Token class

// Custom exception class for runtime errors in Megaladon
class MegaladonError : public std::runtime_error {
public:
    // Constructor for errors with a specific token
    MegaladonError(const Token& token, const std::string& message)
        : std::runtime_error(formatMessage(token, message)), token_(token), message_(message) {}

    // Constructor for general errors (e.g., compile-time, or without specific token)
    MegaladonError(const std::string& message)
        : std::runtime_error(message), token_(Token(TokenType::EOF_TOKEN, "", MegaladonValue(), 0)), message_(message) {} // Dummy token for general errors

    const Token& getToken() const { return token_; }
    const std::string& getErrorMessage() const { return message_; }


    // Static method for reporting errors (can be used for syntax errors during parsing)
    static void report(int line, const std::string& where, const std::string& message);
    static void report(const Token& token, const std::string& message);

    static bool hadError;
    static bool hadRuntimeError;

private:
    Token token_;
    std::string message_;

    static std::string formatMessage(const Token& token, const std::string& message);
};