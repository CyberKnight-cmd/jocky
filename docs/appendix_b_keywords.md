# Appendix B: Keywords & Operator Table

> *"This appendix serves as an exhaustive, tabular reference for all JOCKY keywords, reserved tokens, operators, and compiler annotations."*

---

## Table of Contents
1. [Keywords Directory](#keywords-directory)
2. [Complete Operator Precedence & Associativity Table](#complete-operator-precedence--associativity-table)
3. [Syntactic Punctuation & Delimiters](#syntactic-punctuation--delimiters)
4. [Compiler Annotations Reference](#compiler-annotations-reference)

---

## 1. Keywords Directory

| Keyword | Category | Semantics & Description | Example Usage |
| :--- | :--- | :--- | :--- |
| **`fn`** | Declaration | Declares a named function or method receiver | `fn add(int a, int b) -> int { return a + b; }` |
| **`struct`** | Declaration | Defines a composite data structure | `struct Process { int pid; string name; }` |
| **`import`** | Module | Imports a built-in or user package namespace | `import process;` |
| **`const`** | Declaration | Declares a compile-time immutable constant | `const int MAX_SIZE = 1024;` |
| **`auto`** | Type Inference | Infers static type from the initializing expression | `auto procs = process.list();` |
| **`if`** | Control Flow | Conditional branch execution (no parens required) | `if count > 0 { log.info("Active"); }` |
| **`else`** | Control Flow | Alternate conditional branch | `if ok { ... } else { ... }` |
| **`while`** | Control Flow | Repeats block while condition evaluates to true | `while is_running { poll(); }` |
| **`for`** | Control Flow | Indexed loop or collection iterator (`for..in`) | `for p in procs { log.info(p.name); }` |
| **`in`** | Control Flow | Collection traversal separator in `for..in` loops | `for item in collection { ... }` |
| **`return`** | Control Flow | Returns value(s) from a function to caller | `return result, nil;` |
| **`break`** | Control Flow | Terminates innermost enclosing loop | `if found { break; }` |
| **`continue`** | Control Flow | Advances to next iteration of enclosing loop | `if skip { continue; }` |
| **`true`** | Literal | Boolean truth value | `bool is_valid = true;` |
| **`false`** | Literal | Boolean falsehood value | `bool is_valid = false;` |
| **`nil`** | Literal | Represents unassigned reference / absence of error | `Error err = nil;` |
| **`void`** | Type Specifier | Represents empty/absent return value | `fn main() -> void { ... }` |
| **`int`** | Type Specifier | 64-bit signed two's complement integer (`i64`) | `int pid = 1042;` |
| **`float`** | Type Specifier | 64-bit IEEE 754 floating-point number (`f64`) | `float score = 98.6;` |
| **`bool`** | Type Specifier | Boolean logical type (`true` or `false`) | `bool is_elevated = true;` |
| **`byte`** | Type Specifier | 8-bit unsigned octet (`u8`, 0..255) | `byte magic = 'M';` |
| **`bytes`** | Type Specifier | Heap-allocated mutable binary blob buffer | `bytes blob = x"4D5A9000";` |
| **`string`** | Type Specifier | Immutable UTF-8 encoded text sequence | `string case_id = "CASE-01";` |
| **`list`** | Type Specifier | Dynamically growing untyped array vector | `list pids = [1024, 2048];` |
| **`map`** | Type Specifier | Associative hash table key-value store | `map meta = {"host": "DC01"};` |
| **`Error`** | Type Specifier | Diagnostic error struct (`code`, `message`) | `fn open() -> (CaseFile, Error)` |
| **`_`** | Special Token | Blank identifier for write-only value discard | `auto _, err = call();` |

---

## 2. Complete Operator Precedence & Associativity Table

| Precedence | Operator | Description | Associativity | Example |
| :---: | :--- | :--- | :---: | :--- |
| **1 (Highest)**| `()` | Function / Method Call | Left-to-Right | `process.list()` |
| | `[]` | Array / Map Indexing | Left-to-Right | `items[0]`, `meta["ip"]` |
| | `.` | Member / Field Access | Left-to-Right | `proc.name` |
| | `!` | Error Try / Propagate | Left-to-Right | `evidence.open(id)!` |
| | `++` | Postfix Increment | Left-to-Right | `counter++` |
| | `--` | Postfix Decrement | Left-to-Right | `counter--` |
| **2** | `+` | Unary Plus | Right-to-Left | `+val` |
| | `-` | Unary Negation | Right-to-Left | `-val` |
| | `!` | Logical NOT | Right-to-Left | `!is_ready` |
| | `~` | Bitwise NOT | Right-to-Left | `~mask` |
| | `(type)` | Explicit Type Cast | Right-to-Left | `(string)pid`, `(float)count` |
| **3** | `*` | Multiplication | Left-to-Right | `a * b` |
| | `/` | Division | Left-to-Right | `a / b` |
| | `%` | Modulo Remainder | Left-to-Right | `a % b` |
| **4** | `+` | Addition / String Concat | Left-to-Right | `a + b`, `"A" + "B"` |
| | `-` | Subtraction | Left-to-Right | `a - b` |
| **5** | `<<` | Bitwise Shift Left | Left-to-Right | `val << 4` |
| | `>>` | Bitwise Shift Right | Left-to-Right | `val >> 2` |
| **6** | `<` | Less Than | Left-to-Right | `a < b` |
| | `<=` | Less Than or Equal | Left-to-Right | `a <= b` |
| | `>` | Greater Than | Left-to-Right | `a > b` |
| | `>=` | Greater Than or Equal | Left-to-Right | `a >= b` |
| **7** | `==` | Value Equality | Left-to-Right | `a == b` |
| | `!=` | Value Inequality | Left-to-Right | `err != nil` |
| **8** | `&` | Bitwise AND | Left-to-Right | `flags & 0xFF` |
| **9** | `^` | Bitwise XOR | Left-to-Right | `val ^ key` |
| **10** | `\|` | Bitwise OR | Left-to-Right | `flags \| 0x01` |
| **11** | `&&` | Short-Circuit Logical AND| Left-to-Right | `is_admin && has_token` |
| **12** | `\|\|` | Short-Circuit Logical OR | Left-to-Right | `is_root \|\| is_admin` |
| **13** | `? :` | Ternary Conditional | Right-to-Left | `cond ? val1 : val2` |
| **14 (Lowest)**| `=` | Assignment | Right-to-Left | `x = 10` |
| | `+=`, `-=` | Compound Arithmetic | Right-to-Left | `x += 5`, `x -= 2` |
| | `*=`, `/=`, `%=`| Compound Arithmetic | Right-to-Left | `x *= 2`, `x /= 4` |
| | `&=`, `\|=`, `^=`| Compound Bitwise | Right-to-Left | `flags &= 0xF0` |

---

## 3. Syntactic Punctuation & Delimiters

| Symbol | Name | Syntactic Purpose |
| :--- | :--- | :--- |
| `;` | Semicolon | Mandatory statement and field terminator |
| `,` | Comma | Parameter, element, and argument separator |
| `{` `}` | Curly Braces | Lexical block, struct definition, and map literal delimiter |
| `(` `)` | Parentheses | Function parameter list and expression grouping delimiter |
| `[` `]` | Square Brackets | List literal and index access delimiter |
| `->` | Arrow | Return type specifier delimiter in function signatures |
| `:` | Colon | Key-value separator in maps and struct field initializers |
| `@` | At Symbol | Compiler annotation prefix |

---

## 4. Compiler Annotations Reference

| Annotation | Valid Targets | Semantic Effect |
| :--- | :--- | :--- |
| **`@privileged`** | Functions, Methods | Injects OS token assertion preamble (`SeDebugPrivilege` / `root`) |
| **`@platform("os")`**| Functions, Structs | Conditionally compiles or generates stub for target OS |
| **`@deprecated("msg")`**| Functions, Structs | Emits compile-time diagnostic warning at all reference sites |
| **`@inline`** | Functions, Methods | Inlines function AST directly at call sites to dissolve call graphs |
| **`@noreturn`** | Functions | Directs CFG analyzer that function never returns to caller |
