#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "lexer.h"
#include "ast.h"
#include <vector>
#include <memory>
#include <stdexcept>

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    // 解析入口
    std::unique_ptr<Program> parseProgram();

private:
    std::vector<Token> tokens;
    size_t current;

    // 辅助方法
    Token peek() const;
    Token previous() const;
    Token advance();
    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    Token consume(TokenType type, const std::string& message);
    void error(const std::string& message);

    // 解析方法（按文法规则组织）

    // 1.1 基础程序
    std::vector<std::unique_ptr<Declaration>> parseDeclarations();
    std::unique_ptr<Declaration> parseDeclaration();
    std::unique_ptr<FunctionDecl> parseFunctionDecl();

    // 1.4 函数输入
    std::vector<Parameter> parseParameterList();
    Parameter parseParameter();

    // 1.5 函数输出
    std::unique_ptr<Type> parseReturnType();

    // 0.2 类型
    std::unique_ptr<Type> parseType();

    // 1.1, 1.2 语句块和语句串
    std::unique_ptr<Block> parseBlock();
    std::vector<std::unique_ptr<Statement>> parseStatements();
    std::unique_ptr<Statement> parseStatement();

    // 2.1, 2.3 变量声明语句
    std::unique_ptr<LetStmt> parseLetStmt();

    // 2.2 赋值语句
    std::unique_ptr<AssignStmt> parseAssignStmt(const std::string& name);

    // 1.3 返回语句
    std::unique_ptr<ReturnStmt> parseReturnStmt();

    // 4.1 选择结构
    std::unique_ptr<IfStmt> parseIfStmt();

    // 5.1 循环结构
    std::unique_ptr<WhileStmt> parseWhileStmt();

    // 5.2 for 循环
    std::unique_ptr<ForStmt> parseForStmt();

    // 5.3 loop 循环
    std::unique_ptr<LoopStmt> parseLoopStmt();

    // 3.1-3.5 表达式
    std::unique_ptr<Expression> parseExpression();        // 比较表达式
    std::unique_ptr<Expression> parseAddSubExpr();        // 加减表达式
    std::unique_ptr<Expression> parseTerm();              // 乘除表达式
    std::unique_ptr<Expression> parsePostfix();           // 后缀表达式（数组索引）
    std::unique_ptr<Expression> parseFactor();            // 因子
    std::unique_ptr<Expression> parsePrimary();           // 基本表达式
};

#endif // PARSER_H
