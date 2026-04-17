#include "parser.h"
#include <iostream>
#include <sstream>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), current(0) {}

// ============ 辅助方法 ============

Token Parser::peek() const {
    return tokens[current];
}

Token Parser::previous() const {
    return tokens[current - 1];
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::END_OF_FILE;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();

    std::stringstream ss;
    ss << "Error at line " << peek().line << ", column " << peek().column
       << ": " << message << " (got '" << peek().lexeme << "')";
    throw std::runtime_error(ss.str());
}

void Parser::error(const std::string& message) {
    std::stringstream ss;
    ss << "Error at line " << peek().line << ", column " << peek().column
       << ": " << message;
    throw std::runtime_error(ss.str());
}

// ============ 1.1 基础程序 ============

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();
    program->declarations = parseDeclarations();
    return program;
}

std::vector<std::unique_ptr<Declaration>> Parser::parseDeclarations() {
    std::vector<std::unique_ptr<Declaration>> declarations;

    while (!isAtEnd()) {
        declarations.push_back(parseDeclaration());
    }

    return declarations;
}

std::unique_ptr<Declaration> Parser::parseDeclaration() {
    if (check(TokenType::KW_FN)) {
        return parseFunctionDecl();
    }

    error("Expected declaration (fn)");
    return nullptr;
}

std::unique_ptr<FunctionDecl> Parser::parseFunctionDecl() {
    consume(TokenType::KW_FN, "Expected 'fn'");

    Token nameToken = consume(TokenType::IDENTIFIER, "Expected function name");
    std::string name = nameToken.lexeme;

    consume(TokenType::LPAREN, "Expected '(' after function name");
    std::vector<Parameter> params = parseParameterList();
    consume(TokenType::RPAREN, "Expected ')' after parameters");

    std::unique_ptr<Type> returnType = nullptr;
    if (match(TokenType::ARROW)) {
        returnType = parseReturnType();
    }

    auto body = parseBlock();

    return std::make_unique<FunctionDecl>(name, std::move(params),
                                          std::move(returnType), std::move(body));
}

// ============ 1.4 函数输入 ============

std::vector<Parameter> Parser::parseParameterList() {
    std::vector<Parameter> params;

    if (check(TokenType::RPAREN)) {
        return params; // 空参数列表
    }

    do {
        params.push_back(parseParameter());
    } while (match(TokenType::COMMA));

    return params;
}

Parameter Parser::parseParameter() {
    bool isMut = match(TokenType::KW_MUT);

    Token nameToken = consume(TokenType::IDENTIFIER, "Expected parameter name");
    std::string name = nameToken.lexeme;

    consume(TokenType::COLON, "Expected ':' after parameter name");

    auto type = parseType();

    return Parameter(isMut, name, std::move(type));
}

// ============ 1.5 函数输出 & 0.2 类型 ============

std::unique_ptr<Type> Parser::parseReturnType() {
    return parseType();
}

std::unique_ptr<Type> Parser::parseType() {
    if (match(TokenType::KW_I32)) {
        return std::make_unique<I32Type>();
    }

    error("Expected type (i32)");
    return nullptr;
}

// ============ 1.1, 1.2 语句块和语句串 ============

std::unique_ptr<Block> Parser::parseBlock() {
    consume(TokenType::LBRACE, "Expected '{'");

    auto block = std::make_unique<Block>();
    block->statements = parseStatements();

    consume(TokenType::RBRACE, "Expected '}'");

    return block;
}

std::vector<std::unique_ptr<Statement>> Parser::parseStatements() {
    std::vector<std::unique_ptr<Statement>> statements;

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        statements.push_back(parseStatement());
    }

    return statements;
}

// ============ 语句解析 ============

std::unique_ptr<Statement> Parser::parseStatement() {
    // 1.2 空语句
    if (match(TokenType::SEMICOLON)) {
        return std::make_unique<EmptyStmt>();
    }

    // 1.3 返回语句
    if (check(TokenType::KW_RETURN)) {
        return parseReturnStmt();
    }

    // 2.1, 2.3 变量声明语句
    if (check(TokenType::KW_LET)) {
        return parseLetStmt();
    }

    // 4.1 选择结构
    if (check(TokenType::KW_IF)) {
        return parseIfStmt();
    }

    // 5.1 循环结构
    if (check(TokenType::KW_WHILE)) {
        return parseWhileStmt();
    }

    // 2.2 赋值语句或表达式语句
    if (check(TokenType::IDENTIFIER)) {
        Token nameToken = advance();
        std::string name = nameToken.lexeme;

        if (match(TokenType::ASSIGN)) {
            // 赋值语句
            auto value = parseExpression();
            consume(TokenType::SEMICOLON, "Expected ';' after assignment");
            return std::make_unique<AssignStmt>(name, std::move(value));
        } else {
            // 回退，作为表达式语句处理
            current--;
            auto expr = parseExpression();
            consume(TokenType::SEMICOLON, "Expected ';' after expression");
            return std::make_unique<ExprStmt>(std::move(expr));
        }
    }

    // 表达式语句
    auto expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression");
    return std::make_unique<ExprStmt>(std::move(expr));
}

// ============ 2.1, 2.3 变量声明语句 ============

std::unique_ptr<LetStmt> Parser::parseLetStmt() {
    consume(TokenType::KW_LET, "Expected 'let'");

    bool isMut = match(TokenType::KW_MUT);

    Token nameToken = consume(TokenType::IDENTIFIER, "Expected variable name");
    std::string name = nameToken.lexeme;

    std::unique_ptr<Type> type = nullptr;
    if (match(TokenType::COLON)) {
        type = parseType();
    }

    std::unique_ptr<Expression> init = nullptr;
    if (match(TokenType::ASSIGN)) {
        init = parseExpression();
    }

    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");

    return std::make_unique<LetStmt>(isMut, name, std::move(type), std::move(init));
}

// ============ 2.2 赋值语句 ============

std::unique_ptr<AssignStmt> Parser::parseAssignStmt(const std::string& name) {
    auto value = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after assignment");
    return std::make_unique<AssignStmt>(name, std::move(value));
}

// ============ 1.3 返回语句 ============

std::unique_ptr<ReturnStmt> Parser::parseReturnStmt() {
    consume(TokenType::KW_RETURN, "Expected 'return'");

    std::unique_ptr<Expression> value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = parseExpression();
    }

    consume(TokenType::SEMICOLON, "Expected ';' after return statement");

    return std::make_unique<ReturnStmt>(std::move(value));
}

// ============ 4.1 选择结构 ============

std::unique_ptr<IfStmt> Parser::parseIfStmt() {
    consume(TokenType::KW_IF, "Expected 'if'");

    auto condition = parseExpression();
    auto thenBlock = parseBlock();

    std::unique_ptr<Block> elseBlock = nullptr;
    if (match(TokenType::KW_ELSE)) {
        elseBlock = parseBlock();
    }

    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBlock),
                                    std::move(elseBlock));
}

// ============ 5.1 循环结构 ============

std::unique_ptr<WhileStmt> Parser::parseWhileStmt() {
    consume(TokenType::KW_WHILE, "Expected 'while'");

    auto condition = parseExpression();
    auto body = parseBlock();

    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

// ============ 3.1-3.5 表达式 ============

// 3.2 比较表达式
std::unique_ptr<Expression> Parser::parseExpression() {
    auto left = parseAddSubExpr();

    while (check(TokenType::EQ) || check(TokenType::NE) ||
           check(TokenType::LT) || check(TokenType::LE) ||
           check(TokenType::GT) || check(TokenType::GE)) {
        TokenType op = advance().type;
        auto right = parseAddSubExpr();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }

    return left;
}

// 3.3 加减表达式
std::unique_ptr<Expression> Parser::parseAddSubExpr() {
    auto left = parseTerm();

    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        TokenType op = advance().type;
        auto right = parseTerm();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }

    return left;
}

// 3.4 乘除表达式
std::unique_ptr<Expression> Parser::parseTerm() {
    auto left = parseFactor();

    while (check(TokenType::STAR) || check(TokenType::SLASH)) {
        TokenType op = advance().type;
        auto right = parseFactor();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }

    return left;
}

// 3.1 因子
std::unique_ptr<Expression> Parser::parseFactor() {
    return parsePrimary();
}

// 3.1, 3.5 基本表达式
std::unique_ptr<Expression> Parser::parsePrimary() {
    // 数字字面量
    if (check(TokenType::NUMBER)) {
        Token token = advance();
        int value = std::stoi(token.lexeme);
        return std::make_unique<NumberLiteral>(value);
    }

    // 标识符或函数调用
    if (check(TokenType::IDENTIFIER)) {
        Token nameToken = advance();
        std::string name = nameToken.lexeme;

        // 3.5 函数调用
        if (match(TokenType::LPAREN)) {
            std::vector<std::unique_ptr<Expression>> args;

            if (!check(TokenType::RPAREN)) {
                do {
                    args.push_back(parseExpression());
                } while (match(TokenType::COMMA));
            }

            consume(TokenType::RPAREN, "Expected ')' after arguments");
            return std::make_unique<CallExpr>(name, std::move(args));
        }

        // 普通标识符
        return std::make_unique<Identifier>(name);
    }

    // 括号表达式
    if (match(TokenType::LPAREN)) {
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }

    error("Expected expression");
    return nullptr;
}

