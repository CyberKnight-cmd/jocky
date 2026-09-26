# Chapter 21: Compiler Diagnostics & Error Codes

> *"A helpful compiler does not simply state that something is broken; it explains precisely why the code violates type invariants and demonstrates the exact remedy."*

---

## Table of Contents
1. [Diagnostic Architecture & Error Formatting](#diagnostic-architecture--error-formatting)
2. [Diagnostic Codes Directory (E0001 – E0015)](#diagnostic-codes-directory-e0001--e0015)
   - [E0001: Redefinition of Symbol](#e0001-redefinition-of-symbol)
   - [E0002: Syntax Error / Missing Semicolon](#e0002-syntax-error--missing-semicolon)
   - [E0003: Circular Import Dependency](#e0003-circular-import-dependency)
   - [E0004: Reserved Identifier / Leading Underscore](#e0004-reserved-identifier--leading-underscore)
   - [E0005: Ambiguous Mixed Bitwise/Comparison Expression](#e0005-ambiguous-mixed-bitwisecomparison-expression)
   - [E0006: Assignment of Nil to Primitive Type](#e0006-assignment-of-nil-to-primitive-type)
   - [E0007: Binary Type Mismatch / Implicit Conversion Prohibited](#e0007-binary-type-mismatch--implicit-conversion-prohibited)
   - [E0008: Assignment to Constant Variable](#e0008-assignment-to-constant-variable)
   - [E0009: Uninitialized Variable Declaration](#e0009-uninitialized-variable-declaration)
   - [E0010: Undefined Identifier in Scope](#e0010-undefined-identifier-in-scope)
   - [E0011: Global Mutable Variable in Agent Mode](#e0011-global-mutable-variable-in-agent-mode)
   - [E0012: Missing Return Statement in Non-Void Function](#e0012-missing-return-statement-in-non-void-function)
   - [E0013: Mismatched Types in Ternary Expression](#e0013-mismatched-types-in-ternary-expression)
   - [E0014: Invalid Use of `!` in Non-Error-Returning Function](#e0014-invalid-use-of--in-non-error-returning-function)
   - [E0015: Unresolved Module Import](#e0015-unresolved-module-import)
3. [Master Diagnostic Reference Table](#master-diagnostic-reference-table)
4. [Chapter Summary](#chapter-summary)

---

## 1. Diagnostic Architecture & Error Formatting

When the JOCKY compiler detects a lexical, syntactic, or semantic violation, it emits a standardized diagnostic message containing:
1. **Source File Path & Line/Column Coordinates**
2. **Diagnostic Severity** (`[ERROR]` or `[WARN]`)
3. **Diagnostic Code** (`E0001` through `E0015`)
4. **Source Code Context Window** with underline pointer (`^`)
5. **Remediation Hint**

```
[ERROR] E0007: Binary operator '+' cannot be applied to types 'string' and 'int'.
  --> triage.jky:14:26
   |
14 |     string msg = "PID: " + p.pid;
   |                          ^
   |
   = hint: JOCKY strictly forbids implicit type conversions.
   = fix:  Use an explicit type cast: "PID: " + (string)p.pid;
```

---

## 2. Diagnostic Codes Directory (E0001 – E0015)

---

### E0001: Redefinition of Symbol
- **Category:** Semantic Analysis (Pass 1)
- **Description:** Occurs when an identifier is declared more than once within the same lexical scope or package namespace.
- **Problematic Code:**
```jocky
int timeout = 5000;
string timeout = "5s"; // Error: 'timeout' already declared
```
- **Error Message:** `E0001: Redefinition of symbol 'timeout' in the same scope.`
- **Fix:** Rename the second variable or reassign without redeclaring:
```jocky
int timeout = 5000;
string timeout_label = "5s";
```

---

### E0002: Syntax Error / Missing Semicolon
- **Category:** Lexical & Parser Syntax
- **Description:** A statement or declaration does not conform to JOCKY grammar, or omits a mandatory semicolon.
- **Problematic Code:**
```jocky
int count = 10
auto info = host.info();
```
- **Error Message:** `E0002: Syntax error: Expected ';' at end of declaration.`
- **Fix:** Terminate all statements with semicolons:
```jocky
int count = 10;
auto info = host.info();
```

---

### E0003: Circular Import Dependency
- **Category:** Module Resolution
- **Description:** Two or more packages form an import cycle, preventing deterministic DAG topological sorting.
- **Problematic Code:** `package A` imports `package B`, while `package B` imports `package A`.
- **Error Message:** `E0003: Circular import dependency detected: package A -> package B -> package A.`
- **Fix:** Move shared types or functions to a separate leaf package (e.g. `models/`).

---

### E0004: Reserved Identifier / Leading Underscore
- **Category:** Lexer & Semantic Analysis
- **Description:** User code attempts to declare an identifier with a leading underscore (reserved for compiler internals).
- **Problematic Code:**
```jocky
int _temp_counter = 0;
```
- **Error Message:** `E0004: Identifier '_temp_counter' is reserved. User identifiers cannot start with '_' (except lone '_').`
- **Fix:** Remove the leading underscore:
```jocky
int temp_counter = 0;
```

---

### E0005: Ambiguous Mixed Bitwise/Comparison Expression
- **Category:** Parser & Semantic Analysis
- **Description:** Mixing bitwise/shift operators (`&`, `|`, `^`, `<<`, `>>`) with relational (`<`, `==`) or additive operators without explicit parentheses.
- **Problematic Code:**
```jocky
if flags & 0x01 == 0x01 { ... }
```
- **Error Message:** `E0005: Ambiguous mixed bitwise and comparison expression. Explicit parentheses are mandatory.`
- **Fix:** Enclose the bitwise operation in parentheses:
```jocky
if (flags & 0x01) == 0x01 { ... }
```

---

### E0006: Assignment of Nil to Primitive Type
- **Category:** Type Checker
- **Description:** Attempting to assign `nil` to a primitive stack value type (`int`, `float`, `byte`, `bool`).
- **Problematic Code:**
```jocky
int pid = nil;
```
- **Error Message:** `E0006: Cannot assign 'nil' to primitive value type 'int'. Only reference types may be nil.`
- **Fix:** Initialize primitive types with valid values (`0`, `0.0`, `false`, `0x00`):
```jocky
int pid = 0;
```

---

### E0007: Binary Type Mismatch / Implicit Conversion Prohibited
- **Category:** Type Checker
- **Description:** Binary operators applied to differing operand types without explicit casting.
- **Problematic Code:**
```jocky
int a = 10;
float b = 2.5;
auto c = a + b;
```
- **Error Message:** `E0007: Binary operator '+' cannot be applied to 'int' and 'float'. Implicit conversions are forbidden.`
- **Fix:** Explicitly cast one operand:
```jocky
auto c = (float)a + b;
```

---

### E0008: Assignment to Constant Variable
- **Category:** Semantic Analysis
- **Description:** Attempting to reassign a value to an identifier declared with `const`.
- **Problematic Code:**
```jocky
const int MAX = 100;
MAX = 200;
```
- **Error Message:** `E0008: Cannot assign to constant variable 'MAX'.`
- **Fix:** Use a standard mutable variable declaration if mutation is intended:
```jocky
int max_limit = 100;
max_limit = 200;
```

---

### E0009: Uninitialized Variable Declaration
- **Category:** Parser Syntax
- **Description:** Declaring a variable without providing an initial assignment value.
- **Problematic Code:**
```jocky
string case_id;
```
- **Error Message:** `E0009: Variable 'case_id' declared without initializer. Uninitialized variables are prohibited.`
- **Fix:** Provide an explicit initial value:
```jocky
string case_id = "";
```

---

### E0010: Undefined Identifier in Scope
- **Category:** Semantic Symbol Resolver
- **Description:** Referencing a variable, function, or struct that has not been declared or is out of lexical scope.
- **Problematic Code:**
```jocky
fn test() -> void {
    log.info(target_host); // 'target_host' not declared
}
```
- **Error Message:** `E0010: Undefined identifier 'target_host' in current lexical scope.`
- **Fix:** Declare the variable in the current or outer scope before referencing.

---

### E0011: Global Mutable Variable in Agent Mode
- **Category:** Semantic Analysis (Agent Mode)
- **Description:** Declaring a top-level mutable variable outside of functions in Agent Mode.
- **Problematic Code:**
```jocky
int active_scans = 0; // Top-level mutable variable
fn main() -> void { ... }
```
- **Error Message:** `E0011: Global mutable variables are prohibited in Agent Mode. Use 'const' or local variables.`
- **Fix:** Declare as `const` or encapsulate inside a struct/function:
```jocky
const int INITIAL_SCANS = 0;
```

---

### E0012: Missing Return Statement in Non-Void Function
- **Category:** Control Flow Analyzer
- **Description:** A non-void function contains code paths that exit without a `return` statement.
- **Problematic Code:**
```jocky
fn get_score(bool valid) -> int {
    if valid { return 100; }
    // Missing return on false branch!
}
```
- **Error Message:** `E0012: Function declared to return 'int' has paths exiting without a return value.`
- **Fix:** Provide a return statement covering all execution branches:
```jocky
fn get_score(bool valid) -> int {
    if valid { return 100; }
    return 0;
}
```

---

### E0013: Mismatched Types in Ternary Expression
- **Category:** Type Checker
- **Description:** The true and false branches of a ternary conditional expression evaluate to differing static types.
- **Problematic Code:**
```jocky
auto res = is_ok ? "SUCCESS" : 500;
```
- **Error Message:** `E0013: Ternary operator branches have conflicting types: 'string' and 'int'.`
- **Fix:** Ensure both branches evaluate to identical types:
```jocky
auto res = is_ok ? "SUCCESS" : "ERROR_500";
```

---

### E0014: Invalid Use of `!` in Non-Error-Returning Function
- **Category:** Semantic Analysis
- **Description:** Using the `!` error propagation operator inside a function whose return type signature does not include `Error`.
- **Problematic Code:**
```jocky
fn main() -> void {
    auto cf = evidence.open("CASE-01")!;
}
```
- **Error Message:** `E0014: Cannot use '!' operator in function returning 'void'. Enclosing function must return 'Error'.`
- **Fix:** Handle the error explicitly using tuple assignment:
```jocky
fn main() -> void {
    auto cf, err = evidence.open("CASE-01");
    if err != nil { return; }
}
```

---

### E0015: Unresolved Module Import
- **Category:** Module Resolution
- **Description:** An `import` statement specifies a package name that cannot be located in the built-in standard library or project directories.
- **Problematic Code:**
```jocky
import nonexistent_module;
```
- **Error Message:** `E0015: Cannot resolve module 'nonexistent_module'. Package not found.`
- **Fix:** Verify package name spelling and ensure the target directory contains `.jky` files.

---

## 3. Master Diagnostic Reference Table

| Code | Diagnostic Name | Stage | Fatal? |
| :--- | :--- | :--- | :---: |
| **E0001** | Redefinition of Symbol | Sema Pass 1 | Yes |
| **E0002** | Syntax Error / Missing Semicolon | Parser | Yes |
| **E0003** | Circular Import Dependency | Module DAG | Yes |
| **E0004** | Reserved Identifier / Leading Underscore | Lexer / Sema | Yes |
| **E0005** | Ambiguous Mixed Bitwise Expression | Parser | Yes |
| **E0006** | Assignment of Nil to Primitive Type | Type Check | Yes |
| **E0007** | Binary Type Mismatch / Implicit Conversion | Type Check | Yes |
| **E0008** | Assignment to Constant Variable | Sema Pass 2 | Yes |
| **E0009** | Uninitialized Variable Declaration | Parser | Yes |
| **E0010** | Undefined Identifier in Scope | Symbol Resolve| Yes |
| **E0011** | Global Mutable Variable in Agent Mode | Sema Pass 1 | Yes |
| **E0012** | Missing Return Statement | Flow Analysis | Yes |
| **E0013** | Mismatched Types in Ternary Expression | Type Check | Yes |
| **E0014** | Invalid Use of `!` Operator | Sema Pass 2 | Yes |
| **E0015** | Unresolved Module Import | Module Resolve| Yes |

---

## 4. Chapter Summary

- **Total Diagnostic Spectrum:** 15 standardized compiler error codes (`E0001`–`E0015`).
- **Defensive Design:** Errors enforce explicit type casting, uninitialized variable prevention, and operator disambiguation.
- **Fast Feedback:** `jky check` runs semantic validation in milliseconds.

In the next chapter, **[Chapter 22: Future Architecture & Technical Roadmap](ch22_roadmap.md)**, we examine the evolution of JOCKY from v0.1 to v1.0, including direct LLVM IR emission, kernel drivers, and agent beacon networks.
