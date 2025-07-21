// src/ast/ast.h
#pragma once

#include <vector>
#include <memory>
#include <string>
#include "../lexer/lexer.h" // For Token
#include "../types/value.h" // For MegaladonValue

// Forward declare Visitor
class Visitor;

// Base class for all AST nodes
struct Expr {
    virtual ~Expr() = default;
    virtual MegaladonValue accept(Visitor& visitor) = 0; // Expressions return a value
};

struct Stmt {
    virtual ~Stmt() = default;
    virtual void accept(Visitor& visitor) = 0; // Statements perform actions, return void
};

// --- Expressions ---
struct BinaryExpr : public Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> l, Token o, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}

    MegaladonValue accept(Visitor& visitor) override;
};

struct GroupingExpr : public Expr {
    std::unique_ptr<Expr> expression;

    GroupingExpr(std::unique_ptr<Expr> expr)
        : expression(std::move(expr)) {}

    MegaladonValue accept(Visitor& visitor) override;
};

struct LiteralExpr : public Expr {
    MegaladonValue value; // Use MegaladonValue to store literals directly

    LiteralExpr(MegaladonValue val) : value(std::move(val)) {}

    MegaladonValue accept(Visitor& visitor) override;
};

struct UnaryExpr : public Expr {
    Token op;
    std::unique_ptr<Expr> right;

    UnaryExpr(Token o, std::unique_ptr<Expr> r)
        : op(std::move(o)), right(std::move(r)) {}

    MegaladonValue accept(Visitor& visitor) override;
};

struct VariableExpr : public Expr {
    Token name;

    VariableExpr(Token n) : name(std::move(n)) {}

    MegaladonValue accept(Visitor& visitor) override;
};

struct AssignExpr : public Expr {
    Token name;
    std::unique_ptr<Expr> value;

    AssignExpr(Token n, std::unique_ptr<Expr> v)
        : name(std::move(n)), value(std::move(v)) {}

    MegaladonValue accept(Visitor& visitor) override;
};

struct LogicalExpr : public Expr {
    std::unique_ptr<Expr> left;
    Token op; // AND or OR
    std::unique_ptr<Expr> right;

    LogicalExpr(std::unique_ptr<Expr> l, Token o, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}

    MegaladonValue accept(Visitor& visitor) override;
};

struct CallExpr : public Expr {
    std::unique_ptr<Expr> callee;
    Token paren; // The '(' token
    std::vector<std::unique_ptr<Expr>> arguments;

    CallExpr(std::unique_ptr<Expr> c, Token p, std::vector<std::unique_ptr<Expr>> args)
        : callee(std::move(c)), paren(std::move(p)), arguments(std::move(args)) {}

    MegaladonValue accept(Visitor& visitor) override;
};

struct GetExpr : public Expr {
    std::unique_ptr<Expr> object;
    Token name; // The method/property name

    GetExpr(std::unique_ptr<Expr> obj, Token n) : object(std::move(obj)), name(std::move(n)) {}

    MegaladonValue accept(Visitor& visitor) override;
};

struct SetExpr : public Expr {
    std::unique_ptr<Expr> object;
    Token name;
    std::unique_ptr<Expr> value;

    SetExpr(std::unique_ptr<Expr> obj, Token n, std::unique_ptr<Expr> v)
        : object(std::move(obj)), name(std::move(n)), value(std::move(v)) {}

    MegaladonValue accept(Visitor& visitor) override;
};

struct ListExpr : public Expr {
    std::vector<std::unique_ptr<Expr>> elements;

    ListExpr(std::vector<std::unique_ptr<Expr>> elems) : elements(std::move(elems)) {}

    MegaladonValue accept(Visitor& visitor) override;
};

struct IndexExpr : public Expr {
    std::unique_ptr<Expr> object; // The list/string being indexed
    std::unique_ptr<Expr> index;  // The index expression

    IndexExpr(std::unique_ptr<Expr> obj, std::unique_ptr<Expr> idx)
        : object(std::move(obj)), index(std::move(idx)) {}

    MegaladonValue accept(Visitor& visitor) override;
};

struct IndexAssignExpr : public Expr {
    std::unique_ptr<Expr> object; // The list/string being indexed
    std::unique_ptr<Expr> index;  // The index expression
    std::unique_ptr<Expr> value;  // The value to assign

    IndexAssignExpr(std::unique_ptr<Expr> obj, std::unique_ptr<Expr> idx, std::unique_ptr<Expr> val)
        : object(std::move(obj)), index(std::move(idx)), value(std::move(val)) {}

    MegaladonValue accept(Visitor& visitor) override;
};


// --- Statements ---
struct ExpressionStmt : public Stmt {
    std::unique_ptr<Expr> expression;

    ExpressionStmt(std::unique_ptr<Expr> expr)
        : expression(std::move(expr)) {}

    void accept(Visitor& visitor) override;
};

struct PrintStmt : public Stmt {
    std::unique_ptr<Expr> expression;

    PrintStmt(std::unique_ptr<Expr> expr)
        : expression(std::move(expr)) {}

    void accept(Visitor& visitor) override;
};

struct VarDeclStmt : public Stmt {
    Token name;
    std::unique_ptr<Expr> initializer; // Can be nullptr for uninitialized vars

    VarDeclStmt(Token n, std::unique_ptr<Expr> init)
        : name(std::move(n)), initializer(std::move(init)) {}

    void accept(Visitor& visitor) override;
};

struct BlockStmt : public Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;

    BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts)
        : statements(std::move(stmts)) {}

    void accept(Visitor& visitor) override;
};

struct IfStmt : public Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch; // Can be nullptr

    IfStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> thenB, std::unique_ptr<Stmt> elseB)
        : condition(std::move(cond)), thenBranch(std::move(thenB)), elseBranch(std::move(elseB)) {}

    void accept(Visitor& visitor) override;
};

struct WhileStmt : public Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;

    WhileStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> b)
        : condition(std::move(cond)), body(std::move(b)) {}

    void accept(Visitor& visitor) override;
};

struct FunctionStmt : public Stmt {
    Token name;
    std::vector<Token> params;
    std::unique_ptr<BlockStmt> body;

    FunctionStmt(Token n, std::vector<Token> p, std::unique_ptr<BlockStmt> b)
        : name(std::move(n)), params(std::move(p)), body(std::move(b)) {}

    void accept(Visitor& visitor) override;
};

struct ReturnStmt : public Stmt {
    Token keyword;
    std::unique_ptr<Expr> value; // Can be nullptr for void return

    ReturnStmt(Token k, std::unique_ptr<Expr> val)
        : keyword(std::move(k)), value(std::move(val)) {}

    void accept(Visitor& visitor) override;
};

// --- Visitor Base Class ---
// This uses the Visitor pattern to traverse the AST.
// The Interpreter will implement this interface.
class Visitor {
public:
    virtual MegaladonValue visit(BinaryExpr& expr) = 0;
    virtual MegaladonValue visit(GroupingExpr& expr) = 0;
    virtual MegaladonValue visit(LiteralExpr& expr) = 0;
    virtual MegaladonValue visit(UnaryExpr& expr) = 0;
    virtual MegaladonValue visit(VariableExpr& expr) = 0;
    virtual MegaladonValue visit(AssignExpr& expr) = 0;
    virtual MegaladonValue visit(LogicalExpr& expr) = 0;
    virtual MegaladonValue visit(CallExpr& expr) = 0;
    virtual MegaladonValue visit(GetExpr& expr) = 0;
    virtual MegaladonValue visit(SetExpr& expr) = 0;
    virtual MegaladonValue visit(ListExpr& expr) = 0;
    virtual MegaladonValue visit(IndexExpr& expr) = 0;
    virtual MegaladonValue visit(IndexAssignExpr& expr) = 0;

    virtual void visit(ExpressionStmt& stmt) = 0;
    virtual void visit(PrintStmt& stmt) = 0;
    virtual void visit(VarDeclStmt& stmt) = 0;
    virtual void visit(BlockStmt& stmt) = 0;
    virtual void visit(IfStmt& stmt) = 0;
    virtual void visit(WhileStmt& stmt) = 0;
    virtual void visit(FunctionStmt& stmt) = 0;
    virtual void visit(ReturnStmt& stmt) = 0;
};