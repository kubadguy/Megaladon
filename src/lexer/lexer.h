// src/lexer/lexer.h
#pragma once

#include <string>
#include <vector>
#include <map>
#include "../types/value.h" // For MegaladonValue (literals)

// Enum for all token types
enum TokenType {
    // Single-character tokens.
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACKET, RIGHT_BRACKET, COMMA, DOT, MINUS, PLUS,
    SLASH, ASTERISK, PERCENT, COLON, TILDE,

    // One or two character tokens.
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    // Literals.
    IDENTIFIER, STRING, NUMBER, BOOLEAN,

    // Keywords.
    AND, ELSE, FALSE, FUNC_DECL, HELPWORD, IF, INPUT, OR,
    PRINT, RETURN, SAY, TRUE, VAR_DECL, WHILE, END_FUNC, NULL_TYPE,

    END_OF_FILE
};

// Represents a token produced by the lexer
struct Token {
    TokenType type;
    std::string lexeme; // The raw text of the token
    std::string string_val; // For string literals
    double number_val;      // For number literals
    bool boolean_val;       // For boolean literals
    int line;
    int column;

    // Constructor for general tokens
    Token(TokenType type, std::string lexeme, int line, int column)
        : type(type), lexeme(std::move(lexeme)), line(line), column(column) {}

    // Constructor for string literals
    Token(TokenType type, std::string lexeme, std::string string_val, int line, int column)
        : type(type), lexeme(std::move(lexeme)), string_val(std::move(string_val)), line(line), column(column) {}

    // Constructor for number literals
    Token(TokenType type, std::string lexeme, double number_val, int line, int column)
        : type(type), lexeme(std::move(lexeme)), number_val(number_val), line(line), column(column) {}

    // Constructor for boolean literals
    Token(TokenType type, std::string lexeme, bool boolean_val, int line, int column)
        : type(type), lexeme(std::move(lexeme)), boolean_val(boolean_val), line(line), column(column) {}

    std::string toString() const;
};

// The Lexer class
class Lexer {
public:
    Lexer(std::string source);
    std::vector<Token> scanTokens();

private:
    std::string source;
    std::vector<Token> tokens;
    int start = 0;   // Start of the current lexeme
    int current = 0; // Current character being considered
    int line = 1;    // Current line number
    int column = 1;  // Current column number

    static const std::map<std::string, TokenType> keywords;

    bool isAtEnd();
    char advance();
    char peek();
    char peekNext();
    bool match(char expected);
    void addToken(TokenType type);
    void addToken(TokenType type, const std::string& string_val);
    void addToken(TokenType type, double number_val);
    void addToken(TokenType type, bool boolean_val);
    void string();
    void number();
    void identifier();
    void error(int line, int col, const std::string& message);
    void scanToken();
};