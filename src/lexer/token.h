#pragma once

#include <string>
#include "../types/value.h" // For MegaladonValue

enum class TokenType {
    // Single-character tokens.
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    LEFT_BRACKET, RIGHT_BRACKET, // Added for lists
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR, MODULO, // Added MODULO

    // One or two character tokens.
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    // Literals.
    IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER, TOKEN_BOOLEAN, // Renamed STRING, NUMBER, BOOLEAN
    
    // Keywords.
    AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,

    EOF_TOKEN,

    NUMBER, STRING
};

struct Token {
    TokenType type;
    std::string lexeme;
    MegaladonValue literal; // Changed from 'void*' to MegaladonValue
    int line;

    Token(TokenType type, std::string lexeme, MegaladonValue literal, int line)
        : type(type), lexeme(std::move(lexeme)), literal(std::move(literal)), line(line) {}

    Token(TokenType type, std::string lexeme, int line) // Constructor for tokens without a literal value
        : type(type), lexeme(std::move(lexeme)), line(line) {}

    std::string toString() const; // Implemented in token.cpp (if you have one) or directly here if small
};