#include "error.h"
#include <iostream>

bool MegaladonError::hadError = false;
bool MegaladonError::hadRuntimeError = false;

std::string MegaladonError::formatMessage(const Token& token, const std::string& message) {
    std::string error_msg = "[line " + std::to_string(token.line) + "] Error";
    if (token.type == TokenType::EOF_TOKEN) {
        error_msg += " at end";
    } else if (!token.lexeme.empty()) {
        error_msg += " at '" + token.lexeme + "'";
    }
    error_msg += ": " + message;
    return error_msg;
}

void MegaladonError::report(int line, const std::string& where, const std::string& message) {
    std::cerr << "[line " << line << "] Error" << where << ": " << message << std::endl;
    hadError = true;
}

void MegaladonError::report(const Token& token, const std::string& message) {
    std::cerr << MegaladonError::formatMessage(token, message) << std::endl;
    hadError = true;
}