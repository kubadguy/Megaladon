#pragma once

#include "../ast/ast.h"
#include "../environment/environment.h"
#include "../types/value.h"
#include "../util/error.h"
#include <stdexcept> // Required for std::runtime_error
#include <vector>
#include <string>
#include <memory>
#include <iostream> // For I/O in builtins, if any here

// Forward declare AST nodes to prevent circular includes if they include interpreter.h
class Stmt;
class Expr;
class BlockStmt; // Add other specific AST node forward declarations if necessary
class ExpressionStmt;
class PrintStmt;
class VarStmt;
class AssignExpr;
class BinaryExpr;
class CallExpr;
class GroupingExpr;
class LiteralExpr;
class LogicalExpr;
class UnaryExpr;
class VariableExpr;
class IfStmt;
class WhileStmt;
class FunctionStmt;
class ReturnStmt;
class ListExpr;
class GetExpr;
class SetExpr;


// Custom exception for control flow (like return statements)
// Inherit from std::runtime_error
struct ReturnValue : public std::runtime_error {
    MegaladonValue value;
    // Call base class constructor and then initialize value
    ReturnValue(MegaladonValue val) : std::runtime_error(""), value(std::move(val)) {}
};

class Interpreter {
public:
    Interpreter();
    void interpret(const std::vector<std::shared_ptr<Stmt>>& statements);
    void executeBlock(const std::vector<std::shared_ptr<Stmt>>& statements, std::shared_ptr<Environment> environment);

private:
    std::shared_ptr<Environment> globals;
    std::shared_ptr<Environment> environment; // Current active environment

    // Visitor pattern implementation
    MegaladonValue evaluate(std::shared_ptr<Expr> expr);
    void execute(std::shared_ptr<Stmt> stmt);

    // Expression visitors
    MegaladonValue visit(std::shared_ptr<AssignExpr> expr);
    MegaladonValue visit(std::shared_ptr<BinaryExpr> expr);
    MegaladonValue visit(std::shared_ptr<CallExpr> expr);
    MegaladonValue visit(std::shared_ptr<GroupingExpr> expr);
    MegaladonValue visit(std::shared_ptr<LiteralExpr> expr);
    MegaladonValue visit(std::shared_ptr<LogicalExpr> expr);
    MegaladonValue visit(std::shared_ptr<UnaryExpr> expr);
    MegaladonValue visit(std::shared_ptr<VariableExpr> expr);
    MegaladonValue visit(std::shared_ptr<ListExpr> expr);
    MegaladonValue visit(std::shared_ptr<GetExpr> expr);
    MegaladonValue visit(std::shared_ptr<SetExpr> expr);


    // Statement visitors
    void visit(std::shared_ptr<ExpressionStmt> stmt);
    void visit(std::shared_ptr<PrintStmt> stmt);
    void visit(std::shared_ptr<VarStmt> stmt);
    void visit(std::shared_ptr<BlockStmt> stmt);
    void visit(std::shared_ptr<IfStmt> stmt);
    void visit(std::shared_ptr<WhileStmt> stmt);
    void visit(std::shared_ptr<FunctionStmt> stmt);
    void visit(std::shared_ptr<ReturnStmt> stmt);


    // Helper for type checking
    void checkNumberOperand(const Token& op, const MegaladonValue& operand);
    void checkNumberOperands(const Token& op, const MegaladonValue& left, const MegaladonValue& right);

    bool isTruthy(const MegaladonValue& value);
    bool isEqual(const MegaladonValue& a, const MegaladonValue& b);
};