#include "lexer.h"
#include <cctype>

Lexer::Lexer(const std::string& source)
    : source(source), current(0), line(1), column(1) {}

bool Lexer::isAtEnd() const {
    return current >= source.length();
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[current];
}

char Lexer::peekNext() const {
    if (current + 1 >= source.length()) return '\0';
    return source[current + 1];
}

char Lexer::advance() {
    if (isAtEnd()) return '\0';
    char c = source[current++];
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

bool Lexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
        } else {
            break;
        }
    }
}

void Lexer::skipLineComment() {
    // 跳过 // 注释，直到行尾
    while (!isAtEnd() && peek() != '\n') {
        advance();
    }
}

void Lexer::skipBlockComment() {
    // 跳过 /* */ 注释
    while (!isAtEnd()) {
        if (peek() == '*' && peekNext() == '/') {
            advance(); // 消费 *
            advance(); // 消费 /
            break;
        }
        advance();
    }
}

Token Lexer::makeToken(TokenType type, const std::string& lexeme) {
    return Token(type, lexeme, line, column - lexeme.length());
}

Token Lexer::scanIdentifierOrKeyword() {
    int startColumn = column;
    std::string lexeme;

    while (!isAtEnd() && isAlphaNumeric(peek())) {
        lexeme += advance();
    }

    // 检查是否为关键字
    auto it = keywords.find(lexeme);
    if (it != keywords.end()) {
        return Token(it->second, lexeme, line, startColumn);
    }

    // 特殊处理单独的下划线作为通配符
    if (lexeme == "_") {
        return Token(TokenType::KW_UNDERSCORE, lexeme, line, startColumn);
    }

    return Token(TokenType::IDENTIFIER, lexeme, line, startColumn);
}

Token Lexer::scanNumber() {
    int startColumn = column;
    std::string lexeme;

    while (!isAtEnd() && isDigit(peek())) {
        lexeme += advance();
    }

    return Token(TokenType::NUMBER, lexeme, line, startColumn);
}

Token Lexer::nextToken() {
    skipWhitespace();

    if (isAtEnd()) {
        return makeToken(TokenType::END_OF_FILE, "");
    }

    int startColumn = column;
    char c = peek();

    // 标识符或关键字
    if (isAlpha(c)) {
        return scanIdentifierOrKeyword();
    }

    // 数字
    if (isDigit(c)) {
        return scanNumber();
    }

    // 单字符和多字符记号
    advance(); // 消费当前字符

    switch (c) {
        // 单字符记号
        case '(': return Token(TokenType::LPAREN, "(", line, startColumn);
        case ')': return Token(TokenType::RPAREN, ")", line, startColumn);
        case '{': return Token(TokenType::LBRACE, "{", line, startColumn);
        case '}': return Token(TokenType::RBRACE, "}", line, startColumn);
        case '[': return Token(TokenType::LBRACKET, "[", line, startColumn);
        case ']': return Token(TokenType::RBRACKET, "]", line, startColumn);
        case ';': return Token(TokenType::SEMICOLON, ";", line, startColumn);
        case ':': return Token(TokenType::COLON, ":", line, startColumn);
        case ',': return Token(TokenType::COMMA, ",", line, startColumn);
        case '+': return Token(TokenType::PLUS, "+", line, startColumn);
        case '*': return Token(TokenType::STAR, "*", line, startColumn);
        case '&': return Token(TokenType::AMPERSAND, "&", line, startColumn);

        // 可能是多字符的记号
        case '-':
            if (peek() == '>') {
                advance();
                return Token(TokenType::ARROW, "->", line, startColumn);
            }
            return Token(TokenType::MINUS, "-", line, startColumn);

        case '=':
            if (peek() == '=') {
                advance();
                return Token(TokenType::EQ, "==", line, startColumn);
            } else if (peek() == '>') {
                advance();
                return Token(TokenType::FAT_ARROW, "=>", line, startColumn);
            }
            return Token(TokenType::ASSIGN, "=", line, startColumn);

        case '!':
            if (peek() == '=') {
                advance();
                return Token(TokenType::NE, "!=", line, startColumn);
            }
            return Token(TokenType::INVALID, "!", line, startColumn);

        case '<':
            if (peek() == '=') {
                advance();
                return Token(TokenType::LE, "<=", line, startColumn);
            }
            return Token(TokenType::LT, "<", line, startColumn);

        case '>':
            if (peek() == '=') {
                advance();
                return Token(TokenType::GE, ">=", line, startColumn);
            }
            return Token(TokenType::GT, ">", line, startColumn);

        case '.':
            if (peek() == '.') {
                advance();
                return Token(TokenType::DOTDOT, "..", line, startColumn);
            }
            return Token(TokenType::DOT, ".", line, startColumn);

        case '/':
            if (peek() == '/') {
                advance();
                skipLineComment();
                return nextToken(); // 递归获取下一个有效 Token
            } else if (peek() == '*') {
                advance();
                skipBlockComment();
                return nextToken(); // 递归获取下一个有效 Token
            }
            return Token(TokenType::SLASH, "/", line, startColumn);

        case '#':
            return Token(TokenType::END_OF_FILE, "#", line, startColumn);

        default:
            return Token(TokenType::INVALID, std::string(1, c), line, startColumn);
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    Token token;

    do {
        token = nextToken();
        tokens.push_back(token);
    } while (token.type != TokenType::END_OF_FILE);

    return tokens;
}
