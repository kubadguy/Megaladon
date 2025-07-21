#pragma once

#include <string>
#include <vector>
#include "token.h" // For TokenType and Token struct

class Lexer {
public:
    Lexer(const std::string& source);
    std::vector<Token> scanTokens();

private:
    const std::string& source;
    std::vector<Token> tokens;
    int start;
    int current;
    int line;

    bool isAtEnd() const;
    void scanToken();
    char advance();
    void addToken(TokenType type);
    void addToken(TokenType type, const MegaladonValue& literal); // For literals
    bool match(char expected);
    char peek() const;
    char peekNext() const;

    void string();
    void number();
    void identifier();

    bool isDigit(char c);
    bool isAlpha(char c);
    bool isAlphaNumeric(char c);
};