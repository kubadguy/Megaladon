// src/ast/ast.cpp
#include "ast.h"
#include "../interpreter/interpreter.h" // For Visitor class access. Note: Recursive include.
                                        // This is a common pattern for Visitor, but ensure
                                        // Interpreter is only forward-declared in ast.h
                                        // if not strictly needed there.
                                        // Here it's needed for the accept methods.

// Expression accept methods
MegaladonValue BinaryExpr::accept(Visitor& visitor) {
    return visitor.visit(*this);
}

MegaladonValue GroupingExpr::accept(Visitor& visitor) {
    return visitor.visit(*this);
}

MegaladonValue LiteralExpr::accept(Visitor& visitor) {
    return visitor.visit(*this);
}

MegaladonValue UnaryExpr::accept(Visitor& visitor) {
    return visitor.visit(*this);
}

MegaladonValue VariableExpr::accept(Visitor& visitor) {
    return visitor.visit(*this);
}

MegaladonValue AssignExpr::accept(Visitor& visitor) {
    return visitor.visit(*this);
}

MegaladonValue LogicalExpr::accept(Visitor& visitor) {
    return visitor.visit(*this);
}

MegaladonValue CallExpr::accept(Visitor& visitor) {
    return visitor.visit(*this);
}

MegaladonValue GetExpr::accept(Visitor& visitor) {
    return visitor.visit(*this);
}

MegaladonValue SetExpr::accept(Visitor& visitor) {
    return visitor.visit(*this);
}

MegaladonValue ListExpr::accept(Visitor& visitor) {
    return visitor.visit(*this);
}

MegaladonValue IndexExpr::accept(Visitor& visitor) {
    return visitor.visit(*this);
}

MegaladonValue IndexAssignExpr::accept(Visitor& visitor) {
    return visitor.visit(*this);
}

// Statement accept methods
void ExpressionStmt::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void PrintStmt::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void VarDeclStmt::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void BlockStmt::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void IfStmt::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void WhileStmt::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void FunctionStmt::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void ReturnStmt::accept(Visitor& visitor) {
    visitor.visit(*this);
}