// src/main.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "interpreter/interpreter.h"
#include "util/error.h" // Global error flags

// Function to run Megaladon code from a string
void run(const std::string& source) {
    ResetErrors(); // Clear previous errors

    Lexer lexer(source);
    std::vector<Token> tokens = lexer.scanTokens();

    if (hadError) return; // Stop if lexical errors occurred

    Parser parser(tokens);
    std::vector<std::unique_ptr<Stmt>> statements = parser.parse();

    if (hadError) return; // Stop if parsing errors occurred

    Interpreter interpreter;
    interpreter.interpret(statements);

    if (hadRuntimeError) return; // Stop if runtime errors occurred
}

// Function to run Megaladon code from a file
void runFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << path << "'." << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    run(buffer.str());
}

// Function for interactive REPL (Read-Eval-Print Loop)
void runPrompt() {
    std::cout << "Megaladon REPL (Type 'exit()' to quit or 'helpword' for help.)" << std::endl;
    std::string line;
    Interpreter interpreter; // Single interpreter instance for REPL

    while (true) {
        std::cout << ">>> ";
        if (!std::getline(std::cin, line)) { // Ctrl+D or EOF
            break;
        }
        if (line == "exit()") {
            break;
        }

        ResetErrors(); // Reset errors for each new line

        Lexer lexer(line);
        std::vector<Token> tokens = lexer.scanTokens();

        if (hadError) continue;

        Parser parser(tokens);
        // In REPL, allow single expressions or statements.
        // If the line is an expression, evaluate and print.
        // If it's a statement, execute it.
        // This is a simplified approach. A full REPL parser is more complex.
        try {
            std::vector<std::unique_ptr<Stmt>> statements = parser.parse();
            if (hadError) continue;

            for (const auto& stmt : statements) {
                // If it's just an expression statement, print its value
                if (ExpressionStmt* expr_stmt = dynamic_cast<ExpressionStmt*>(stmt.get())) {
                    MegaladonValue result = interpreter.evaluate(*expr_stmt->expression);
                    // Only print if it's not VOID (e.g., assignment doesn't print)
                    if (!result.isVoid()) {
                        std::cout << result.toString() << std::endl;
                    }
                } else {
                    interpreter.execute(*stmt);
                }
            }
        } catch (const Parser::ParseError& e) {
            // Error already reported by parser, just continue
        } catch (const std::runtime_error& e) {
            ReportRuntimeError(e.what());
        }

        // Reset runtime error flag for next line
        hadRuntimeError = false;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cout << "Usage: " << argv[0] << " [script_file]" << std::endl;
        return 64; // Incorrect usage
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        runPrompt();
    }

    return 0;
}