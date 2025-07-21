// src/util/error.h
#pragma once

#include <string>
#include <iostream>

// Global flag to indicate if an error occurred during scanning/parsing/runtime
extern bool hadError;
extern bool hadRuntimeError;

// Function to report syntax/lexical errors
void ReportError(int line, int column, const std::string& where, const std::string& message);

// Function to report runtime errors
void ReportRuntimeError(const std::string& message);

// Reset error flags
void ResetErrors();