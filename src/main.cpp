#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "interpreter/interpreter.h"
#include "util/error.h"

// Function to run Megaladon code from a string
void run(const std::string& source) {
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.scanTokens();

    if (MegaladonError::hadError) {
        return; // Exit if lexical errors occurred
    }

    Parser parser(tokens);
    std::vector<std::shared_ptr<Stmt>> statements = parser.parse();

    if (MegaladonError::hadError) {
        return; // Exit if parsing errors occurred
    }

    Interpreter interpreter;
    interpreter.interpret(statements);

    if (MegaladonError::hadRuntimeError) {
        return; // Exit if runtime errors occurred
    }
}

// Function to run Megaladon code from a file
void runFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "MegaladonError: Could not open file '" << path << "'.\n";
        exit(74); // Exit code for I/O error
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    run(buffer.str());

    if (MegaladonError::hadError) exit(65); // Exit code for data format error
    if (MegaladonError::hadRuntimeError) exit(70); // Exit code for internal software error
}

// Function for interactive prompt
void runPrompt() {
    std::cout << "Megaladon REPL\n";
    std::cout << "Type 'exit()' to quit.\n";
    std::string line;
    for (;;) {
        std::cout << ">>> "; // Megaladon prompt
        if (!std::getline(std::cin, line)) break;
        if (line == "exit()") break;

        // For single-line input in REPL, add a semicolon if it's an expression
        // This is a common REPL hack: if it's an expression, print its value.
        bool is_expression = true; // Heuristic: assume expression unless it looks like a statement
        if (line.find("var ") == 0 || line.find("fun ") == 0 || line.find("if ") == 0 ||
            line.find("while ") == 0 || line.find("for ") == 0 || line.find("print ") == 0) {
            is_expression = false;
        }

        std::string source_to_run = line;
        if (is_expression && line.find_last_of(';') == std::string::npos) {
            source_to_run = "print (" + line + ");"; // Wrap as print statement
        } else if (line.find_last_of(';') == std::string::npos) {
            source_to_run += ";"; // Ensure statements end with semicolon
        }

        MegaladonError::hadError = false;
        MegaladonError::hadRuntimeError = false;
        run(source_to_run);

        // Reset error flags for next prompt
        MegaladonError::hadError = false;
        MegaladonError::hadRuntimeError = false;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cout << "Usage: megaladon [script]\n";
        return 64; // Incorrect usage exit code
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        runPrompt();
    }

    return 0;
}