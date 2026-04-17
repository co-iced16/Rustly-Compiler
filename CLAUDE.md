# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Rustly-Compiler is a lexer and parser implementation for a Rust-like subset language. It's a compiler frontend that performs lexical analysis and syntax analysis, outputting token streams and abstract syntax trees (AST).

## Build and Run

```bash
# Build the project
make

# Clean build artifacts
make clean

# Run all tests
make test

# Run on a specific file (Windows)
./rustly-compiler.exe tests/test_basic.txt

# Run on a specific file (Linux/macOS)
./rustly-compiler tests/test_basic.txt

# Show token stream only
./rustly-compiler.exe tests/test_basic.txt --tokens-only

# Verbose output (shows both tokens and AST)
./rustly-compiler.exe tests/test_basic.txt --verbose
```

**Note:** On Windows, the executable is `rustly-compiler.exe`. On Linux/macOS, it's `rustly-compiler`.

## Architecture

The compiler follows a classic three-stage pipeline:

```
Source Code → Lexer → Token Stream → Parser → AST → Output
```

### Lexer (lexer.h/cpp)

- **State machine approach** with character-by-character scanning
- **Longest match principle** for operators (e.g., `==` vs `=`, `->` vs `-`)
- **Keyword recognition** via hash table lookup in `token.h`
- **Comment handling**: Skips `//` line comments and `/* */` block comments
- Tracks line and column numbers for error reporting

Key methods:
- `nextToken()`: Returns next token, skipping whitespace/comments
- `tokenize()`: Returns complete token vector
- `scanIdentifierOrKeyword()`: Distinguishes keywords from identifiers
- `scanNumber()`: Parses integer literals

### Parser (parser.h/cpp)

- **Recursive descent parser** with one method per grammar rule
- **Left recursion eliminated** through iteration (e.g., binary expressions)
- **Operator precedence** handled by parsing hierarchy:
  - `parseExpression()`: Comparison operators (`==`, `!=`, `<`, `<=`, `>`, `>=`)
  - `parseAddSubExpr()`: Addition/subtraction (`+`, `-`)
  - `parseTerm()`: Multiplication/division (`*`, `/`)
  - `parsePrimary()`: Literals, identifiers, function calls, parenthesized expressions

Key methods:
- `parseProgram()`: Entry point, returns `Program` AST node
- `parseStatement()`: Dispatches to specific statement parsers based on lookahead
- `consume()`: Matches expected token or throws error with location

### AST (ast.h/cpp)

All AST nodes inherit from `ASTNode` with a `print()` method for visualization.

**Node hierarchy:**
- `Program`: Top-level, contains `Declaration` list
- `Declaration`: `FunctionDecl`
- `Statement`: `LetStmt`, `AssignStmt`, `IfStmt`, `WhileStmt`, `ReturnStmt`, `ExprStmt`, `EmptyStmt`
- `Expression`: `BinaryExpr`, `CallExpr`, `Identifier`, `NumberLiteral`
- `Type`: `I32Type`
- `Block`: Contains `Statement` list

**Memory management:** Uses `std::unique_ptr` for ownership, `std::vector` for collections.

## Grammar Coverage

The parser implements 19 mandatory grammar rules from the assignment:

- **0.1-0.3**: Variable attributes (`mut`), types (`i32`), lvalues
- **1.1-1.5**: Program structure, statements, functions (parameters, return types)
- **2.0-2.3**: Variable declarations and assignments
- **3.1-3.5**: Expressions (literals, identifiers, binary ops, function calls)
- **4.1**: If statements with optional else
- **5.0-5.1**: While loops

## Error Handling

Errors include:
- **Location**: Line and column numbers
- **Context**: Expected vs actual token
- **Example**: `Error at line 3, column 5: Expected ';' after variable declaration (got 'return')`

Thrown as `std::runtime_error` from `Parser::consume()` and `Parser::error()`.

## Testing

Test files in `tests/` demonstrate different language features:
- `test_basic.txt`: Minimal function with return
- `test_expr.txt`: Arithmetic expressions with operator precedence
- `test_func.txt`: Multiple functions and function calls
- `test_control.txt`: If statements and while loops

## Code Conventions

- **C++17** standard
- **Smart pointers**: `std::unique_ptr` for AST nodes (no raw `new`/`delete`)
- **Move semantics**: AST construction uses `std::move()` to transfer ownership
- **Const correctness**: Helper methods like `peek()`, `check()` are const
- **RAII**: Lexer and Parser manage their own state

## Important Implementation Details

1. **Token lookahead**: Parser uses `peek()` to check next token without consuming
2. **Backtracking**: Assignment vs expression statement resolved by looking ahead after identifier
3. **Expression parsing**: Uses precedence climbing via separate methods for each precedence level
4. **Parameter passing**: `Parameter` struct uses move semantics for `std::unique_ptr<Type>`
5. **Block structure**: `Block` is separate from `Statement` to allow reuse in if/while bodies

## Extending the Compiler

To add new features:

1. **New token type**: Add to `TokenType` enum in `token.h` and update `tokenTypeToString()`
2. **New keyword**: Add to `keywords` map in `token.h`
3. **New AST node**: Inherit from appropriate base class in `ast.h`, implement `print()`
4. **New grammar rule**: Add parsing method to `parser.h/cpp` following existing patterns
5. **Update tests**: Add test case in `tests/` directory
