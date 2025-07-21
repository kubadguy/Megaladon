// src/lexer/lexer.cpp
#include "lexer.h"
#include "../util/error.h" // For error reporting
#include <iostream>
#include <cctype> // For isdigit, isalpha, isalnum, isspace
#include <utility> // For std::move

// --- Token toString for debugging ---
std::string Token::toString() const {
    std::string s;
    switch (type) {
        // Single-character tokens
        case LEFT_PAREN:    s = "LEFT_PAREN"; break;
        case RIGHT_PAREN:   s = "RIGHT_PAREN"; break;
        case LEFT_BRACKET:  s = "LEFT_BRACKET"; break;
        case RIGHT_BRACKET: s = "RIGHT_BRACKET"; break;
        case COMMA:         s = "COMMA"; break;
        case DOT:           s = "DOT"; break;
        case MINUS:         s = "MINUS"; break;
        case PLUS:          s = "PLUS"; break;
        case SLASH:         s = "SLASH"; break;
        case ASTERISK:      s = "ASTERISK"; break;
        case PERCENT:       s = "PERCENT"; break;
        case COLON:         s = "COLON"; break;
        case TILDE:         s = "TILDE"; break;

        // One or two character tokens
        case BANG:          s = "BANG"; break;
        case BANG_EQUAL:    s = "BANG_EQUAL"; break;
        case EQUAL:         s = "EQUAL"; break;
        case EQUAL_EQUAL:   s = "EQUAL_EQUAL"; break;
        case GREATER:       s = "GREATER"; break;
        case GREATER_EQUAL: s = "GREATER_EQUAL"; break;
        case LESS:          s = "LESS"; break;
        case LESS_EQUAL:    s = "LESS_EQUAL"; break;

        // Literals
        case IDENTIFIER:    s = "IDENTIFIER"; break;
        case STRING:        s = "STRING"; break;
        case NUMBER:        s = "NUMBER"; break;
        case BOOLEAN:       s = "BOOLEAN"; break;

        // Keywords
        case AND:           s = "AND"; break;
        case ELSE:          s = "ELSE"; break;
        case FALSE:         s = "FALSE"; break;
        case FUNC_DECL:     s = "FUNC_DECL"; break;
        case HELPWORD:      s = "HELPWORD"; break;
        case IF:            s = "IF"; break;
        case INPUT:         s = "INPUT"; break;
        case OR:            s = "OR"; break;
        case PRINT:         s = "PRINT"; break;
        case RETURN:        s = "RETURN"; break;
        case SAY:           s = "SAY"; break;
        case TRUE:          s = "TRUE"; break;
        case VAR_DECL:      s = "VAR_DECL"; break;
        case WHILE:         s = "WHILE"; break;
        case END_FUNC:      s = "END_FUNC"; break;
        case NULL_TYPE:     s = "NULL_TYPE"; break;

        // Special
        case END_OF_FILE:   s = "END_OF_FILE"; break;
    }

    s += " " + lexeme;
    if (type == STRING) s += " (val: \"" + string_val + "\")";
    if (type == NUMBER) s += " (val: " + std::to_string(number_val) + ")";
    if (type == BOOLEAN) s += " (val: " + (boolean_val ? "true" : "false") + ")";
    s += " at " + std::to_string(line) + ":" + std::to_string(column);
    return s;
}


// --- Lexer Class Implementation ---

// Initialize keyword map
const std::map<std::string, TokenType> Lexer::keywords = {
    {"and", AND},
    {"else", ELSE},
    {"false", FALSE},
    {"func", FUNC_DECL},
    {"helpword", HELPWORD}, // A keyword for help/documentation?
    {"if", IF},
    {"input", INPUT},
    {"or", OR},
    {"print", PRINT},
    {"return", RETURN},
    {"say", SAY}, // Like print, but maybe more for conversation with Timestalker?
    {"true", TRUE},
    {"var", VAR_DECL},
    {"while", WHILE},
    {"endfunc", END_FUNC},
    {"void", NULL_TYPE} // Representing null/void
};

Lexer::Lexer(std::string source) : source(std::move(source)) {}

std::vector<Token> Lexer::scanTokens() {
    while (!isAtEnd()) {
        start = current;
        scanToken();
    }
    tokens.emplace_back(END_OF_FILE, "", line, column); // Add EOF token
    return tokens;
}

bool Lexer::isAtEnd() {
    return current >= source.length();
}

char Lexer::advance() {
    column++;
    return source[current++];
}

char Lexer::peek() {
    if (isAtEnd()) return '\0';
    return source[current];
}

char Lexer::peekNext() {
    if (current + 1 >= source.length()) return '\0';
    return source[current + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;
    current++;
    column++;
    return true;
}

void Lexer::addToken(TokenType type) {
    std::string text = source.substr(start, current - start);
    tokens.emplace_back(type, text, line, column - text.length()); // Adjust column back to start of lexeme
}

void Lexer::addToken(TokenType type, const std::string& string_val) {
    std::string text = source.substr(start, current - start);
    tokens.emplace_back(type, text, string_val, line, column - text.length());
}

void Lexer::addToken(TokenType type, double number_val) {
    std::string text = source.substr(start, current - start);
    tokens.emplace_back(type, text, number_val, line, column - text.length());
}

void Lexer::addToken(TokenType type, bool boolean_val) {
    std::string text = source.substr(start, current - start);
    tokens.emplace_back(type, text, boolean_val, line, column - text.length());
}


void Lexer::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') {
            line++;
            column = 0; // Reset column for new line
        }
        advance();
    }

    if (isAtEnd()) {
        error(line, column, "Unterminated string.");
        return;
    }

    // The closing ".
    advance();

    // Trim the surrounding quotes.
    std::string value = source.substr(start + 1, current - 2 - start);
    addToken(STRING, value);
}

void Lexer::number() {
    while (std::isdigit(peek())) advance();

    // Look for a fractional part.
    if (peek() == '.' && std::isdigit(peekNext())) {
        // Consume the "."
        advance();
        while (std::isdigit(peek())) advance();
    }

    try {
        addToken(NUMBER, std::stod(source.substr(start, current - start)));
    } catch (const std::out_of_range& e) {
        error(line, column, "Number literal too large or small.");
    } catch (const std::invalid_argument& e) {
        error(line, column, "Invalid number format.");
    }
}

void Lexer::identifier() {
    while (std::isalnum(peek()) || peek() == '_') advance(); // Allow underscores

    std::string text = source.substr(start, current - start);
    auto it = keywords.find(text);
    if (it != keywords.end()) {
        // Found a keyword
        if (it->second == TRUE || it->second == FALSE) {
            addToken(BOOLEAN, (it->second == TRUE));
        } else {
            addToken(it->second);
        }
    } else {
        addToken(IDENTIFIER); // It's a regular identifier
    }
}

void Lexer::error(int line, int col, const std::string& message) {
    ReportError(line, col, "", message); // Use global error reporting
}

void Lexer::scanToken() {
    char c = advance();
    switch (c) {
        case '(': addToken(LEFT_PAREN); break;
        case ')': addToken(RIGHT_PAREN); break;
        case '[': addToken(LEFT_BRACKET); break;
        case ']': addToken(RIGHT_BRACKET); break;
        case ',': addToken(COMMA); break;
        case '.': addToken(DOT); break;
        case '-': addToken(MINUS); break;
        case '+': addToken(PLUS); break;
        case ';': /* Ignore semicolons for now, or use as statement terminator */ break;
        case '*': addToken(ASTERISK); break;
        case '%': addToken(PERCENT); break;
        case ':': addToken(COLON); break;
        case '~': addToken(TILDE); break; // For bitwise NOT or other special ops

        case '!':
            addToken(match('=') ? BANG_EQUAL : BANG);
            break;
        case '=':
            addToken(match('=') ? EQUAL_EQUAL : EQUAL);
            break;
        case '<':
            addToken(match('=') ? LESS_EQUAL : LESS);
            break;
        case '>':
            addToken(match('=') ? GREATER_EQUAL : GREATER);
            break;

        case '/':
            if (match('/')) {
                // A comment goes until the end of the line.
                while (peek() != '\n' && !isAtEnd()) advance();
            } else {
                addToken(SLASH);
            }
            break;

        case ' ':
        case '\r':
        case '\t':
            // Ignore whitespace.
            break;

        case '\n':
            line++;
            column = 1; // Reset column on new line
            break;

        case '"': string(); break;

        default:
            if (std::isdigit(c)) {
                number();
            } else if (std::isalpha(c) || c == '_') {
                identifier();
            } else {
                error(line, column, "Unexpected character '" + std::string(1, c) + "'.");
            }
            break;
    }
}