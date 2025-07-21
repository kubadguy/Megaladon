#pragma once

#include <vector>
#include <memory> // For std::shared_ptr
#include <stdexcept> // For std::runtime_error

#include "../ast/ast.h"         // Contains all Expr and Stmt declarations
#include "../environment/environment.h" // For Environment
#include "../types/value.h"      // For MegaladonValue

// A custom exception to unwind the stack for 'return' statements
class ReturnValue : public std::runtime_error {
public:
    MegaladonValue value;
    ReturnValue(MegaladonValue value) : std::runtime_error(""), value(std::move(value)) {}
};

class Interpreter : public ExprVisitor<MegaladonValue>, public StmtVisitor<void> {
public:
    Interpreter();

    void interpret(const std::vector<std::shared_ptr<Stmt>>& statements);

    // StmtVisitor methods
    void visit(std::shared_ptr<ExpressionStmt> stmt) override;
    void visit(std::shared_ptr<PrintStmt> stmt) override;
    void visit(std::shared_ptr<VarStmt> stmt) override;
    void visit(std::shared_ptr<BlockStmt> stmt) override;
    void visit(std::shared_ptr<IfStmt> stmt) override;
    void visit(std::shared_ptr<WhileStmt> stmt) override;
    void visit(std::shared_ptr<FunctionStmt> stmt) override;
    void visit(std::shared_ptr<ReturnStmt> stmt) override;


    // ExprVisitor methods
    MegaladonValue visit(std::shared_ptr<AssignExpr> expr) override;
    MegaladonValue visit(std::shared_ptr<BinaryExpr> expr) override;
    MegaladonValue visit(std::shared_ptr<CallExpr> expr) override;
    MegaladonValue visit(std::shared_ptr<GetExpr> expr) override;
    MegaladonValue visit(std::shared_ptr<GroupingExpr> expr) override;
    MegaladonValue visit(std::shared_ptr<LiteralExpr> expr) override;
    MegaladonValue visit(std::shared_ptr<LogicalExpr> expr) override;
    MegaladonValue visit(std::shared_ptr<SetExpr> expr) override;
    MegaladonValue visit(std::shared_ptr<UnaryExpr> expr) override;
    MegaladonValue visit(std::shared_ptr<VariableExpr> expr) override;
    MegaladonValue visit(std::shared_ptr<ListExpr> expr) override;


    // Public access for evaluating expressions (used by statements)
    MegaladonValue evaluate(std::shared_ptr<Expr> expr);
    void execute(std::shared_ptr<Stmt> stmt); // Public because main needs to call it

    // Public for function calls to execute a block
    void executeBlock(const std::vector<std::shared_ptr<Stmt>>& statements, std::shared_ptr<Environment> new_environment);


    std::shared_ptr<Environment> globals; // Global environment
    std::shared_ptr<Environment> environment; // Current active environment

private:
    // Helper methods
    void checkNumberOperand(const Token& op, const MegaladonValue& operand);
    void checkNumberOperands(const Token& op, const MegaladonValue& left, const MegaladonValue& right);
    bool isTruthy(const MegaladonValue& value);
    bool isEqual(const MegaladonValue& a, const MegaladonValue& b);
};