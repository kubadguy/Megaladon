// src/parser/parser.cpp
#include "parser.h"
#include "../util/error.h" // For error reporting
#include <stdexcept>
#include <utility> // For std::move

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!isAtEnd()) {
        try {
            statements.push_back(declaration());
        } catch (const ParseError& e) {
            // Error already reported by 'error' method.
            // Just synchronize and continue.
            synchronize();
        }
    }
    return statements;
}

// --- Declaration Parsing ---
std::unique_ptr<Stmt> Parser::declaration() {
    if (match({VAR_DECL})) return varDeclaration();
    if (match({FUNC_DECL})) return functionDeclaration("function");
    return statement();
}

std::unique_ptr<Stmt> Parser::varDeclaration() {
    Token name = consume(IDENTIFIER, "Expect variable name after 'var'.");

    std::unique_ptr<Expr> initializer = nullptr;
    if (match({EQUAL})) {
        initializer = expression();
    }

    // Megaladon statements are implicitly terminated by newline or end of block.
    // For now, we'll assume statements end without an explicit terminator.
    // If you add a terminator like ';', you'd consume it here.
    // consume(SEMICOLON, "Expect ';' after variable declaration.");

    return std::make_unique<VarDeclStmt>(std::move(name), std::move(initializer));
}

std::unique_ptr<Stmt> Parser::functionDeclaration(const std::string& kind) {
    Token name = consume(IDENTIFIER, "Expect " + kind + " name.");
    consume(LEFT_PAREN, "Expect '(' after " + kind + " name.");

    std::vector<Token> parameters;
    if (!check(RIGHT_PAREN)) {
        do {
            if (parameters.size() >= 255) {
                error(peek(), "Cannot have more than 255 parameters.");
            }
            parameters.push_back(consume(IDENTIFIER, "Expect parameter name."));
        } while (match({COMMA}));
    }
    consume(RIGHT_PAREN, "Expect ')' after parameters.");

    consume(COLON, "Expect ':' before " + kind + " body."); // `func myFunc(a, b):`
    std::unique_ptr<BlockStmt> body = block();

    // Megaladon functions might have an explicit end marker
    if (kind == "function") {
        consume(END_FUNC, "Expect 'endfunc' after function body.");
    }

    return std::make_unique<FunctionStmt>(std::move(name), std::move(parameters), std::move(body));
}


// --- Statement Parsing ---
std::unique_ptr<Stmt> Parser::statement() {
    if (match({PRINT, SAY})) return printStatement();
    if (match({IF})) return ifStatement();
    if (match({WHILE})) return whileStatement();
    if (match({RETURN})) return returnStatement();
    if (match({COLON})) return block(); // Implicit block for if/while bodies, or explicit blocks
    return expressionStatement();
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
    std::unique_ptr<Expr> expr = expression();
    // consume(SEMICOLON, "Expect ';' after expression."); // If using explicit terminators
    return std::make_unique<ExpressionStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::printStatement() {
    std::unique_ptr<Expr> value = expression();
    // consume(SEMICOLON, "Expect ';' after value.");
    return std::make_unique<PrintStmt>(std::move(value));
}

std::unique_ptr<Stmt> Parser::ifStatement() {
    consume(LEFT_PAREN, "Expect '(' after 'if'.");
    std::unique_ptr<Expr> condition = expression();
    consume(RIGHT_PAREN, "Expect ')' after if condition.");

    consume(COLON, "Expect ':' before 'if' body.");
    std::unique_ptr<Stmt> thenBranch = statement();

    std::unique_ptr<Stmt> elseBranch = nullptr;
    if (match({ELSE})) {
        consume(COLON, "Expect ':' before 'else' body.");
        elseBranch = statement();
    }

    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::whileStatement() {
    consume(LEFT_PAREN, "Expect '(' after 'while'.");
    std::unique_ptr<Expr> condition = expression();
    consume(RIGHT_PAREN, "Expect ')' after while condition.");

    consume(COLON, "Expect ':' before 'while' body.");
    std::unique_ptr<Stmt> body = statement();

    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::returnStatement() {
    Token keyword = previous(); // The 'return' token
    std::unique_ptr<Expr> value = nullptr;
    if (!check(END_OF_FILE)) { // Check if there's an expression after return
        // If we implement implicit statement termination without ;, this check needs care
        // A simple check is to see if the next token is not a keyword that starts a new statement
        // For simplicity, for now, we'll just consume if it's not EOF or end of block (like 'endfunc')
        // In a real parser, this would be `if (!check(SEMICOLON) && !check(END_FUNC))`
        if (!check(END_FUNC)) {
             value = expression();
        }
    }
    // consume(SEMICOLON, "Expect ';' after return value."); // If using explicit terminators
    return std::make_unique<ReturnStmt>(std::move(keyword), std::move(value));
}


std::unique_ptr<BlockStmt> Parser::block() {
    std::vector<std::unique_ptr<Stmt>> statements;
    // In Megaladon, a block starts with COLON and contains indented statements.
    // For simplicity of parsing, we will treat any sequence of declarations/statements
    // following the COLON until EOF or a keyword that signals end of block (like `endfunc`, `else`)
    // as part of the current block. A more robust parser would handle indentation.

    // For now, we'll treat a block as a sequence of statements until a closing token
    // or next `var`, `func`, `if`, `while` which signifies new top-level statements.
    // This is a simplification. A real block would be `{ ... }` or indentation based.
    // For now, we assume simple sequential statements after a colon until a new
    // declaration or `endfunc` is encountered.

    while (!isAtEnd() && !check(END_FUNC) &&
           !check(ELSE) && !check(VAR_DECL) && !check(FUNC_DECL) &&
           !check(IF) && !check(WHILE) && !check(RETURN)) {
        statements.push_back(declaration());
    }

    // No explicit closing brace/keyword for a general block yet in this simplified model.
    // This needs careful consideration for implicit blocks (like after `if:` or `while:`).
    // For now, it will parse statements until a new declaration or EOF or `endfunc`.
    return std::make_unique<BlockStmt>(std::move(statements));
}


// --- Expression Parsing ---
std::unique_ptr<Expr> Parser::expression() {
    return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
    std::unique_ptr<Expr> expr = logicOr();

    if (match({EQUAL})) {
        Token equals = previous();
        std::unique_ptr<Expr> value = assignment(); // Right-associativity

        if (dynamic_cast<VariableExpr*>(expr.get())) {
            Token name = static_cast<VariableExpr*>(expr.get())->name;
            return std::make_unique<AssignExpr>(std::move(name), std::move(value));
        } else if (dynamic_cast<GetExpr*>(expr.get())) {
            // For object.property = value
            GetExpr* get = static_cast<GetExpr*>(expr.get());
            return std::make_unique<SetExpr>(std::move(get->object), get->name, std::move(value));
        } else if (dynamic_cast<IndexExpr*>(expr.get())) {
            // For list[index] = value
            IndexExpr* index_expr = static_cast<IndexExpr*>(expr.get());
            return std::make_unique<IndexAssignExpr>(std::move(index_expr->object), std::move(index_expr->index), std::move(value));
        }

        error(equals, "Invalid assignment target.");
    }
    return expr;
}


std::unique_ptr<Expr> Parser::logicOr() {
    std::unique_ptr<Expr> expr = logicAnd();
    while (match({OR})) {
        Token op = previous();
        std::unique_ptr<Expr> right = logicAnd();
        expr = std::make_unique<LogicalExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::logicAnd() {
    std::unique_ptr<Expr> expr = equality();
    while (match({AND})) {
        Token op = previous();
        std::unique_ptr<Expr> right = equality();
        expr = std::make_unique<LogicalExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::equality() {
    std::unique_ptr<Expr> expr = comparison();

    while (match({BANG_EQUAL, EQUAL_EQUAL})) {
        Token op = previous();
        std::unique_ptr<Expr> right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    std::unique_ptr<Expr> expr = term();

    while (match({GREATER, GREATER_EQUAL, LESS, LESS_EQUAL})) {
        Token op = previous();
        std::unique_ptr<Expr> right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    std::unique_ptr<Expr> expr = factor();

    while (match({MINUS, PLUS})) {
        Token op = previous();
        std::unique_ptr<Expr> right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    std::unique_ptr<Expr> expr = unary();

    while (match({SLASH, ASTERISK, PERCENT})) {
        Token op = previous();
        std::unique_ptr<Expr> right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    if (match({BANG, MINUS})) {
        Token op = previous();
        std::unique_ptr<Expr> right = unary(); // Unary operator is right-associative
        return std::make_unique<UnaryExpr>(std::move(op), std::move(right));
    }
    return call();
}

std::unique_ptr<Expr> Parser::call() {
    std::unique_ptr<Expr> expr = primary();

    while (true) {
        if (match({LEFT_PAREN})) {
            expr = finishCall(std::move(expr));
        } else if (match({DOT})) { // For method calls or property access: object.method() or object.property
            Token name = consume(IDENTIFIER, "Expect property or method name after '.'.");
            expr = std::make_unique<GetExpr>(std::move(expr), std::move(name));
        } else if (match({LEFT_BRACKET})) { // For list/string indexing: list[index]
            std::unique_ptr<Expr> index = expression();
            consume(RIGHT_BRACKET, "Expect ']' after index expression.");
            expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index));
        } else {
            break;
        }
    }
    return expr;
}

std::unique_ptr<Expr> Parser::finishCall(std::unique_ptr<Expr> callee) {
    std::vector<std::unique_ptr<Expr>> arguments;
    if (!check(RIGHT_PAREN)) {
        do {
            if (arguments.size() >= 255) {
                error(peek(), "Cannot have more than 255 arguments.");
            }
            arguments.push_back(expression());
        } while (match({COMMA}));
    }
    Token paren = consume(RIGHT_PAREN, "Expect ')' after arguments.");
    return std::make_unique<CallExpr>(std::move(callee), std::move(paren), std::move(arguments));
}


std::unique_ptr<Expr> Parser::primary() {
    if (match({FALSE})) return std::make_unique<LiteralExpr>(MegaladonValue(false));
    if (match({TRUE})) return std::make_unique<LiteralExpr>(MegaladonValue(true));
    if (match({NULL_TYPE})) return std::make_unique<LiteralExpr>(MegaladonValue(MegaladonValue::VOID));

    if (match({NUMBER})) return std::make_unique<LiteralExpr>(MegaladonValue(previous().number_val));
    if (match({STRING})) return std::make_unique<LiteralExpr>(MegaladonValue(previous().string_val));

    if (match({LEFT_PAREN})) {
        std::unique_ptr<Expr> expr = expression();
        consume(RIGHT_PAREN, "Expect ')' after expression.");
        return std::make_unique<GroupingExpr>(std::move(expr));
    }

    if (match({IDENTIFIER})) {
        return std::make_unique<VariableExpr>(previous());
    }

    if (match({LEFT_BRACKET})) { // List literal: [1, "hello", true]
        std::vector<std::unique_ptr<Expr>> elements;
        if (!check(RIGHT_BRACKET)) {
            do {
                elements.push_back(expression());
            } while (match({COMMA}));
        }
        consume(RIGHT_BRACKET, "Expect ']' after list elements.");
        return std::make_unique<ListExpr>(std::move(elements));
    }

    error(peek(), "Expect expression.");
    return nullptr; // Should throw ParseError
}


// --- Helper functions ---
bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::check(TokenType type) {
    if (isAtEnd()) return false;
    return peek().type == type;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::isAtEnd() {
    return peek().type == END_OF_FILE;
}

Token Parser::peek() {
    return tokens[current];
}

Token Parser::previous() {
    return tokens[current - 1];
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    error(peek(), message);
    throw ParseError(message); // Throw to unwind stack and synchronize
}

void Parser::error(const Token& token, const std::string& message) {
    if (token.type == END_OF_FILE) {
        ReportError(token.line, token.column, " at end", message);
    } else {
        ReportError(token.line, token.column, " at '" + token.lexeme + "'", message);
    }
}

void Parser::synchronize() {
    advance(); // Consume the erroneous token

    while (!isAtEnd()) {
        // If previous was a semicolon (or implied end of statement),
        // we might be at the start of a new statement.
        // For Megaladon, if we see a new keyword that starts a declaration/statement,
        // we can assume we've synchronized.
        switch (peek().type) {
            case VAR_DECL:
            case FUNC_DECL:
            case IF:
            case WHILE:
            case PRINT:
            case SAY:
            case RETURN:
                return;
            default:
                break;
        }
        advance(); // Keep skipping tokens
    }
}