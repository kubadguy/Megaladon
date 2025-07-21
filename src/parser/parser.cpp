#include "parser.h"
#include "../util/error.h" // Assuming MegaladonError is defined here

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

std::vector<std::shared_ptr<Stmt>> Parser::parse() {
    std::vector<std::shared_ptr<Stmt>> statements;
    while (!isAtEnd()) {
        statements.push_back(declaration());
    }
    return statements;
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EOF_TOKEN;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

Token Parser::peek() const {
    return tokens[current];
}

Token Parser::previous() const {
    return tokens[current - 1];
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw MegaladonError(peek(), message);
}

void Parser::synchronize() {
    advance();

    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;

        switch (peek().type) {
            case TokenType::CLASS:
            case TokenType::FUN:
            case TokenType::VAR:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::RETURN:
                return;
            default: break;
        }
        advance();
    }
}

// --- Declarations ---

std::shared_ptr<Stmt> Parser::declaration() {
    try {
        if (match({TokenType::FUN})) return function("function");
        if (match({TokenType::VAR})) return varDeclaration();
        return statement();
    } catch (const MegaladonError& error) {
        synchronize();
        return nullptr; // Or return an error statement
    }
}

std::shared_ptr<VarStmt> Parser::varDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");

    std::shared_ptr<Expr> initializer = nullptr;
    if (match({TokenType::EQUAL})) {
        initializer = expression();
    }

    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return std::make_shared<VarStmt>(name, initializer);
}

std::shared_ptr<FunctionStmt> Parser::function(const std::string& kind) {
    Token name = consume(TokenType::IDENTIFIER, "Expect " + kind + " name.");
    consume(TokenType::LEFT_PAREN, "Expect '(' after " + kind + " name.");
    std::vector<Token> parameters;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            if (parameters.size() >= 255) { // Arbitrary limit for parameters
                MegaladonError::report(peek(), "Cannot have more than 255 parameters.");
            }
            parameters.push_back(consume(TokenType::IDENTIFIER, "Expect parameter name."));
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");

    consume(TokenType::LEFT_BRACE, "Expect '{' before " + kind + " body.");
    std::shared_ptr<BlockStmt> body = std::static_pointer_cast<BlockStmt>(block());
    return std::make_shared<FunctionStmt>(name, parameters, body);
}


// --- Statements ---

std::shared_ptr<Stmt> Parser::statement() {
    if (match({TokenType::PRINT})) return printStatement();
    if (match({TokenType::LEFT_BRACE})) return block();
    if (match({TokenType::IF})) return ifStatement();
    if (match({TokenType::WHILE})) return whileStatement();
    if (match({TokenType::FOR})) return forStatement(); // Will need to implement for loop parsing
    if (match({TokenType::RETURN})) return returnStatement();

    return expressionStatement();
}

std::shared_ptr<PrintStmt> Parser::printStatement() {
    std::shared_ptr<Expr> value = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    return std::make_shared<PrintStmt>(value);
}

std::shared_ptr<Stmt> Parser::block() {
    std::vector<std::shared_ptr<Stmt>> statements;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(declaration());
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return std::make_shared<BlockStmt>(statements);
}

std::shared_ptr<IfStmt> Parser::ifStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    std::shared_ptr<Expr> condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");

    std::shared_ptr<Stmt> thenBranch = statement();
    std::shared_ptr<Stmt> elseBranch = nullptr;
    if (match({TokenType::ELSE})) {
        elseBranch = statement();
    }
    return std::make_shared<IfStmt>(condition, thenBranch, elseBranch);
}

std::shared_ptr<WhileStmt> Parser::whileStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    std::shared_ptr<Expr> condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after while condition.");
    std::shared_ptr<Stmt> body = statement();

    return std::make_shared<WhileStmt>(condition, body);
}

std::shared_ptr<Stmt> Parser::forStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");

    std::shared_ptr<VarStmt> initializer = nullptr;
    if (match({TokenType::VAR})) {
        initializer = varDeclaration();
    } else if (match({TokenType::SEMICOLON})) {
        // No initializer
    } else {
        initializer = std::static_pointer_cast<VarStmt>(expressionStatement());
    }

    std::shared_ptr<Expr> condition = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        condition = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

    std::shared_ptr<Expr> increment = nullptr;
    if (!check(TokenType::RIGHT_PAREN)) {
        increment = expression();
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

    std::shared_ptr<Stmt> body = statement();

    if (increment != nullptr) {
        body = std::make_shared<BlockStmt>(std::vector<std::shared_ptr<Stmt>>{
            body,
            std::make_shared<ExpressionStmt>(increment)
        });
    }

    if (condition == nullptr) {
        condition = std::make_shared<LiteralExpr>(MegaladonValue(true)); // Infinite loop if no condition
    }
    body = std::make_shared<WhileStmt>(condition, body);

    if (initializer != nullptr) {
        body = std::make_shared<BlockStmt>(std::vector<std::shared_ptr<Stmt>>{
            initializer,
            body
        });
    }

    return body;
}

std::shared_ptr<ReturnStmt> Parser::returnStatement() {
    Token keyword = previous();
    std::shared_ptr<Expr> value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    return std::make_shared<ReturnStmt>(keyword, value);
}

std::shared_ptr<ExpressionStmt> Parser::expressionStatement() {
    std::shared_ptr<Expr> expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_shared<ExpressionStmt>(expr);
}


// --- Expressions ---

std::shared_ptr<Expr> Parser::expression() {
    return assignment();
}

std::shared_ptr<Expr> Parser::assignment() {
    std::shared_ptr<Expr> expr = orLogic(); // Changed from `equality()` to `orLogic()`

    if (match({TokenType::EQUAL})) {
        Token equals = previous();
        std::shared_ptr<Expr> value = assignment(); // Allow for right-associativity

        if (std::shared_ptr<VariableExpr> var_expr = std::dynamic_pointer_cast<VariableExpr>(expr)) {
            Token name = var_expr->name;
            return std::make_shared<AssignExpr>(name, value);
        }
        // Handle list element assignment: list[index] = value
        if (std::shared_ptr<GetExpr> get_expr = std::dynamic_pointer_cast<GetExpr>(expr)) {
            return std::make_shared<SetExpr>(get_expr->object, get_expr->index, value);
        }

        MegaladonError::report(equals, "Invalid assignment target.");
    }

    return expr;
}

std::shared_ptr<Expr> Parser::orLogic() {
    std::shared_ptr<Expr> expr = andLogic();

    while (match({TokenType::OR})) {
        Token op = previous();
        std::shared_ptr<Expr> right = andLogic();
        expr = std::make_shared<LogicalExpr>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::andLogic() {
    std::shared_ptr<Expr> expr = equality();

    while (match({TokenType::AND})) {
        Token op = previous();
        std::shared_ptr<Expr> right = equality();
        expr = std::make_shared<LogicalExpr>(expr, op, right);
    }
    return expr;
}


std::shared_ptr<Expr> Parser::equality() {
    std::shared_ptr<Expr> expr = comparison();

    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
        Token op = previous();
        std::shared_ptr<Expr> right = comparison();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::comparison() {
    std::shared_ptr<Expr> expr = term();

    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token op = previous();
        std::shared_ptr<Expr> right = term();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::term() {
    std::shared_ptr<Expr> expr = factor();

    while (match({TokenType::MINUS, TokenType::PLUS})) {
        Token op = previous();
        std::shared_ptr<Expr> right = factor();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::factor() {
    std::shared_ptr<Expr> expr = unary();

    while (match({TokenType::SLASH, TokenType::STAR, TokenType::MODULO})) { // Added MODULO
        Token op = previous();
        std::shared_ptr<Expr> right = unary();
        expr = std::make_shared<BinaryExpr>(expr, op, right);
    }
    return expr;
}

std::shared_ptr<Expr> Parser::unary() {
    if (match({TokenType::BANG, TokenType::MINUS})) {
        Token op = previous();
        std::shared_ptr<Expr> right = unary();
        return std::make_shared<UnaryExpr>(op, right);
    }
    return call();
}

std::shared_ptr<Expr> Parser::call() {
    std::shared_ptr<Expr> expr = primary();

    while (true) {
        if (match({TokenType::LEFT_PAREN})) {
            expr = finishCall(expr);
        } else if (match({TokenType::DOT})) {
            // For properties or methods like list.add()
            Token name = consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
            expr = std::make_shared<GetExpr>(expr, name); // GetExpr for dot access
        } else if (match({TokenType::LEFT_BRACKET})) { // For list indexing
            std::shared_ptr<Expr> index = expression();
            consume(TokenType::RIGHT_BRACKET, "Expect ']' after list index.");
            // Re-use GetExpr for list access
            expr = std::make_shared<GetExpr>(expr, index);
        }
        else {
            break;
        }
    }
    return expr;
}

std::shared_ptr<Expr> Parser::finishCall(std::shared_ptr<Expr> callee) {
    std::vector<std::shared_ptr<Expr>> arguments;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            if (arguments.size() >= 255) { // Arbitrary limit for arguments
                MegaladonError::report(peek(), "Cannot have more than 255 arguments.");
            }
            arguments.push_back(expression());
        } while (match({TokenType::COMMA}));
    }

    Token paren = consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
    return std::make_shared<CallExpr>(callee, paren, arguments);
}


std::shared_ptr<Expr> Parser::primary() {
    if (match({TokenType::FALSE})) return std::make_shared<LiteralExpr>(MegaladonValue(false));
    if (match({TokenType::TRUE})) return std::make_shared<LiteralExpr>(MegaladonValue(true));
    if (match({TokenType::NIL})) return std::make_shared<LiteralExpr>(MegaladonValue(ValueType::VOID)); // Using VOID for NIL

    if (match({TokenType::NUMBER, TokenType::STRING})) {
        return std::make_shared<LiteralExpr>(previous().literal);
    }

    if (match({TokenType::IDENTIFIER})) {
        return std::make_shared<VariableExpr>(previous());
    }

    if (match({TokenType::LEFT_PAREN})) {
        std::shared_ptr<Expr> expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return std::make_shared<GroupingExpr>(expr);
    }

    if (match({TokenType::LEFT_BRACKET})) { // For list literals like [1, 2, "hello"]
        std::vector<std::shared_ptr<Expr>> elements;
        if (!check(TokenType::RIGHT_BRACKET)) {
            do {
                elements.push_back(expression());
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after list elements.");
        return std::make_shared<ListExpr>(elements);
    }


    throw MegaladonError(peek(), "Expect expression.");
}