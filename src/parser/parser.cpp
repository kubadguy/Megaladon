#include "parser.h"
#include "../lexer/token.h"
#include "../lexer/token_type.h"
#include "../ast/ast.h"
#include "../util/error.h" // For MegaladonError

// Constructor
Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), current(0) {}

// Parse method - entry point
std::vector<std::shared_ptr<Stmt>> Parser::parse() {
    std::vector<std::shared_ptr<Stmt>> statements;
    while (!isAtEnd()) {
        statements.push_back(declaration());
    }
    return statements;
}

// Helper to advance and return current token
Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

// Helper to check if current token matches any of the types
bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

// Helper to check if current token matches any of the types and consume it
bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

// Helper to consume a token or report error
Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw ParseError(peek(), message); // Use ParseError for syntax errors
}

// Helper to check if end of file is reached
bool Parser::isAtEnd() const {
    return peek().type == TokenType::EOF_TOKEN;
}

// Helper to get current token
Token Parser::peek() const {
    return tokens[current];
}

// Helper to get previous token
Token Parser::previous() const {
    return tokens[current - 1];
}

// Error recovery
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
            default:
                break; // Continue to next token
        }
        advance();
    }
}

// --- Declaration Parsing ---
std::shared_ptr<Stmt> Parser::declaration() {
    try {
        if (match({TokenType::VAR})) {
            return varDeclaration(); // Correctly calls specific var declaration parser
        }
        // Add other declaration types here (e.g., functions, classes)
        if (match({TokenType::FUN})) {
             // Example: return functionDeclaration();
             // For now, if FUN is not fully implemented, maybe just skip or error
             throw ParseError(peek(), "Function declarations not fully supported yet.");
        }
        return statement(); // If not a declaration, assume it's a regular statement
    } catch (const ParseError& e) {
        MegaladonError::report(e.token, e.what()); // Report syntax error
        synchronize(); // Attempt error recovery
        return nullptr; // Return nullptr to indicate parsing failure for this statement
    }
}

// --- Specific Declaration Parsers ---
std::shared_ptr<Stmt> Parser::varDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name."); // Get variable name

    std::shared_ptr<Expr> initializer = nullptr;
    if (match({TokenType::EQUAL})) { // Check if there's an initializer
        initializer = expression(); // Parse the initializer expression
    }

    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration."); // Require semicolon
    return std::make_shared<VarStmt>(name, initializer); // Create and return VarStmt
}

// --- Statement Parsing ---
std::shared_ptr<Stmt> Parser::statement() {
    if (match({TokenType::PRINT})) return printStatement();
    if (match({TokenType::LEFT_BRACE})) return std::make_shared<BlockStmt>(block());
    if (match({TokenType::IF})) return ifStatement();
    if (match({TokenType::WHILE})) return whileStatement();
    if (match({TokenType::FOR})) return forStatement(); // For statement
    if (match({TokenType::RETURN})) return returnStatement();

    return expressionStatement();
}

std::shared_ptr<Stmt> Parser::printStatement() {
    std::shared_ptr<Expr> value = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    return std::make_shared<PrintStmt>(value);
}

std::vector<std::shared_ptr<Stmt>> Parser::block() {
    std::vector<std::shared_ptr<Stmt>> statements;

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(declaration()); // A block can contain declarations
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return statements;
}

std::shared_ptr<Stmt> Parser::ifStatement() {
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

std::shared_ptr<Stmt> Parser::whileStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    std::shared_ptr<Expr> condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after while condition.");

    std::shared_ptr<Stmt> body = statement();
    return std::make_shared<WhileStmt>(condition, body);
}

std::shared_ptr<Stmt> Parser::forStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");

    std::shared_ptr<Stmt> initializer;
    if (match({TokenType::SEMICOLON})) {
        initializer = nullptr;
    } else if (match({TokenType::VAR})) {
        initializer = varDeclaration(); // Initialize with var declaration
    } else {
        initializer = expressionStatement(); // Initialize with expression statement
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

    // Desugar 'for' loop into a 'while' loop with block
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

std::shared_ptr<Stmt> Parser::returnStatement() {
    Token keyword = previous();
    std::shared_ptr<Expr> value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    return std::make_shared<ReturnStmt>(keyword, value);
}

std::shared_ptr<Stmt> Parser::expressionStatement() {
    std::shared_ptr<Expr> expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_shared<ExpressionStmt>(expr);
}

// --- Expression Parsing (recursive descent) ---

std::shared_ptr<Expr> Parser::expression() {
    return assignment();
}

std::shared_ptr<Expr> Parser::assignment() {
    std::shared_ptr<Expr> expr = orLogic(); // Changed from `logicOr` to `orLogic` for consistency

    if (match({TokenType::EQUAL})) {
        Token equals = previous();
        std::shared_ptr<Expr> value = assignment(); // Right-associative assignment

        // If the left-hand side is a VariableExpr, create an AssignExpr
        if (std::shared_ptr<VariableExpr> var_expr = std::dynamic_pointer_cast<VariableExpr>(expr)) {
            return std::make_shared<AssignExpr>(var_expr->name, value);
        }
        // If the left-hand side is a GetExpr (for property/indexed assignment)
        if (std::shared_ptr<GetExpr> get_expr = std::dynamic_pointer_cast<GetExpr>(expr)) {
            // Check if it's property access (using name) or indexed access (using index)
            if (get_expr->index != nullptr) { // Indexed access (e.g., list[0] = value)
                 return std::make_shared<SetExpr>(get_expr->object, get_expr->index, value);
            } else { // Property access (e.g., obj.prop = value)
                // This assumes GetExpr can represent property access via 'name'
                // If GetExpr is solely for indexed access, you'll need another Expr type for property access
                // For now, assuming GetExpr is only used for indexed access.
                throw ParseError(equals, "Invalid assignment target. Property assignment is not yet fully implemented via GetExpr's name token.");
            }
        }

        // If assignment target is not a variable or valid property/indexed access
        throw ParseError(equals, "Invalid assignment target.");
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

// Call expression parsing for function calls and property access
std::shared_ptr<Expr> Parser::call() {
    std::shared_ptr<Expr> expr = primary();

    while (true) {
        if (match({TokenType::LEFT_PAREN})) {
            expr = finishCall(expr);
        } else if (match({TokenType::DOT})) {
            Token name = consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
            // This is where GetExpr (for property access) or other structures would be used.
            // For now, assuming GetExpr is only for indexed access as per constructor.
            // If you want property access, you might need a different AST node or an overloaded GetExpr constructor.
            // The current GetExpr(object, index) implies indexed access.
            // If .name is for properties, this part needs a specific GetExpr for properties.
            throw ParseError(name, "Property access via '.' is not fully implemented with current GetExpr structure. Only indexed access (list[idx]) is.");
        } else if (match({TokenType::LEFT_BRACKET})) { // For list indexing
            std::shared_ptr<Expr> index = expression();
            consume(TokenType::RIGHT_BRACKET, "Expect ']' after index.");
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
            if (arguments.size() >= 255) {
                error(peek(), "Cannot have more than 255 arguments.");
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
    if (match({TokenType::NIL})) return std::make_shared<LiteralExpr>(MegaladonValue(ValueType::NIL)); // Use ValueType::NIL for Nil
    if (match({TokenType::NUMBER})) return std::make_shared<LiteralExpr>(MegaladonValue(std::stod(previous().lexeme)));
    if (match({TokenType::STRING})) return std::make_shared<LiteralExpr>(MegaladonValue(previous().lexeme));

    if (match({TokenType::LEFT_BRACKET})) { // For list literals e.g., [1, 2, "hello"]
        std::vector<std::shared_ptr<Expr>> elements;
        if (!check(TokenType::RIGHT_BRACKET)) {
            do {
                elements.push_back(expression());
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after list literal.");
        return std::make_shared<ListExpr>(elements);
    }

    if (match({TokenType::IDENTIFIER})) {
        return std::make_shared<VariableExpr>(previous());
    }

    if (match({TokenType::LEFT_PAREN})) {
        std::shared_ptr<Expr> expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return std::make_shared<GroupingExpr>(expr);
    }

    throw ParseError(peek(), "Expect expression.");
}

// Private ParseError class implementation
Parser::ParseError::ParseError(Token token, const std::string& message)
    : std::runtime_error(message), token(token) {}