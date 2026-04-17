#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <unordered_map>

// Token 类型枚举
enum class TokenType {
    // 关键字 (Keywords)
    KW_I32,         // i32
    KW_LET,         // let
    KW_IF,          // if
    KW_ELSE,        // else
    KW_WHILE,       // while
    KW_RETURN,      // return
    KW_MUT,         // mut
    KW_FN,          // fn
    KW_FOR,         // for
    KW_IN,          // in
    KW_LOOP,        // loop
    KW_BREAK,       // break
    KW_CONTINUE,    // continue
    KW_STRUCT,      // struct
    KW_MATCH,       // match
    KW_UNDERSCORE,  // _ (通配符)

    // 标识符和字面量 (Identifiers and Literals)
    IDENTIFIER,     // 标识符
    NUMBER,         // 数字字面量

    // 运算符 (Operators)
    PLUS,           // +
    MINUS,          // -
    STAR,           // *
    SLASH,          // /
    EQ,             // ==
    NE,             // !=
    LT,             // <
    LE,             // <=
    GT,             // >
    GE,             // >=
    ASSIGN,         // =
    AMPERSAND,      // &

    // 界符 (Delimiters)
    LPAREN,         // (
    RPAREN,         // )
    LBRACE,         // {
    RBRACE,         // }
    LBRACKET,       // [
    RBRACKET,       // ]

    // 分隔符 (Separators)
    SEMICOLON,      // ;
    COLON,          // :
    COMMA,          // ,

    // 特殊符号 (Special Symbols)
    ARROW,          // ->
    DOT,            // .
    DOTDOT,         // ..
    FAT_ARROW,      // =>

    // 其他 (Others)
    END_OF_FILE,    // 文件结束
    INVALID         // 无效记号
};

// Token 结构体
struct Token {
    TokenType type;      // 记号类型
    std::string lexeme;  // 词素（原始文本）
    int line;            // 行号
    int column;          // 列号

    Token() : type(TokenType::INVALID), line(0), column(0) {}

    Token(TokenType t, const std::string& lex, int l, int c)
        : type(t), lexeme(lex), line(l), column(c) {}
};

// 关键字映射表
static const std::unordered_map<std::string, TokenType> keywords = {
    {"i32", TokenType::KW_I32},
    {"let", TokenType::KW_LET},
    {"if", TokenType::KW_IF},
    {"else", TokenType::KW_ELSE},
    {"while", TokenType::KW_WHILE},
    {"return", TokenType::KW_RETURN},
    {"mut", TokenType::KW_MUT},
    {"fn", TokenType::KW_FN},
    {"for", TokenType::KW_FOR},
    {"in", TokenType::KW_IN},
    {"loop", TokenType::KW_LOOP},
    {"break", TokenType::KW_BREAK},
    {"continue", TokenType::KW_CONTINUE},
    {"struct", TokenType::KW_STRUCT},
    {"match", TokenType::KW_MATCH}
};

// Token 类型转字符串（用于调试和输出）
inline std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::KW_I32: return "KW_I32";
        case TokenType::KW_LET: return "KW_LET";
        case TokenType::KW_IF: return "KW_IF";
        case TokenType::KW_ELSE: return "KW_ELSE";
        case TokenType::KW_WHILE: return "KW_WHILE";
        case TokenType::KW_RETURN: return "KW_RETURN";
        case TokenType::KW_MUT: return "KW_MUT";
        case TokenType::KW_FN: return "KW_FN";
        case TokenType::KW_FOR: return "KW_FOR";
        case TokenType::KW_IN: return "KW_IN";
        case TokenType::KW_LOOP: return "KW_LOOP";
        case TokenType::KW_BREAK: return "KW_BREAK";
        case TokenType::KW_CONTINUE: return "KW_CONTINUE";
        case TokenType::KW_STRUCT: return "KW_STRUCT";
        case TokenType::KW_MATCH: return "KW_MATCH";
        case TokenType::KW_UNDERSCORE: return "KW_UNDERSCORE";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::EQ: return "EQ";
        case TokenType::NE: return "NE";
        case TokenType::LT: return "LT";
        case TokenType::LE: return "LE";
        case TokenType::GT: return "GT";
        case TokenType::GE: return "GE";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::AMPERSAND: return "AMPERSAND";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::COLON: return "COLON";
        case TokenType::COMMA: return "COMMA";
        case TokenType::ARROW: return "ARROW";
        case TokenType::DOT: return "DOT";
        case TokenType::DOTDOT: return "DOTDOT";
        case TokenType::FAT_ARROW: return "FAT_ARROW";
        case TokenType::END_OF_FILE: return "END_OF_FILE";
        case TokenType::INVALID: return "INVALID";
        default: return "UNKNOWN";
    }
}

#endif // TOKEN_H
