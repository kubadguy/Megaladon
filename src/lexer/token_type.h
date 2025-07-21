#ifndef MEGALADON_TOKEN_TYPE_H
#define MEGALADON_TOKEN_TYPE_H

// Enum to define all possible token types in the Megaladon language.
enum class TokenType {
    // Single-character tokens.
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    LEFT_BRACKET, RIGHT_BRACKET, // Added for list literals and indexing
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR, MODULO, // Added MODULO

    // One or two character tokens.
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    // Literals.
    IDENTIFIER, STRING, NUMBER, BOOLEAN, // BOOLEAN for true/false

    // Keywords.
    AND, CLASS, ELSE, FUN, FOR, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS, VAR, WHILE, TRUE, FALSE, // Added TRUE/FALSE as separate keywords

    EOF_TOKEN // End of File
};

#endif // MEGALADON_TOKEN_TYPE_H