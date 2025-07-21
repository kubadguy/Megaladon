// src/util/error.cpp
#include "error.h"

bool hadError = false;
bool hadRuntimeError = false;

void ReportError(int line, int column, const std::string& where, const std::string& message) {
    std::cerr << "[Line " << line << ", Col " << column << "] Error" << where << ": " << message << std::endl;
    hadError = true;
}

void ReportRuntimeError(const std::string& message) {
    std::cerr << "Runtime Error: " << message << std::endl;
    hadRuntimeError = true;
}

void ResetErrors() {
    hadError = false;
    hadRuntimeError = false;
}