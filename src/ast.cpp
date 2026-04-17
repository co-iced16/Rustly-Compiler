#include "ast.h"
#include <iostream>
#include <string>

// 辅助函数：打印缩进
static void printIndent(int indent) {
    for (int i = 0; i < indent; ++i) {
        std::cout << "  ";
    }
}

// ============ 类型 ============

void I32Type::print([[maybe_unused]] int indent) const {
    std::cout << "i32";
}

// ============ 表达式 ============

void NumberLiteral::print([[maybe_unused]] int indent) const {
    std::cout << value;
}

void Identifier::print([[maybe_unused]] int indent) const {
    std::cout << name;
}

void BinaryExpr::print([[maybe_unused]] int indent) const {
    std::cout << "(";
    left->print(0);
    std::cout << " ";

    // 打印运算符
    switch (op) {
        case TokenType::PLUS: std::cout << "+"; break;
        case TokenType::MINUS: std::cout << "-"; break;
        case TokenType::STAR: std::cout << "*"; break;
        case TokenType::SLASH: std::cout << "/"; break;
        case TokenType::EQ: std::cout << "=="; break;
        case TokenType::NE: std::cout << "!="; break;
        case TokenType::LT: std::cout << "<"; break;
        case TokenType::LE: std::cout << "<="; break;
        case TokenType::GT: std::cout << ">"; break;
        case TokenType::GE: std::cout << ">="; break;
        default: std::cout << "?"; break;
    }

    std::cout << " ";
    right->print(0);
    std::cout << ")";
}

void CallExpr::print([[maybe_unused]] int indent) const {
    std::cout << function << "(";
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) std::cout << ", ";
        args[i]->print(0);
    }
    std::cout << ")";
}

// ============ 语句 ============

void Block::print(int indent) const {
    printIndent(indent);
    std::cout << "{\n";
    for (const auto& stmt : statements) {
        stmt->print(indent + 1);
    }
    printIndent(indent);
    std::cout << "}\n";
}

void EmptyStmt::print(int indent) const {
    printIndent(indent);
    std::cout << ";\n";
}

void ExprStmt::print(int indent) const {
    printIndent(indent);
    expr->print(0);
    std::cout << ";\n";
}

void ReturnStmt::print(int indent) const {
    printIndent(indent);
    std::cout << "return";
    if (value) {
        std::cout << " ";
        value->print(0);
    }
    std::cout << ";\n";
}

void LetStmt::print(int indent) const {
    printIndent(indent);
    std::cout << "let ";
    if (isMut) std::cout << "mut ";
    std::cout << name;

    if (type) {
        std::cout << ": ";
        type->print(0);
    }

    if (init) {
        std::cout << " = ";
        init->print(0);
    }

    std::cout << ";\n";
}

void AssignStmt::print(int indent) const {
    printIndent(indent);
    std::cout << target << " = ";
    value->print(0);
    std::cout << ";\n";
}

void IfStmt::print(int indent) const {
    printIndent(indent);
    std::cout << "if ";
    condition->print(0);
    std::cout << " ";
    thenBlock->print(indent);

    if (elseBlock) {
        printIndent(indent);
        std::cout << "else ";
        elseBlock->print(indent);
    }
}

void WhileStmt::print(int indent) const {
    printIndent(indent);
    std::cout << "while ";
    condition->print(0);
    std::cout << " ";
    body->print(indent);
}

// ============ 声明 ============

void FunctionDecl::print(int indent) const {
    printIndent(indent);
    std::cout << "fn " << name << "(";

    for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0) std::cout << ", ";
        if (params[i].isMut) std::cout << "mut ";
        std::cout << params[i].name << ": ";
        params[i].type->print(0);
    }

    std::cout << ")";

    if (returnType) {
        std::cout << " -> ";
        returnType->print(0);
    }

    std::cout << " ";
    body->print(indent);
}

// ============ 程序 ============

void Program::print(int indent) const {
    for (const auto& decl : declarations) {
        decl->print(indent);
        std::cout << "\n";
    }
}
