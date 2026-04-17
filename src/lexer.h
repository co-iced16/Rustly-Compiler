#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <string>
#include <vector>

class Lexer {
public:
    explicit Lexer(const std::string& source);

    // 获取下一个 Token
    Token nextToken();

    // 获取所有 Token（用于一次性扫描）
    std::vector<Token> tokenize();

private:
    std::string source;      // 源代码
    size_t current;          // 当前位置
    int line;                // 当前行号
    int column;              // 当前列号

    // 辅助方法
    bool isAtEnd() const;
    char peek() const;
    char peekNext() const;
    char advance();
    void skipWhitespace();
    void skipLineComment();
    void skipBlockComment();

    // 扫描方法
    Token scanIdentifierOrKeyword();
    Token scanNumber();
    Token makeToken(TokenType type, const std::string& lexeme);

    // 字符判断
    bool isAlpha(char c) const;
    bool isDigit(char c) const;
    bool isAlphaNumeric(char c) const;
};

#endif // LEXER_H
