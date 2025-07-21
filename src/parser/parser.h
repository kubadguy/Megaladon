#pragma once

#include <vector>
#include <memory> // For std::shared_ptr
#include <stdexcept> // For std::runtime_error

#include "../lexer/token.h" // For Token and TokenType
#include "../ast/ast.h"     // For all Expr and Stmt classes
#include "../util/error.h"  // For MegaladonError

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::vector<std::shared_ptr<Stmt>> parse();

private:
    const std::vector<Token>& tokens;
    int current;

    bool isAtEnd() const;
    Token advance();
    Token peek() const;
    Token previous() const;
    bool check(TokenType type) const;
    bool match(const std::vector<TokenType>& types);
    Token consume(TokenType type, const std::string& message);
    void synchronize();

    // Declarations
    std::shared_ptr<Stmt> declaration();
    std::shared_ptr<VarStmt> varDeclaration();
    std::shared_ptr<FunctionStmt> function(const std::string& kind);

    // Statements
    std::shared_ptr<Stmt> statement();
    std::shared_ptr<PrintStmt> printStatement();
    std::shared_ptr<Stmt> block(); // Returns a BlockStmt
    std::shared_ptr<IfStmt> ifStatement();
    std::shared_ptr<WhileStmt> whileStatement();
    std::shared_ptr<Stmt> forStatement();
    std::shared_ptr<ReturnStmt> returnStatement();
    std::shared_ptr<ExpressionStmt> expressionStatement();

    // Expressions
    std::shared_ptr<Expr> expression();
    std::shared_ptr<Expr> assignment();
    std::shared_ptr<Expr> orLogic();
    std::shared_ptr<Expr> andLogic();
    std::shared_ptr<Expr> equality();
    std::shared_ptr<Expr> comparison();
    std::shared_ptr<Expr> term();
    std::shared_ptr<Expr> factor(); // Includes MODULO
    std::shared_ptr<Expr> unary();
    std::shared_ptr<Expr> call();
    std::shared_ptr<Expr> finishCall(std::shared_ptr<Expr> callee);
    std::shared_ptr<Expr> primary();
};