#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " <input-file> [options]\n";
    std::cout << "Options:\n";
    std::cout << "  --tokens-only    Only output token stream\n";
    std::cout << "  --ast            Output AST (default)\n";
    std::cout << "  --verbose        Verbose output\n";
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void printTokens(const std::vector<Token>& tokens) {
    std::cout << "=== Token Stream ===\n";
    for (const auto& token : tokens) {
        std::cout << "[" << token.line << ":" << token.column << "] "
                  << tokenTypeToString(token.type);
        if (!token.lexeme.empty()) {
            std::cout << " '" << token.lexeme << "'";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string filename = argv[1];
    bool tokensOnly = false;
    bool verbose = false;

    // 解析命令行参数
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--tokens-only") {
            tokensOnly = true;
        } else if (arg == "--ast") {
            tokensOnly = false;
        } else if (arg == "--verbose") {
            verbose = true;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage(argv[0]);
            return 1;
        }
    }

    try {
        // 读取源文件
        if (verbose) {
            std::cout << "Reading file: " << filename << "\n";
        }
        std::string source = readFile(filename);

        // 词法分析
        if (verbose) {
            std::cout << "Lexical analysis...\n";
        }
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();

        if (tokensOnly) {
            printTokens(tokens);
            return 0;
        }

        if (verbose) {
            printTokens(tokens);
        }

        // 语法分析
        if (verbose) {
            std::cout << "Syntax analysis...\n";
        }
        Parser parser(tokens);
        auto program = parser.parseProgram();

        // 输出 AST
        std::cout << "=== Abstract Syntax Tree ===\n";
        program->print(0);

        if (verbose) {
            std::cout << "\nParsing completed successfully!\n";
        }

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
