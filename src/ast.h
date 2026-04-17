#ifndef AST_H
#define AST_H

#include "token.h"
#include <memory>
#include <vector>
#include <string>

// 前向声明
class ASTNode;
class Expression;
class Statement;
class Declaration;
class Type;
class Block;

// ============ 基类 ============

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0;
};

// ============ 类型 ============

class Type : public ASTNode {
public:
    virtual ~Type() = default;
};

class I32Type : public Type {
public:
    void print(int indent = 0) const override;
};

// ============ 表达式 ============

class Expression : public ASTNode {
public:
    virtual ~Expression() = default;
};

class NumberLiteral : public Expression {
public:
    int value;

    explicit NumberLiteral(int val) : value(val) {}
    void print(int indent = 0) const override;
};

class Identifier : public Expression {
public:
    std::string name;

    explicit Identifier(const std::string& n) : name(n) {}
    void print(int indent = 0) const override;
};

class BinaryExpr : public Expression {
public:
    std::unique_ptr<Expression> left;
    TokenType op;
    std::unique_ptr<Expression> right;

    BinaryExpr(std::unique_ptr<Expression> l, TokenType o, std::unique_ptr<Expression> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
    void print(int indent = 0) const override;
};

class CallExpr : public Expression {
public:
    std::string function;
    std::vector<std::unique_ptr<Expression>> args;

    CallExpr(const std::string& func, std::vector<std::unique_ptr<Expression>> arguments)
        : function(func), args(std::move(arguments)) {}
    void print(int indent = 0) const override;
};

// ============ 语句 ============

class Statement : public ASTNode {
public:
    virtual ~Statement() = default;
};

class Block : public ASTNode {
public:
    std::vector<std::unique_ptr<Statement>> statements;

    void print(int indent = 0) const override;
};

class EmptyStmt : public Statement {
public:
    void print(int indent = 0) const override;
};

class ExprStmt : public Statement {
public:
    std::unique_ptr<Expression> expr;

    explicit ExprStmt(std::unique_ptr<Expression> e) : expr(std::move(e)) {}
    void print(int indent = 0) const override;
};

class ReturnStmt : public Statement {
public:
    std::unique_ptr<Expression> value; // nullptr 表示 return;

    explicit ReturnStmt(std::unique_ptr<Expression> val = nullptr)
        : value(std::move(val)) {}
    void print(int indent = 0) const override;
};

class LetStmt : public Statement {
public:
    bool isMut;
    std::string name;
    std::unique_ptr<Type> type; // nullptr 表示类型推导
    std::unique_ptr<Expression> init; // nullptr 表示未初始化

    LetStmt(bool mut, const std::string& n,
            std::unique_ptr<Type> t = nullptr,
            std::unique_ptr<Expression> i = nullptr)
        : isMut(mut), name(n), type(std::move(t)), init(std::move(i)) {}
    void print(int indent = 0) const override;
};

class AssignStmt : public Statement {
public:
    std::string target; // 简化版：只支持标识符作为左值
    std::unique_ptr<Expression> value;

    AssignStmt(const std::string& t, std::unique_ptr<Expression> v)
        : target(t), value(std::move(v)) {}
    void print(int indent = 0) const override;
};

class IfStmt : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> thenBlock;
    std::unique_ptr<Block> elseBlock; // nullptr 表示没有 else

    IfStmt(std::unique_ptr<Expression> cond,
           std::unique_ptr<Block> thenB,
           std::unique_ptr<Block> elseB = nullptr)
        : condition(std::move(cond)), thenBlock(std::move(thenB)),
          elseBlock(std::move(elseB)) {}
    void print(int indent = 0) const override;
};

class WhileStmt : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> body;

    WhileStmt(std::unique_ptr<Expression> cond, std::unique_ptr<Block> b)
        : condition(std::move(cond)), body(std::move(b)) {}
    void print(int indent = 0) const override;
};

// ============ 声明 ============

class Declaration : public ASTNode {
public:
    virtual ~Declaration() = default;
};

struct Parameter {
    bool isMut;
    std::string name;
    std::unique_ptr<Type> type;

    Parameter(bool mut, const std::string& n, std::unique_ptr<Type> t)
        : isMut(mut), name(n), type(std::move(t)) {}
};

class FunctionDecl : public Declaration {
public:
    std::string name;
    std::vector<Parameter> params;
    std::unique_ptr<Type> returnType; // nullptr 表示无返回值
    std::unique_ptr<Block> body;

    FunctionDecl(const std::string& n,
                 std::vector<Parameter> p,
                 std::unique_ptr<Type> ret,
                 std::unique_ptr<Block> b)
        : name(n), params(std::move(p)), returnType(std::move(ret)),
          body(std::move(b)) {}
    void print(int indent = 0) const override;
};

// ============ 程序 ============

class Program : public ASTNode {
public:
    std::vector<std::unique_ptr<Declaration>> declarations;

    void print(int indent = 0) const override;
};

#endif // AST_H
