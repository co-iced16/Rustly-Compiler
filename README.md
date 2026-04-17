# Rustly-Compiler

一个类 Rust 子集语言的词法分析器和语法分析器实现。

## 项目简介

本项目是编译原理课程的大作业，实现了一个简化版的 Rust 语言编译器前端，包括：

- **词法分析器（Lexer）**：将源代码转换为记号流
- **语法分析器（Parser）**：构建抽象语法树（AST）

## 功能特性

### 已实现的必做功能（19 条语法规则）

- ✅ 0.1 变量属性（`mut`）
- ✅ 0.2 类型（`i32`）
- ✅ 0.3 左值（标识符）
- ✅ 1.1 基础程序结构
- ✅ 1.2 语句（空语句 `;`）
- ✅ 1.3 返回语句（`return;`）
- ✅ 1.4 函数输入（形参列表）
- ✅ 1.5 函数输出（返回类型）
- ✅ 2.0 变量声明
- ✅ 2.1 变量声明语句
- ✅ 2.2 赋值语句
- ✅ 2.3 变量声明赋值语句
- ✅ 3.1 基本表达式
- ✅ 3.2 比较运算
- ✅ 3.3 加减运算
- ✅ 3.4 乘除运算
- ✅ 3.5 函数调用
- ✅ 4.1 选择结构（if）
- ✅ 5.0 循环语句
- ✅ 5.1 while 循环

### 词法分析器特性

- 识别所有关键字（15 个）
- 识别标识符和数值字面量
- 识别运算符（`+`, `-`, `*`, `/`, `==`, `!=`, `<`, `<=`, `>`, `>=`, `=`）
- 识别界符和分隔符
- 识别特殊符号（`->`, `.`, `..`, `=>`）
- 处理注释（`//` 和 `/* */`）
- 最长匹配原则

### 语法分析器特性

- 递归下降解析
- 构建完整的 AST
- 详细的错误报告（行号、列号）
- 支持表达式优先级

## 项目结构

```
Rustly-Compiler/
├── src/
│   ├── token.h          # Token 定义
│   ├── lexer.h          # 词法分析器头文件
│   ├── lexer.cpp        # 词法分析器实现
│   ├── ast.h            # AST 节点定义
│   ├── ast.cpp          # AST 节点实现
│   ├── parser.h         # 语法分析器头文件
│   ├── parser.cpp       # 语法分析器实现
│   └── main.cpp         # 主程序
├── tests/
│   ├── test_basic.txt   # 基础测试
│   ├── test_expr.txt    # 表达式测试
│   ├── test_func.txt    # 函数调用测试
│   └── test_control.txt # 控制流测试
├── Makefile             # 构建文件
└── README.md            # 项目说明
```

## 编译和运行

### 环境要求

- C++17 或更高版本
- g++ 编译器
- Make 构建工具

### 编译项目

```bash
make
```

### 运行编译器

```bash
# 基本用法
./rustly-compiler <input-file>

# 仅输出记号流
./rustly-compiler <input-file> --tokens-only

# 输出 AST（默认）
./rustly-compiler <input-file> --ast

# 详细输出
./rustly-compiler <input-file> --verbose
```

### 运行测试

```bash
make test
```

### 清理构建文件

```bash
make clean
```

## 使用示例

### 示例 1：基础程序

输入文件 `test_basic.txt`：
```rust
fn main() {
    return;
}
```

运行：
```bash
./rustly-compiler tests/test_basic.txt
```

输出：
```
=== Abstract Syntax Tree ===
fn main() {
  return;
}
```

### 示例 2：表达式计算

输入文件 `test_expr.txt`：
```rust
fn calculate(mut x: i32, mut y: i32) -> i32 {
    let mut result: i32;
    result = x + y * 2;
    return result;
}
```

### 示例 3：控制流

输入文件 `test_control.txt`：
```rust
fn max(mut a: i32, mut b: i32) -> i32 {
    if a > b {
        return a;
    }
    return b;
}

fn sum(mut n: i32) -> i32 {
    let mut result: i32;
    result = 0;
    let mut i: i32;
    i = 1;
    while i <= n {
        result = result + i;
        i = i + 1;
    }
    return result;
}
```

## 语法规则

支持的语法包括：

- **函数定义**：`fn name(params) -> type { body }`
- **变量声明**：`let mut name: type;` 或 `let mut name: type = expr;`
- **赋值语句**：`name = expr;`
- **返回语句**：`return;` 或 `return expr;`
- **if 语句**：`if condition { block }`
- **while 循环**：`while condition { block }`
- **表达式**：支持算术运算（`+`, `-`, `*`, `/`）和比较运算（`==`, `!=`, `<`, `<=`, `>`, `>=`）
- **函数调用**：`function(args)`

## 错误处理

编译器提供详细的错误信息，包括：

- 错误位置（行号和列号）
- 错误类型描述
- 期望的记号类型

示例错误输出：
```
Error at line 3, column 5: Expected ';' after variable declaration (got 'return')
```

## 技术实现

### 词法分析

- 使用状态机方法
- 最长匹配原则
- 支持注释跳过
- 关键字识别通过哈希表查找

### 语法分析

- 递归下降解析器
- 左递归消除（通过迭代实现）
- 运算符优先级处理
- 错误恢复机制

## 作者

编译原理课程大作业

## 许可证

本项目仅用于学习和教学目的。
