// src/interpreter/interpreter.h
#pragma once

#include <vector>
#include <memory>
#include <map>
#include "../ast/ast.h"
#include "../types/value.h"
#include "../environment/environment.h"
#include "../builtins/builtins.h" // For MegaladonCallable and built-in function registrations

// MegaladonCallable is forward declared in value.h. Define it here.
// Base class for all callable entities (functions, built-ins)
class MegaladonCallable {
public:
    virtual ~MegaladonCallable() = default;
    virtual MegaladonValue call(class Interpreter& interpreter, const std::vector<MegaladonValue>& arguments) = 0;
    virtual int arity() const = 0; // Number of expected arguments
    virtual std::string toString() const { return "[Callable]"; }
};

// Represents a user-defined function in Megaladon
class MegaladonFunction : public MegaladonCallable {
public:
    const FunctionStmt& declaration;
    std::shared_ptr<Environment> closure; // The environment where the function was declared

    MegaladonFunction(const FunctionStmt& declaration, std::shared_ptr<Environment> closure)
        : declaration(declaration), closure(std::move(closure)) {}

    MegaladonValue call(class Interpreter& interpreter, const std::vector<MegaladonValue>& arguments) override;
    int arity() const override { return declaration.params.size(); }
    std::string toString() const override { return "[Function " + declaration.name.lexeme + "]"; }
};


// Interpreter implements the Visitor pattern for AST traversal
class Interpreter : public Visitor {
public:
    Interpreter();
    void interpret(const std::vector<std::unique_ptr<Stmt>>& statements);

    // Expression Visitors
    MegaladonValue visit(BinaryExpr& expr) override;
    MegaladonValue visit(GroupingExpr& expr) override;
    MegaladonValue visit(LiteralExpr& expr) override;
    MegaladonValue visit(UnaryExpr& expr) override;
    MegaladonValue visit(VariableExpr& expr) override;
    MegaladonValue visit(AssignExpr& expr) override;
    MegaladonValue visit(LogicalExpr& expr) override;
    MegaladonValue visit(CallExpr& expr) override;
    MegaladonValue visit(GetExpr& expr) override;
    MegaladonValue visit(SetExpr& expr) override;
    MegaladonValue visit(ListExpr& expr) override;
    MegaladonValue visit(IndexExpr& expr) override;
    MegaladonValue visit(IndexAssignExpr& expr) override;

    // Statement Visitors
    void visit(ExpressionStmt& stmt) override;
    void visit(PrintStmt& stmt) override;
    void visit(VarDeclStmt& stmt) override;
    void visit(BlockStmt& stmt) override;
    void visit(IfStmt& stmt) override;
    void visit(WhileStmt& stmt) override;
    void visit(FunctionStmt& stmt) override;
    void visit(ReturnStmt& stmt) override;

    // Helper method to execute blocks with their own environment
    void executeBlock(const std::vector<std::unique_ptr<Stmt>>& statements, std::shared_ptr<Environment> environment);

private:
    MegaladonValue evaluate(Expr& expr);
    void execute(Stmt& stmt);

    // Convert value to boolean for control flow
    bool isTruthy(const MegaladonValue& value);

    // Check if operands are numbers
    void checkNumberOperand(const Token& op, const MegaladonValue& operand);
    void checkNumberOperands(const Token& op, const MegaladonValue& left, const MegaladonValue& right);

    std::shared_ptr<Environment> globalEnvironment;
    std::shared_ptr<Environment> currentEnvironment; // Points to the currently active environment

    // Special exception for 'return' to unwind the call stack
    struct ReturnValue : public std::runtime_error {
        MegaladonValue value;
        ReturnValue(MegaladonValue val) : std::runtime_error(""), value(std::move(val)) {}
    };
};