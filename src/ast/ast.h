#ifndef MEGALADON_AST_H
#define MEGALADON_AST_H

#include <memory>
#include <vector>
#include "../lexer/token.h"
#include "../types/value.h" // Assuming MegaladonValue is defined here

// Forward declarations for visitors
template <typename T>
class ExprVisitor;
template <typename T>
class StmtVisitor;

// Forward declarations for expressions and statements for shared_ptr
class Expr;
class AssignExpr;
class BinaryExpr;
class CallExpr;
class GetExpr;
class GroupingExpr;
class LiteralExpr;
class LogicalExpr;
class SetExpr;
class UnaryExpr;
class VariableExpr;
class ListExpr; // New expression type for lists

class Stmt;
class BlockStmt;
class ExpressionStmt;
class FunctionStmt;
class IfStmt;
class PrintStmt;
class ReturnStmt;
class VarStmt;
class WhileStmt;


// --- Expressions ---
class Expr : public std::enable_shared_from_this<Expr> {
public:
    virtual ~Expr() = default;
    virtual MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) = 0;
};

class AssignExpr : public Expr, public std::enable_shared_from_this<AssignExpr> {
public:
    AssignExpr(Token name, std::shared_ptr<Expr> value)
        : name(name), value(value) {}
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;
    Token name;
    std::shared_ptr<Expr> value;
};

class BinaryExpr : public Expr, public std::enable_shared_from_this<BinaryExpr> {
public:
    BinaryExpr(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right)
        : left(left), op(op), right(right) {}
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;
    std::shared_ptr<Expr> left;
    Token op;
    std::shared_ptr<Expr> right;
};

class CallExpr : public Expr, public std::enable_shared_from_this<CallExpr> {
public:
    CallExpr(std::shared_ptr<Expr> callee, Token paren, std::vector<std::shared_ptr<Expr>> arguments)
        : callee(callee), paren(paren), arguments(arguments) {}
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;
    std::shared_ptr<Expr> callee;
    Token paren;
    std::vector<std::shared_ptr<Expr>> arguments;
};

class GetExpr : public Expr, public std::enable_shared_from_this<GetExpr> {
public:
    // IMPORTANT: Members are initialized in declaration order to avoid -Wreorder warnings
    GetExpr(std::shared_ptr<Expr> object, std::shared_ptr<Expr> index)
        : object(object), name(Token()), index(index) {} // Initialize 'name' with a default Token
    // Constructor for property access (e.g., obj.prop) - if you add this back later
    // GetExpr(std::shared_ptr<Expr> object, Token name)
    //     : object(object), name(name), index(nullptr) {} // Initialize 'index' with nullptr
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;
    std::shared_ptr<Expr> object;
    Token name; // For property access (e.g., obj.prop)
    std::shared_ptr<Expr> index; // For indexed access (e.g., list[idx])
};

class GroupingExpr : public Expr, public std::enable_shared_from_this<GroupingExpr> {
public:
    GroupingExpr(std::shared_ptr<Expr> expression)
        : expression(expression) {}
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;
    std::shared_ptr<Expr> expression;
};

class LiteralExpr : public Expr, public std::enable_shared_from_this<LiteralExpr> {
public:
    LiteralExpr(MegaladonValue value)
        : value(value) {}
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;
    MegaladonValue value;
};

class LogicalExpr : public Expr, public std::enable_shared_from_this<LogicalExpr> {
public:
    LogicalExpr(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right)
        : left(left), op(op), right(right) {}
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;
    std::shared_ptr<Expr> left;
    Token op;
    std::shared_ptr<Expr> right;
};

class SetExpr : public Expr, public std::enable_shared_from_this<SetExpr> {
public:
    // IMPORTANT: Members are initialized in declaration order to avoid -Wreorder warnings
    SetExpr(std::shared_ptr<Expr> object, std::shared_ptr<Expr> index, std::shared_ptr<Expr> value)
        : object(object), name(Token()), index(index), value(value) {} // Initialize 'name' with a default Token
    // Constructor for property assignment (e.g., obj.prop = val) - if you add this back later
    // SetExpr(std::shared_ptr<Expr> object, Token name, std::shared_ptr<Expr> value)
    //     : object(object), name(name), index(nullptr), value(value) {} // Initialize 'index' with nullptr
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;
    std::shared_ptr<Expr> object;
    Token name; // For property assignment
    std::shared_ptr<Expr> index; // For indexed assignment
    std::shared_ptr<Expr> value;
};

class UnaryExpr : public Expr, public std::enable_shared_from_this<UnaryExpr> {
public:
    UnaryExpr(Token op, std::shared_ptr<Expr> right)
        : op(op), right(right) {}
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;
    Token op;
    std::shared_ptr<Expr> right;
};

class VariableExpr : public Expr, public std::enable_shared_from_this<VariableExpr> {
public:
    VariableExpr(Token name)
        : name(name) {}
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;
    Token name;
};

class ListExpr : public Expr, public std::enable_shared_from_this<ListExpr> {
public:
    ListExpr(std::vector<std::shared_ptr<Expr>> elements)
        : elements(elements) {}
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;
    std::vector<std::shared_ptr<Expr>> elements;
};

// --- Statements ---
class Stmt : public std::enable_shared_from_this<Stmt> {
public:
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor<void>& visitor) = 0;
};

class BlockStmt : public Stmt, public std::enable_shared_from_this<BlockStmt> {
public:
    BlockStmt(std::vector<std::shared_ptr<Stmt>> statements)
        : statements(statements) {}
    void accept(StmtVisitor<void>& visitor) override;
    std::vector<std::shared_ptr<Stmt>> statements;
};

class ExpressionStmt : public Stmt, public std::enable_shared_from_this<ExpressionStmt> {
public:
    ExpressionStmt(std::shared_ptr<Expr> expression)
        : expression(expression) {}
    void accept(StmtVisitor<void>& visitor) override;
    std::shared_ptr<Expr> expression;
};

class FunctionStmt : public Stmt, public std::enable_shared_from_this<FunctionStmt> {
public:
    FunctionStmt(Token name, std::vector<Token> params, std::vector<std::shared_ptr<Stmt>> body)
        : name(name), params(params), body(body) {}
    void accept(StmtVisitor<void>& visitor) override;
    Token name;
    std::vector<Token> params;
    std::vector<std::shared_ptr<Stmt>> body;
};

class IfStmt : public Stmt, public std::enable_shared_from_this<IfStmt> {
public:
    IfStmt(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> thenBranch, std::shared_ptr<Stmt> elseBranch)
        : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}
    void accept(StmtVisitor<void>& visitor) override;
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> thenBranch;
    std::shared_ptr<Stmt> elseBranch;
};

class PrintStmt : public Stmt, public std::enable_shared_from_this<PrintStmt> {
public:
    PrintStmt(std::shared_ptr<Expr> expression)
        : expression(expression) {}
    void accept(StmtVisitor<void>& visitor) override;
    std::shared_ptr<Expr> expression;
};

class ReturnStmt : public Stmt, public std::enable_shared_from_this<ReturnStmt> {
public:
    ReturnStmt(Token keyword, std::shared_ptr<Expr> value)
        : keyword(keyword), value(value) {}
    void accept(StmtVisitor<void>& visitor) override;
    Token keyword;
    std::shared_ptr<Expr> value;
};

class VarStmt : public Stmt, public std::enable_shared_from_this<VarStmt> {
public:
    VarStmt(Token name, std::shared_ptr<Expr> initializer)
        : name(name), initializer(initializer) {}
    void accept(StmtVisitor<void>& visitor) override;
    Token name;
    std::shared_ptr<Expr> initializer; // Can be nullptr if no initializer
};

class WhileStmt : public Stmt, public std::enable_shared_from_this<WhileStmt> {
public:
    WhileStmt(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> body)
        : condition(condition), body(body) {}
    void accept(StmtVisitor<void>& visitor) override;
    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> body;
};

#endif // MEGALADON_AST_H