# Rustly-Compiler 设计文档

## 1. 项目概述

### 1.1 项目简介

Rustly-Compiler 是一个类 Rust 子集语言的编译器前端实现，包含词法分析器（Lexer）和语法分析器（Parser）两大核心模块。项目接收符合类 Rust 语法的源代码文件，依次经过词法分析和语法分析两个阶段，最终输出记号流（Token Stream）和抽象语法树（AST）。

### 1.2 开发环境

- **编程语言**: C++17
- **编译器**: MinGW g++ 13.1.0
- **构建系统**: GNU Make
- **开发平台**: Windows 11

### 1.3 功能覆盖

| 类别 | 规则数 | 状态 |
|------|--------|------|
| 必做功能 | 19/19 | 全部实现 |
| 选做功能 | 20/20 | 全部实现 |
| 总计 | 39 条语法规则 | 完成 |

---

## 2. 词法分析器设计

### 2.1 设计原理

词法分析器是编译器的第一个阶段，负责将源代码字符流转换为记号（Token）序列。本实现采用**有限状态机**方法，通过逐字符扫描并依据最长匹配原则识别各类记号。

### 2.2 状态转换设计

词法分析器的核心是 `nextToken()` 方法，其工作流程如下：

```
        开始
         |
         v
    +---------+
    | 跳过空白 |-----> 遇到文件结束? -----> 返回 EOF
    +---------+
         |
         v
    +---------+
    | 读取字符 |
    +---------+
         |
    +----+----+----+----+
    |    |    |    |    |
    v    v    v    v    v
 字母  数字   /    -    =   其他字符
    |    |    |    |    |
    v    v    v    v    v
 扫描   扫描  检查   检查  检查
 标识符 数字 下一字符 下一字符 下一字符
    |    |    |    |    |
    v    v    v    v    v
 查表   返回  //?   ->?   ==?
 关键字  数字  是->   是->   是->
    |         跳过注释  ARROW  FAT_ARROW EQ
    v
 返回
 IDENTIFIER
 或 KEYWORD
```

### 2.3 关键字识别

关键字通过哈希表（`std::unordered_map`）进行匹配。标识符扫描完成后，查表判断是否为关键字：

```cpp
static const std::unordered_map<std::string, TokenType> keywords = {
    {"i32", KW_I32}, {"let", KW_LET}, {"if", KW_IF},
    {"else", KW_ELSE}, {"while", KW_WHILE}, {"return", KW_RETURN},
    {"mut", KW_MUT}, {"fn", KW_FN}, {"for", KW_FOR},
    {"in", KW_IN}, {"loop", KW_LOOP}, {"break", KW_BREAK},
    {"continue", KW_CONTINUE}, {"struct", KW_STRUCT}, {"match", KW_MATCH}
};
```

这种设计确保了 `if123` 被正确识别为标识符而非关键字加数字，因为扫描器会持续读取字母数字字符直到非字母数字字符为止，然后整串查表。

### 2.4 最长匹配原则

对于可能产生多字符记号的场景（如 `==` vs `=`、`->` vs `-`、`<=` vs `<`），实现采用**向前查看**（lookahead）策略：

```cpp
case '=':
    if (peek() == '=') { advance(); return Token(EQ, "==", ...); }
    else if (peek() == '>') { advance(); return Token(FAT_ARROW, "=>", ...); }
    return Token(ASSIGN, "=", ...);

case '-':
    if (peek() == '>') { advance(); return Token(ARROW, "->", ...); }
    return Token(MINUS, "-", ...);
```

### 2.5 注释处理

- **单行注释** `//`：跳过当前行剩余字符，递归调用 `nextToken()` 获取下一个有效记号
- **块注释** `/* */`：扫描直到遇到 `*/`，递归调用 `nextToken()` 获取下一个有效记号

### 2.6 位置追踪

每个记号附带行号和列号信息，扫描过程中实时更新：

```cpp
char Lexer::advance() {
    char c = source[current++];
    if (c == '\n') { line++; column = 1; }
    else { column++; }
    return c;
}
```

### 2.7 程序实例与运行结果

**测试输入** (`tests/test_identifiers.txt`)：验证 `if123` 被识别为标识符

```rust
fn test_keywords_as_prefix() -> i32 {
    let if123: i32 = 100;
    let letvar: i32 = 200;
    let whileloop: i32 = 300;
    let fn_name: i32 = (if123 + letvar) - whileloop;
    return fn_name;
}
```

**运行命令**：
```bash
./rustly-compiler.exe tests/test_identifiers.txt --tokens-only
```

**输出结果**（截取关键部分）：
```
=== Token Stream ===
[2:1]  KW_FN 'fn'
[2:4]  IDENTIFIER 'test_keywords_as_prefix'
...
[4:9]  IDENTIFIER 'if123'      ← if123 被正确识别为标识符
[4:14] COLON ':'
[4:16] KW_I32 'i32'
[4:20] ASSIGN '='
[4:22] NUMBER '100'
...
[6:9]  IDENTIFIER 'letvar'     ← letvar 被正确识别为标识符
[6:15] COLON ':'
[6:17] KW_I32 'i32'
```

---

## 3. 语法分析器设计

### 3.1 设计原理

语法分析器采用**递归下降**（Recursive Descent）策略，为每条文法规则编写对应的解析方法。该方法自顶向下、从左到右扫描记号流，构建出完整的抽象语法树。

### 3.2 左递归消除

原始文法中的左递归产生式（如 `<表达式> -> <表达式> + <项>`）会导致递归下降解析器无限循环。本实现通过**迭代替代递归**的方式消除左递归：

**原始文法（左递归）**：
```
<表达式> -> <表达式> <比较运算符> <加减表达式>
```

**消除后（迭代实现）**：
```cpp
std::unique_ptr<Expression> Parser::parseExpression() {
    auto left = parseAddSubExpr();           // 先解析左操作数
    while (check(EQ) || check(NE) || ...) {  // 迭代处理后续运算符
        TokenType op = advance().type;
        auto right = parseAddSubExpr();
        left = std::make_unique<BinaryExpr>(move(left), op, move(right));
    }
    return left;
}
```

### 3.3 运算符优先级处理

通过解析方法的调用层次实现运算符优先级：

```
parseExpression()     ← 最低优先级：比较运算符 (==, !=, <, <=, >, >=)
  └── parseAddSubExpr()   ← 加减运算符 (+, -)
        └── parseTerm()       ← 乘除运算符 (*, /)
              └── parsePostfix()  ← 后缀表达式 (arr[i], tuple.0)
                    └── parsePrimary()  ← 最高优先级：字面量、标识符、括号
```

例如 `a + b * c == d` 的解析过程：

```
parseExpression() 处理 ==
├── parseAddSubExpr() 处理 +
│   ├── parseTerm(): a
│   └── parseTerm() 处理 *
│       ├── parsePrimary(): b
│       └── parsePrimary(): c
└── parseAddSubExpr(): d
```

最终 AST 结构为 `((a + (b * c)) == d)`，正确体现了先乘除、后加减、最后比较的优先级。

### 3.4 主要文法规则与实现对照

| 规则编号 | 产生式 | 实现方法 |
|----------|--------|----------|
| 1.1 | Program -> Declaration* | `parseProgram()`, `parseDeclarations()` |
| 1.4 | 形参列表 -> 形参 (',' 形参)* | `parseParameterList()` |
| 1.5 | 函数头 -> fn ID '(' 参数 ')' '->' 类型 | `parseFunctionDecl()` |
| 2.1 | let 变量声明 | `parseLetStmt()` |
| 2.2 | 左值 '=' 表达式 ';' | `parseAssignStmt()` |
| 3.2 | 表达式 -> 表达式 比较运算符 加减表达式 | `parseExpression()` |
| 3.3 | 加减表达式 -> 加减表达式 加减运算符 项 | `parseAddSubExpr()` |
| 3.4 | 项 -> 项 乘除运算符 因子 | `parseTerm()` |
| 3.5 | 函数调用 ID '(' 实参列表 ')' | `parsePrimary()` 中标识符分支 |
| 4.1 | if 语句 | `parseIfStmt()` |
| 5.1 | while 语句 | `parseWhileStmt()` |

### 3.5 错误处理

解析器在遇到不期望的记号时抛出带位置信息的错误：

```cpp
Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    std::stringstream ss;
    ss << "Error at line " << peek().line << ", column " << peek().column
       << ": " << message << " (got '" << peek().lexeme << "')";
    throw std::runtime_error(ss.str());
}
```

示例错误输出：
```
Error at line 3, column 5: Expected ';' after variable declaration (got 'return')
```

---

## 4. 抽象语法树设计

### 4.1 层次结构

AST 节点采用继承层次设计，所有节点继承自 `ASTNode` 基类：

```
ASTNode
├── Type
│   ├── I32Type
│   ├── RefType              (&T, &mut T)
│   ├── ArrayType            ([T; N])
│   └── TupleType            (T1, T2, ...)
├── Expression
│   ├── NumberLiteral
│   ├── Identifier
│   ├── BinaryExpr
│   ├── CallExpr
│   ├── UnaryExpr            (&expr, *expr)
│   ├── ArrayLiteral
│   ├── IndexExpr            arr[i]
│   ├── TupleLiteral
│   ├── TupleIndexExpr       tuple.0
│   ├── BlockExpr            { ... }
│   ├── IfExpr               if ... else ...
│   └── LoopExpr             loop { ... }
├── Statement
│   ├── EmptyStmt
│   ├── ExprStmt
│   ├── ReturnStmt
│   ├── LetStmt
│   ├── AssignStmt
│   ├── IfStmt
│   ├── WhileStmt
│   ├── ForStmt
│   ├── LoopStmt
│   ├── BreakStmt
│   └── ContinueStmt
├── Block                    (包含语句列表 + 可选的块尾表达式)
├── Declaration
│   └── FunctionDecl
└── Program
```

### 4.2 内存管理

AST 节点通过 `std::unique_ptr` 管理所有权，构建过程中使用 `std::move()` 转移所有权，避免拷贝开销。

### 4.3 输出格式

每个 AST 节点实现 `print(int indent)` 方法，以缩进形式输出树形结构。

---

## 5. 系统架构

### 5.1 总体架构

```
源代码文件
    │
    ▼
┌─────────────┐
│   Lexer     │  词法分析器
│             │  状态机扫描 + 最长匹配
│  token.h    │  33 种记号类型
│  lexer.h/cpp│
└──────┬──────┘
       │
       ▼
  Token Stream (记号流)
       │
       ▼
┌─────────────┐
│   Parser    │  语法分析器
│             │  递归下降 + 优先级层次
│  parser.h/cpp│
└──────┬──────┘
       │
       ▼
    AST (抽象语法树)
       │
       ▼
┌─────────────┐
│   AST       │  树形输出
│  ast.h/cpp  │
└─────────────┘
       │
       ▼
  格式化输出
```

### 5.2 模块说明

| 模块 | 文件 | 职责 |
|------|------|------|
| Token 定义 | `token.h` | TokenType 枚举、Token 结构体、关键字映射 |
| 词法分析 | `lexer.h/cpp` | 字符扫描、记号识别、注释跳过 |
| 语法分析 | `parser.h/cpp` | 递归下降解析、AST 构建、错误报告 |
| AST 定义 | `ast.h` | 所有 AST 节点类定义 |
| AST 实现 | `ast.cpp` | 各节点 print() 方法实现 |
| 主程序 | `main.cpp` | 文件读取、参数解析、管道编排 |

### 5.3 命令行接口

```
Usage: rustly-compiler <input-file> [options]
Options:
  --tokens-only    Only output token stream
  --ast            Output AST (default)
  --verbose        Verbose output
```

---

## 6. 程序实例与运行结果

### 6.1 基础程序

**输入文件** (`tests/test_basic.txt`)：
```rust
fn main() {
    return;
}
```

**运行命令**：`./rustly-compiler.exe tests/test_basic.txt`

**输出**：
```
=== Abstract Syntax Tree ===
fn main() {
  return;
}
```

### 6.2 表达式与运算符优先级

**输入文件** (`tests/test_expr.txt`)：
```rust
fn calculate(mut x: i32, mut y: i32) -> i32 {
    let mut result: i32;
    result = x + y * 2;
    return result;
}
```

**运行命令**：`./rustly-compiler.exe tests/test_expr.txt`

**输出**：
```
=== Abstract Syntax Tree ===
fn calculate(mut x: i32, mut y: i32) -> i32 {
  let mut result: i32;
  result = (x + (y * 2));
  return result;
}
```

**分析**：AST 中 `(x + (y * 2))` 的嵌套结构正确体现了 `y * 2` 优先于 `+` 的运算优先级。

**记号流**（`--tokens-only`）：
```
=== Token Stream ===
[1:1]  KW_FN 'fn'
[1:4]  IDENTIFIER 'calculate'
[1:13] LPAREN '('
[1:14] KW_MUT 'mut'
[1:18] IDENTIFIER 'x'
[1:19] COLON ':'
[1:21] KW_I32 'i32'
[1:24] COMMA ','
[1:26] KW_MUT 'mut'
[1:30] IDENTIFIER 'y'
[1:31] COLON ':'
[1:33] KW_I32 'i32'
[1:36] RPAREN ')'
[1:38] ARROW '->'
[1:41] KW_I32 'i32'
[1:45] LBRACE '{'
[2:5]  KW_LET 'let'
[2:9]  KW_MUT 'mut'
[2:13] IDENTIFIER 'result'
[2:19] COLON ':'
[2:21] KW_I32 'i32'
[2:24] SEMICOLON ';'
[3:5]  IDENTIFIER 'result'
[3:12] ASSIGN '='
[3:14] IDENTIFIER 'x'
[3:16] PLUS '+'
[3:18] IDENTIFIER 'y'
[3:20] STAR '*'
[3:22] NUMBER '2'
[3:23] SEMICOLON ';'
[4:5]  KW_RETURN 'return'
[4:12] IDENTIFIER 'result'
[4:18] SEMICOLON ';'
[5:1]  RBRACE '}'
[6:1]  END_OF_FILE
```

### 6.3 控制流：if 与 while

**输入文件** (`tests/test_control.txt`)：
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

**运行命令**：`./rustly-compiler.exe tests/test_control.txt`

**输出**：
```
=== Abstract Syntax Tree ===
fn max(mut a: i32, mut b: i32) -> i32 {
  if (a > b)   {
    return a;
  }
  return b;
}

fn sum(mut n: i32) -> i32 {
  let mut result: i32;
  result = 0;
  let mut i: i32;
  i = 1;
  while (i <= n)   {
    result = (result + i);
    i = (i + 1);
  }
  return result;
}
```

### 6.4 函数调用

**输入文件** (`tests/test_func.txt`)：
```rust
fn add(mut a: i32, mut b: i32) -> i32 {
    return a + b;
}

fn main() -> i32 {
    let mut x: i32;
    x = add(3, 5);
    return x;
}
```

**输出**：
```
=== Abstract Syntax Tree ===
fn add(mut a: i32, mut b: i32) -> i32 {
  return (a + b);
}

fn main() -> i32 {
  let mut x: i32;
  x = add(3, 5);
  return x;
}
```

### 6.5 else if 语句

**输入文件** (`tests/test_else_if.txt`)：
```rust
fn main() -> i32 {
    let x: i32 = 5;
    let result: i32 = 0;

    if x == 1 {
        result = 10;
    } else if x == 2 {
        result = 20;
    } else if x == 5 {
        result = 50;
    } else {
        result = 100;
    }

    return result;
}
```

**输出**：
```
=== Abstract Syntax Tree ===
fn main() -> i32 {
  let x: i32 = 5;
  let result: i32 = 0;
  if (x == 1)   {
    result = 10;
  }
  else   {
    if (x == 2)     {
      result = 20;
    }
    else     {
      if (x == 5)       {
        result = 50;
      }
      else       {
        result = 100;
      }
    }
  }
  return result;
}
```

**分析**：`else if` 被转换为嵌套的 `if-else` 结构，符合文法 4.3 的实现策略。

### 6.6 for 循环

**输入文件** (`tests/test_for_loop.txt`)：
```rust
fn main() -> i32 {
    let mut sum: i32 = 0;
    for i in 0..10 {
        sum = sum + i;
    }
    for mut j in 1..5 {
        j = j + 1;
    }
    return sum;
}
```

**输出**：
```
=== Abstract Syntax Tree ===
fn main() -> i32 {
  let mut sum: i32 = 0;
  for i in 0..10   {
    sum = (sum + i);
  }
  for mut j in 1..5   {
    j = (j + 1);
  }
  return sum;
}
```

### 6.7 loop 循环与 break/continue

**输入文件** (`tests/test_loop.txt`)：
```rust
fn main() -> i32 {
    let mut count: i32 = 0;
    loop {
        count = count + 1;
        if count == 5 {
            continue;
        }
        if count == 10 {
            break;
        }
    }
    return count;
}
```

**输出**：
```
=== Abstract Syntax Tree ===
fn main() -> i32 {
  let mut count: i32 = 0;
  loop   {
    count = (count + 1);
    if (count == 5)     {
      continue;
    }
    if (count == 10)     {
      break;
    }
  }
  return count;
}
```

### 6.8 引用与解引用

**输入文件** (`tests/test_reference.txt`)：
```rust
fn test_ref(x: &i32, y: &mut i32) -> i32 {
    let a: i32 = *x;
    let b: i32 = *y;
    return a + b;
}

fn main() -> i32 {
    let value: i32 = 10;
    let mut mutable: i32 = 20;
    let ref1: &i32 = &value;
    let ref2: &mut i32 = &mut mutable;
    let result: i32 = *ref1 + *ref2;
    return test_ref(&value, &mut mutable);
}
```

**输出**：
```
=== Abstract Syntax Tree ===
fn test_ref(x: &i32, y: &mut i32) -> i32 {
  let a: i32 = *x;
  let b: i32 = *y;
  return (a + b);
}

fn main() -> i32 {
  let value: i32 = 10;
  let mut mutable: i32 = 20;
  let ref1: &i32 = &value;
  let ref2: &mut i32 = &mut mutable;
  let result: i32 = (*ref1 + *ref2);
  return test_ref(&value, &mut mutable);
}
```

### 6.9 数组类型与操作

**输入文件** (`tests/test_array.txt`)：
```rust
fn sum_array(arr: &[i32; 5]) -> i32 {
    result = *arr[0] + *arr[1] + *arr[2] + *arr[3] + *arr[4];
    return result;
}

fn main() -> i32 {
    let numbers: [i32; 5] = [1, 2, 3, 4, 5];
    let values: [i32; 3] = [10, 20, 30];
    let first: i32 = numbers[0];
    let second: i32 = values[1];
    let sum: i32 = first + second;
    return sum_array(&numbers);
}
```

**输出**：
```
=== Abstract Syntax Tree ===
fn sum_array(arr: &[i32; 5]) -> i32 {
  let result: i32 = 0;
  result = ((((*arr[0] + *arr[1]) + *arr[2]) + *arr[3]) + *arr[4]);
  return result;
}

fn main() -> i32 {
  let numbers: [i32; 5] = [1, 2, 3, 4, 5];
  let values: [i32; 3] = [10, 20, 30];
  let first: i32 = numbers[0];
  let second: i32 = values[1];
  let sum: i32 = (first + second);
  return sum_array(&numbers);
}
```

### 6.10 元组类型与操作

**输入文件** (`tests/test_tuple.txt`)：
```rust
fn swap(pair: (i32, i32)) -> (i32, i32) {
    let a: i32 = pair.0;
    let b: i32 = pair.1;
    return (b, a);
}

fn main() -> i32 {
    let point: (i32, i32) = (10, 20);
    let triple: (i32, i32, i32) = (1, 2, 3);
    let x: i32 = point.0;
    let y: i32 = point.1;
    let sum: i32 = x + y;
    let swapped: (i32, i32) = swap(point);
    let result: i32 = swapped.0 + swapped.1;
    return result;
}
```

**输出**：
```
=== Abstract Syntax Tree ===
fn swap(pair: (i32, i32)) -> (i32, i32) {
  let a: i32 = pair.0;
  let b: i32 = pair.1;
  return (b, a);
}

fn main() -> i32 {
  let point: (i32, i32) = (10, 20);
  let triple: (i32, i32, i32) = (1, 2, 3);
  let x: i32 = point.0;
  let y: i32 = point.1;
  let sum: i32 = (x + y);
  let swapped: (i32, i32) = swap(point);
  let result: i32 = (swapped.0 + swapped.1);
  return result;
}
```

### 6.11 综合测试

**输入文件** (`tests/test_comprehensive.txt`) 同时测试了：
- 6.1 变量不可变属性（省略 `mut`）
- 4.3 else if 语句
- 5.3 loop 循环
- 5.4 break/continue 语句
- 5.2 for 循环
- 函数调用

**输出**：
```
=== Abstract Syntax Tree ===
fn calculate(mut x: i32, y: i32) -> i32 {
  let result: i32 = 0;
  for i in 0..5   {
    result = (result + i);
  }
  return ((result + x) + y);
}

fn main() -> i32 {
  let immutable: i32 = 10;
  let mut mutable: i32 = 20;
  if (immutable == 5)   {
    mutable = 100;
  }
  else   {
    if (immutable == 10)     {
      mutable = 200;
    }
    else     {
      mutable = 300;
    }
  }
  let mut counter: i32 = 0;
  loop   {
    counter = (counter + 1);
    if (counter == 3)     {
      continue;
    }
    if (counter == 7)     {
      break;
    }
  }
  for j in 1..4   {
    mutable = (mutable + j);
  }
  return calculate(mutable, counter);
}
```

---

## 7. 编译与运行说明

### 7.1 构建步骤

```bash
# 编译项目
make

# 清理构建产物
make clean

# 运行全部测试
make test
```

### 7.2 使用方式

```bash
# 基本用法：输出 AST
./rustly-compiler.exe <input-file>

# 仅输出记号流
./rustly-compiler.exe <input-file> --tokens-only

# 详细输出（同时显示记号流和 AST）
./rustly-compiler.exe <input-file> --verbose
```

### 7.3 截屏说明

**关于图片链接：**

Markdown 文档**支持**通过相对路径链接图片，语法为：

```markdown
![截屏描述](screenshots/filename.png)
```

但由于本项目的实际运行结果输出为纯文本，**无需截屏即可完整展示**。以上所有程序实例的运行结果均已以代码块形式呈现。

如需为 PPT 制作截屏，请按照以下步骤操作：

1. **在终端中运行命令**（以 test_basic.txt 为例）：
   ```bash
   cd "D:\private\2 Assignments\1 fundamental of compiling\course project\Rustly-Compiler"
   ./rustly-compiler.exe tests/test_basic.txt
   ```

2. **截取终端窗口**：
   - Windows: 按 `Win + Shift + S`，框选终端输出区域
   - 或使用截图工具（Snipping Tool）

3. **保存图片**到 `screenshots/` 目录：
   ```
   screenshots/
   ├── test_basic_output.png
   ├── test_expr_output.png
   ├── test_control_output.png
   ├── test_func_output.png
   ├── test_tokens_output.png
   └── test_comprehensive_output.png
   ```

4. **在文档中引用**：
   ```markdown
   ![基础程序运行结果](screenshots/test_basic_output.png)
   ```

以下是建议截屏的关键场景：

| 截屏文件 | 对应命令 | 说明 |
|----------|----------|------|
| `test_basic_output.png` | `./rustly-compiler.exe tests/test_basic.txt` | 最简程序，展示基础 AST |
| `test_expr_output.png` | `./rustly-compiler.exe tests/test_expr.txt` | 表达式优先级验证 |
| `test_control_output.png` | `./rustly-compiler.exe tests/test_control.txt` | if/while 控制流 |
| `test_tokens_output.png` | `./rustly-compiler.exe tests/test_expr.txt --tokens-only` | 记号流输出 |
| `test_comprehensive_output.png` | `./rustly-compiler.exe tests/test_comprehensive.txt` | 综合功能展示 |
| `build_success.png` | `make` 编译输出 | 构建成功 |

---

## 8. 选做功能实现总结

### 8.1 已实现的选做功能

| 规则 | 功能 | 实现方式 |
|------|------|----------|
| 4.2 | else 分支 | `parseIfStmt()` 中匹配 `KW_ELSE` 后解析 Block |
| 4.3 | else if | `parseIfStmt()` 中 else 后匹配 `KW_IF` 时递归解析 |
| 5.2 | for 循环 | `parseForStmt()` 解析 `for var in start..end { body }` |
| 5.3 | loop 循环 | `parseLoopStmt()` 解析 `loop { body }` |
| 5.4 | break/continue | `parseStatement()` 中匹配关键字，break 支持带表达式 |
| 6.1 | 不可变属性省略 | `parseLetStmt()` 中 `mut` 为可选匹配 |
| 6.2 | 不可变引用 `&T` | `parseType()` 和 `parsePrimary()` 中处理 `&` |
| 6.3 | 可变引用 `&mut T` | `parseType()` 中匹配 `&` 后尝试匹配 `mut` |
| 6.4 | 解引用 `*expr` | `parsePrimary()` 中匹配 `STAR` 作为一元运算符 |
| 7.0 | 块尾表达式 | `parseBlock()` 中 `}` 前尝试解析表达式作为 `tailExpr` |
| 7.1 | 块表达式 | `parsePrimary()` 中匹配 `LBRACE` 后调用 `parseBlock()` |
| 7.3 | if 表达式 | `parsePrimary()` 中处理 `if ... else ...` 必须含 else |
| 7.4 | loop 表达式 | `parsePrimary()` 中处理 `loop { ... }` |
| 8.1 | 数组类型 `[T; N]` | `parseType()` 中解析 `[` 开头的类型 |
| 8.2 | 数组字面量 | `parsePrimary()` 中解析 `[expr, ...]` |
| 8.3 | 数组索引 | `parsePostfix()` 中处理 `[expr]` 后缀 |
| 9.1 | 元组类型 | `parseType()` 中解析 `(T1, T2, ...)` |
| 9.2 | 元组字面量 | `parsePrimary()` 中解析 `(expr1, expr2, ...)` |
| 9.3 | 元组索引 | `parsePostfix()` 中处理 `.N` 后缀 |

---

## 9. 个人思考与总结

### 9.1 最长匹配原则的重要性

在词法分析阶段，最长匹配原则是确保正确性的关键。例如 `==` 和 `=` 的区分：如果采用单字符匹配，`==` 会被错误地分解为两个 `=` 记号。本实现通过向前查看（lookahead）一个字符，优先尝试匹配更长的记号，有效避免了这一问题。

### 9.2 递归下降与 LR 分析的比较

本项目选择的递归下降方法具有以下优点：
- 代码可读性强，每个方法对应一条文法规则
- 错误报告精准，可以携带行号和列号信息
- 易于调试和扩展

但递归下降也有局限性：
- 需要手动消除左递归
- 无法自动处理回溯（如赋值语句与表达式语句的区分通过前瞻判断解决）
- 对于复杂的歧义文法，需要额外的消歧义逻辑

### 9.3 运算符优先级的优雅解法

通过为不同优先级的运算符分配独立的解析方法（`parseExpression` > `parseAddSubExpr` > `parseTerm` > `parsePostfix` > `parsePrimary`），自然地实现了运算优先级。这种方法避免了显式的优先级表，代码结构清晰且易于扩展。

### 9.4 对未来工作的展望

- **语义分析**：当前仅做词法和语法分析，未来可增加类型检查、作用域分析等语义层
- **代码生成**：输出 LLVM IR 或 WebAssembly 等中间代码，实现完整的编译流程
- **错误恢复**：当前遇到语法错误即终止，可实现 panic-mode 或 phrase-level 的错误恢复策略
- **更丰富的类型系统**：支持更多基本类型（f64, bool, String 等）和复合类型

---

## 10. 参考资料

1. Aho, A. V., Lam, M. S., Sethi, R., & Ullman, J. D. (2007). *Compilers: Principles, Techniques, and Tools* (2nd ed.). Pearson Education. （龙书）
2. Nystrom, R. (2021). *Crafting Interpreters*. https://craftinginterpreters.com/
3. Rust Programming Language. https://doc.rust-lang.org/book/
4. 陈火旺等. (2008). 《编译原理》（第 3 版）. 国防工业出版社.
5. Levine, J. R. (2009). *flex & bison*. O'Reilly Media.
