# Megaladon Language Project

## Overview
Megaladon is an ambitious new programming language aiming to be as powerful, light, and fast as Lua, more versatile than Python, capable of running 500 AI agents in parallel, and easy for beginners to learn. It's designed to be a complete environment, including:
* **Megaladon (Programming Language)**: The core language for general-purpose programming.
* **Megascript (Scripting Language)**: A lighter-weight scripting variant.
* **Megladis (Query and Storage Language)**: For data interaction.
* **Megaladon ML (Markup Language)**: For structured data and content.

This repository currently focuses on the initial development of the **Megaladon programming language**'s front-end, specifically the Lexer (tokenizer) and the Parser (syntax analyzer).

## Project Structure

```bash
megaladon/
├── src/
│   ├── include/
│   │   ├── TokenType.hpp         // Defines all token types for Megaladon.
│   │   ├── Token.hpp             // Defines the structure for a recognized token.
│   │   ├── Lexer.hpp             // Declares the MegaladonLexer class.
│   │   ├── AST.hpp               // Defines Abstract Syntax Tree (AST) node structures.
│   │   ├── Parser.hpp            // Declares the MegaladonParser class.
│   │   ├── ErrorHandler.hpp      // Provides a centralized system for error reporting.
│   │   └── SymbolTable.hpp       // (Placeholder) For managing variable/function scopes and definitions.
│   │
│   ├── Lexer.cpp                 // Implements the MegaladonLexer's tokenization logic.
│   ├── AST.cpp                   // (Placeholder) For complex AST node method implementations.
│   ├── Parser.cpp                // Implements the MegaladonParser's syntax analysis logic.
│   ├── ErrorHandler.cpp          // (Placeholder) For ErrorHandler implementations.
│   └── main.cpp                  // The entry point of the compiler front-end.
│
├── example.meg                   // Sample Megaladon source code for testing.
└── README.md                     // This documentation file.
```

## Getting Started

### Prerequisites
* A C++17 compatible compiler (e.g., g++ for MinGW64 on Windows, or GCC/Clang on Linux/macOS).

### Compilation

1.  **Navigate to the project root:**
    ```bash
    cd /c/megaladon # Or wherever your 'megaladon' folder is located
    ```
2.  **Compile the source files:**
    ```bash
    g++ -std=c++17 src/Lexer.cpp src/Parser.cpp src/main.cpp src/AST.cpp src/ErrorHandler.cpp -o megaladon -I src/include
    ```
    * `-std=c++17`: Ensures C++17 standard features are used.
    * `-o megaladon`: Specifies the output executable name.
    * `-I src/include`: Tells the compiler to look for header files in the `src/include` directory.

### Running the Lexer/Parser

After successful compilation, you can run the `megaladon` executable with a Megaladon source file.

1.  **Run with `example.meg`:**
    ```bash
    ./megaladon example.meg
    ```

    The output will first show the token stream generated by the lexer, and then initiate the parser.
    *Note*: The parser is currently a **placeholder** and will report that it's starting and finishing, but the full AST construction logic is the next development phase. It will likely report "Unexpected token" errors as it encounters grammar elements it hasn't been programmed to parse yet beyond basic function declarations and statements.

## Development Status

* **Lexer (Tokenizer)**: Largely complete and robust for defined Megaladon syntax elements. It converts source code into a stream of tokens.
* **Parser (Syntax Analyzer)**: This is the current focus. The provided `Parser.cpp` is a *skeleton* for a recursive descent parser. It demonstrates the structure and helper methods but does not yet fully implement the grammar rules to build a complete Abstract Syntax Tree (AST). You will see messages indicating that parsing has started/finished, but also "Unexpected token" errors for syntax not yet implemented.

## Contribution / Next Steps

The immediate next step is to **implement the grammar rules within `src/Parser.cpp`** to correctly build the AST from the token stream. This involves:
* Defining concrete parsing methods for each grammatical rule (e.g., `parseFunctionDeclaration()`, `parseIfStatement()`, `parseExpression()`).
* Handling operator precedence and associativity in expression parsing.
* Building `ASTNode` objects to represent the parsed structure.

This is a significant undertaking, and the provided placeholder serves as a starting point.
