#pragma once

#include <vector>
#include <memory> // For std::shared_ptr
#include "../lexer/token.h" // For Token
#include "../types/value.h" // For MegaladonValue

// Forward declarations for visitors
template <typename T> class ExprVisitor;
template <typename T> class StmtVisitor;

// --- Expressions ---
class Expr {
public:
    virtual MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) = 0;
    virtual bool isLiteral() const { return false; } // Helper for type checking
    virtual ~Expr() = default;
};

class AssignExpr : public Expr, public std::enable_shared_from_this<AssignExpr> {
public:
    AssignExpr(Token name, std::shared_ptr<Expr> value)
        : name(std::move(name)), value(std::move(value)), distance(-1) {}
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;

    Token name;
    std::shared_ptr<Expr> value;
    int distance; // For resolution
};

class BinaryExpr : public Expr, public std::enable_shared_from_this<BinaryExpr> {
public:
    BinaryExpr(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;

    std::shared_ptr<Expr> left;
    Token op;
    std::shared_ptr<Expr> right;
};

class CallExpr : public Expr, public std::enable_shared_from_this<CallExpr> {
public:
    CallExpr(std::shared_ptr<Expr> callee, Token paren, std::vector<std::shared_ptr<Expr>> arguments)
        : callee(std::move(callee)), paren(std::move(paren)), arguments(std::move(arguments)) {}
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;

    std::shared_ptr<Expr> callee;
    Token paren; // The closing parenthesis token, for error reporting
    std::vector<std::shared_ptr<Expr>> arguments;
};

class GetExpr : public Expr, public std::enable_shared_from_this<GetExpr> {
public:
    // For properties like object.property
    GetExpr(std::shared_ptr<Expr> object, Token name)
        : object(std::move(object)), name(std::move(name)), index(nullptr) {}
    // For indexed access like list[index]
    GetExpr(std::shared_ptr<Expr> object, std::shared_ptr<Expr> index)
        : object(std::move(object)), index(std::move(index)), name(Token(TokenType::EOF_TOKEN, "", 0)) {} // Dummy token for list access
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;

    std::shared_ptr<Expr> object;
    Token name; // For property access (e.g., obj.prop)
    std::shared_ptr<Expr> index; // For indexed access (e.g., list[idx])
};


class GroupingExpr : public Expr, public std::enable_shared_from_this<GroupingExpr> {
public:
    GroupingExpr(std::shared_ptr<Expr> expression)
        : expression(std::move(expression)) {}
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;

    std::shared_ptr<Expr> expression;
};

class LiteralExpr : public Expr, public std::enable_shared_from_this<LiteralExpr> {
public:
    LiteralExpr(MegaladonValue value)
        : value(std::move(value)) {}
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;
    bool isLiteral() const override { return true; }

    MegaladonValue value;
};

class LogicalExpr : public Expr, public std::enable_shared_from_this<LogicalExpr> {
public:
    LogicalExpr(std::shared_ptr<Expr> left, Token op, std::shared_ptr<Expr> right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;

    std::shared_ptr<Expr> left;
    Token op;
    std::shared_ptr<Expr> right;
};

class SetExpr : public Expr, public std::enable_shared_from_this<SetExpr> {
public:
    // For property assignment: object.property = value
    SetExpr(std::shared_ptr<Expr> object, Token name, std::shared_ptr<Expr> value)
        : object(std::move(object)), name(std::move(name)), index(nullptr), value(std::move(value)) {}
    // For indexed assignment: list[index] = value
    SetExpr(std::shared_ptr<Expr> object, std::shared_ptr<Expr> index, std::shared_ptr<Expr> value)
        : object(std::move(object)), index(std::move(index)), value(std::move(value)), name(Token(TokenType::EOF_TOKEN, "", 0)) {} // Dummy token
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;

    std::shared_ptr<Expr> object;
    Token name; // For property assignment
    std::shared_ptr<Expr> index; // For indexed assignment
    std::shared_ptr<Expr> value;
};

class UnaryExpr : public Expr, public std::enable_shared_from_this<UnaryExpr> {
public:
    UnaryExpr(Token op, std::shared_ptr<Expr> right)
        : op(std::move(op)), right(std::move(right)) {}
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;

    Token op;
    std::shared_ptr<Expr> right;
};

class VariableExpr : public Expr, public std::enable_shared_from_this<VariableExpr> {
public:
    VariableExpr(Token name)
        : name(std::move(name)), distance(-1) {}
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;

    Token name;
    int distance; // For resolution
};

class ListExpr : public Expr, public std::enable_shared_from_this<ListExpr> {
public:
    ListExpr(std::vector<std::shared_ptr<Expr>> elements)
        : elements(std::move(elements)) {}
    MegaladonValue accept(ExprVisitor<MegaladonValue>& visitor) override;

    std::vector<std::shared_ptr<Expr>> elements;
};


// --- Expression Visitor ---
template <typename T>
class ExprVisitor {
public:
    virtual T visit(std::shared_ptr<AssignExpr> expr) = 0;
    virtual T visit(std::shared_ptr<BinaryExpr> expr) = 0;
    virtual T visit(std::shared_ptr<CallExpr> expr) = 0;
    virtual T visit(std::shared_ptr<GetExpr> expr) = 0;
    virtual T visit(std::shared_ptr<GroupingExpr> expr) = 0;
    virtual T visit(std::shared_ptr<LiteralExpr> expr) = 0;
    virtual T visit(std::shared_ptr<LogicalExpr> expr) = 0;
    virtual T visit(std::shared_ptr<SetExpr> expr) = 0;
    virtual T visit(std::shared_ptr<UnaryExpr> expr) = 0;
    virtual T visit(std::shared_ptr<VariableExpr> expr) = 0;
    virtual T visit(std::shared_ptr<ListExpr> expr) = 0; // New list expression
    virtual ~ExprVisitor() = default;
};

// --- Statements ---
class Stmt {
public:
    virtual void accept(StmtVisitor<void>& visitor) = 0;
    virtual ~Stmt() = default;
};

class BlockStmt : public Stmt, public std::enable_shared_from_this<BlockStmt> {
public:
    BlockStmt(std::vector<std::shared_ptr<Stmt>> statements)
        : statements(std::move(statements)) {}
    void accept(StmtVisitor<void>& visitor) override;

    std::vector<std::shared_ptr<Stmt>> statements;
};

class ExpressionStmt : public Stmt, public std::enable_shared_from_this<ExpressionStmt> {
public:
    ExpressionStmt(std::shared_ptr<Expr> expression)
        : expression(std::move(expression)) {}
    void accept(StmtVisitor<void>& visitor) override;

    std::shared_ptr<Expr> expression;
};

class FunctionStmt : public Stmt, public std::enable_shared_from_this<FunctionStmt> {
public:
    FunctionStmt(Token name, std::vector<Token> params, std::shared_ptr<BlockStmt> body)
        : name(std::move(name)), params(std::move(params)), body(std::move(body)) {}
    void accept(StmtVisitor<void>& visitor) override;

    Token name;
    std::vector<Token> params;
    std::shared_ptr<BlockStmt> body;
};


class IfStmt : public Stmt, public std::enable_shared_from_this<IfStmt> {
public:
    IfStmt(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> thenBranch, std::shared_ptr<Stmt> elseBranch)
        : condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}
    void accept(StmtVisitor<void>& visitor) override;

    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> thenBranch;
    std::shared_ptr<Stmt> elseBranch; // Can be nullptr
};

class PrintStmt : public Stmt, public std::enable_shared_from_this<PrintStmt> {
public:
    PrintStmt(std::shared_ptr<Expr> expression)
        : expression(std::move(expression)) {}
    void accept(StmtVisitor<void>& visitor) override;

    std::shared_ptr<Expr> expression;
};

class ReturnStmt : public Stmt, public std::enable_shared_from_this<ReturnStmt> {
public:
    ReturnStmt(Token keyword, std::shared_ptr<Expr> value)
        : keyword(std::move(keyword)), value(std::move(value)) {}
    void accept(StmtVisitor<void>& visitor) override;

    Token keyword;
    std::shared_ptr<Expr> value; // Can be nullptr for implicit return
};

class VarStmt : public Stmt, public std::enable_shared_from_this<VarStmt> {
public:
    VarStmt(Token name, std::shared_ptr<Expr> initializer)
        : name(std::move(name)), initializer(std::move(initializer)) {}
    void accept(StmtVisitor<void>& visitor) override;

    Token name;
    std::shared_ptr<Expr> initializer; // Can be nullptr
};

class WhileStmt : public Stmt, public std::enable_shared_from_this<WhileStmt> {
public:
    WhileStmt(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> body)
        : condition(std::move(condition)), body(std::move(body)) {}
    void accept(StmtVisitor<void>& visitor) override;

    std::shared_ptr<Expr> condition;
    std::shared_ptr<Stmt> body;
};


// --- Statement Visitor ---
template <typename T>
class StmtVisitor {
public:
    virtual T visit(std::shared_ptr<BlockStmt> stmt) = 0;
    virtual T visit(std::shared_ptr<ExpressionStmt> stmt) = 0;
    virtual T visit(std::shared_ptr<FunctionStmt> stmt) = 0;
    virtual T visit(std::shared_ptr<IfStmt> stmt) = 0;
    virtual T visit(std::shared_ptr<PrintStmt> stmt) = 0;
    virtual T visit(std::shared_ptr<ReturnStmt> stmt) = 0;
    virtual T visit(std::shared_ptr<VarStmt> stmt) = 0;
    virtual T visit(std::shared_ptr<WhileStmt> stmt) = 0;
    virtual ~StmtVisitor() = default;
};