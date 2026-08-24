# Chapter 13: Operators & Expressions

> *"The most insidious bugs in systems programming do not stem from complex algorithms, but from subtle operator precedence misunderstandings. In JOCKY, operator evaluation is unambiguous, and risky mixed bitwise expressions are rejected at compile time."*

---

## Table of Contents
1. [Operator Hierarchy & Precedence Table](#operator-hierarchy--precedence-table)
2. [Arithmetic Operators](#arithmetic-operators)
3. [Relational & Equality Operators](#relational--equality-operators)
4. [Logical Operators](#logical-operators)
5. [Bitwise & Shift Operators](#bitwise--shift-operators)
6. [The Strict Bitwise/Comparison Ambiguity Prohibition](#the-strict-bitwisecomparison-ambiguity-prohibition)
7. [Assignment & Compound Operators](#assignment--compound-operators)
8. [Explicit Cast Expressions](#explicit-cast-expressions)
9. [The Ternary Conditional Operator](#the-ternary-conditional-operator)
10. [Complex Expression Evaluation & Practical Examples](#complex-expression-evaluation--practical-examples)
11. [Chapter Summary](#chapter-summary)

---

## 1. Operator Hierarchy & Precedence Table

The following table summarizes all operators supported in JOCKY, listed from highest precedence (binds tightest) to lowest precedence:

| Level | Category | Operator | Description | Associativity |
| :---: | :--- | :--- | :--- | :---: |
| **1** | Primary / Postfix | `()`, `[]`, `.`, `!`, `++`, `--` | Grouping, indexing, member access, error try, postfix inc/dec | Left-to-Right |
| **2** | Unary Prefix | `+`, `-`, `!`, `~`, `(type)` | Unary plus/minus, logical NOT, bitwise NOT, type cast | Right-to-Left |
| **3** | Multiplicative | `*`, `/`, `%` | Multiplication, division, modulo | Left-to-Right |
| **4** | Additive | `+`, `-` | Addition (or string concat), subtraction | Left-to-Right |
| **5** | Bitwise Shifts | `<<`, `>>` | Bitwise shift left, bitwise shift right (arithmetic/logical) | Left-to-Right |
| **6** | Relational | `<`, `<=`, `>`, `>=` | Less than, less-or-equal, greater than, greater-or-equal | Left-to-Right |
| **7** | Equality | `==`, `!=` | Value equality, value inequality | Left-to-Right |
| **8** | Bitwise AND | `&` | Bitwise AND | Left-to-Right |
| **9** | Bitwise XOR | `^` | Bitwise XOR | Left-to-Right |
| **10**| Bitwise OR | `\|` | Bitwise OR | Left-to-Right |
| **11**| Logical AND | `&&` | Short-circuit logical AND | Left-to-Right |
| **12**| Logical OR | `\|\|` | Short-circuit logical OR | Left-to-Right |
| **13**| Conditional | `? :` | Ternary conditional operator | Right-to-Left |
| **14**| Assignment | `=`, `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `^=`, `\|=` | Assignment and compound arithmetic/bitwise assignments | Right-to-Left |

---

## 2. Arithmetic Operators

JOCKY provides standard arithmetic operations for `int` and `float`:

```jocky
int a = 20;
int b = 6;

int sum  = a + b;  // 26
int diff = a - b;  // 14
int prod = a * b;  // 120
int quot = a / b;  // 3 (integer division truncates towards zero)
int rem  = a % b;  // 2 (modulo remainder)
```

### String Concatenation (`+`)
The `+` operator is overloaded for the `string` type to perform string concatenation:

```jocky
string prefix = "CASE-";
string suffix = "2026";
string case_id = prefix + suffix; // "CASE-2026"
```

> [!NOTE]
> Concatenating a string and an integer requires an explicit cast:
> ```jocky
> // CORRECT:
> string msg = "Process count: " + (string)count;
>
> // COMPILE ERROR: E0007: Cannot add string and int without explicit cast
> string msg = "Process count: " + count;
> ```

---

## 3. Relational & Equality Operators

Relational operators evaluate numeric comparisons and return a `bool` (`true` or `false`):

```jocky
int pid = 1042;
bool is_system = pid < 1000;
bool is_valid = pid >= 1 && pid <= 65535;
```

### Equality Semantics:
- For primitive types (`int`, `float`, `byte`, `bool`), `==` and `!=` compare raw bit values.
- For `string` and `bytes`, `==` performs a **deep byte-by-byte content comparison**, not pointer comparison.
- For `struct`, `list`, and `map` instances, `== nil` checks if the reference pointer is `nil`.

```jocky
string str1 = "svchost.exe";
string str2 = "svchost.exe";

if str1 == str2 {
    log.info("Strings match by content."); // Always true
}
```

---

## 4. Logical Operators

JOCKY provides standard boolean logic operators with **short-circuit evaluation**:
- `&&` (Logical AND): Evaluates the right operand only if the left operand is `true`.
- `||` (Logical OR): Evaluates the right operand only if the left operand is `false`.
- `!` (Logical NOT): Inverts a boolean truth value.

```jocky
// Short-circuit safety: proc.name is never evaluated if proc is nil
if proc != nil && proc.name == "malware.exe" {
    log.warn("Target implant detected: PID " + (string)proc.pid);
}
```

---

## 5. Bitwise & Shift Operators

For low-level binary analysis, packet parsing, and memory mask inspection, JOCKY supports standard bitwise operators on `int` and `byte`:

```jocky
int flags = 0b0010_1100;
int mask  = 0b0000_1111;

int bitwise_and = flags & mask;  // 0b0000_1100 (12)
int bitwise_or  = flags | 0x01;  // 0b0010_1101
int bitwise_xor = flags ^ 0xFF;  // Bitwise invert lower 8 bits
int bitwise_not = ~flags;        // Unary bitwise inversion

int shifted_left  = 1 << 8;      // 256
int shifted_right = 1024 >> 2;   // 256
```

---

## 6. The Strict Bitwise/Comparison Ambiguity Prohibition

In standard C and C++, operator precedence contains a notorious historical flaw: equality (`==`) and relational (`<`) operators have **higher precedence than bitwise AND (`&`) and OR (`|`)**.

Consider this classic, catastrophic C bug:
```c
// DANGEROUS C CODE:
if (flags & FLAG_ELEVATED == FLAG_ELEVATED) { ... }

// In C, this is parsed as:
if (flags & (FLAG_ELEVATED == FLAG_ELEVATED)) { ... }
// Which simplifies to: if (flags & 1) -> WRONG LOGIC!
```

Thousands of forensic scripts and security utilities have suffered severe logic flaws because bitwise masks failed silently due to this precedence trap.

### The JOCKY Solution: Mandatory Parentheses
In JOCKY, **mixing bitwise/shift operators with relational/equality or additive operators in the same expression without explicit parentheses is a fatal compile-time syntax error (`E0005`)**:

```jocky
// COMPILE ERROR: E0005: Ambiguous mixed bitwise and comparison expression.
// Explicit parentheses are MANDATORY.
if flags & MASK == MASK { ... }

// COMPILE ERROR: E0005: Ambiguous mixed shift and addition expression.
int offset = base + 1 << 4;

// CORRECT & UNAMBIGUOUS:
if (flags & MASK) == MASK {
    log.info("Flag matched successfully.");
}

int offset = base + (1 << 4);
```

```
+-------------------------------------------------------------------------+
|                  The JOCKY Bitwise Disambiguation Rule                  |
+-------------------------------------------------------------------------+
|   flags & MASK == MASK    ---> COMPILE ERROR: Ambiguous Precedence      |
|                                                                         |
|   (flags & MASK) == MASK  ---> COMPILES CLEANLY & SAFELY                |
+-------------------------------------------------------------------------+
```

---

## 7. Assignment & Compound Operators

In addition to standard assignment (`=`), JOCKY supports compound assignment operators:

```jocky
int counter = 0;
counter += 10;  // counter = counter + 10;
counter -= 2;   // counter = counter - 2;
counter *= 5;   // counter = counter * 5;
counter /= 2;   // counter = counter / 2;
counter %= 3;   // counter = counter % 3;

byte mask = 0xF0;
mask &= 0x30;   // mask = mask & 0x30;
mask |= 0x01;   // mask = mask | 0x01;
mask ^= 0xFF;   // mask = mask ^ 0xFF;
```

---

## 8. Explicit Cast Expressions

Type conversions are expressed using prefix cast notation: `(target_type)expression`.

```jocky
int raw_id = 4912;
string str_id = (string)raw_id;

float ratio = 3.99;
int truncated = (int)ratio; // 3

byte b = (byte)0x1FF;       // 0xFF (masked to 8 bits)
```

---

## 9. The Ternary Conditional Operator

The ternary operator `condition ? true_expr : false_expr` allows compact value selection based on a boolean condition:

```jocky
bool is_secure = true;
string protocol = is_secure ? "https" : "http";
int port = is_secure ? 443 : 80;
```

---

## 10. Complex Expression Evaluation & Practical Examples

### Example 1: PE Header Offset Calculation
```jocky
fn parse_pe_optional_header(bytes pe_buffer) -> int {
    // Verify DOS magic "MZ"
    if pe_buffer[0] != 'M' || pe_buffer[1] != 'Z' {
        return 0;
    }

    // e_lfanew offset located at offset 0x3C (60)
    int lfanew = ((int)pe_buffer[0x3C]) | (((int)pe_buffer[0x3D]) << 8);

    // PE Signature verification
    int pe_sig_offset = lfanew;
    if pe_buffer[pe_sig_offset] == 'P' && pe_buffer[pe_sig_offset + 1] == 'E' {
        // Optional header starts 24 bytes after PE signature
        return pe_sig_offset + 24;
    }

    return 0;
}
```

### Example 2: Memory Protection Bitmask Audit
```jocky
const int PAGE_EXECUTE = 0x10;
const int PAGE_EXECUTE_READ = 0x20;
const int PAGE_EXECUTE_READWRITE = 0x40;

fn is_memory_executable(int protect_flags) -> bool {
    // Explicit parentheses mandatory under JOCKY compiler rules
    bool is_exec = ((protect_flags & PAGE_EXECUTE) != 0) ||
                   ((protect_flags & PAGE_EXECUTE_READ) != 0) ||
                   ((protect_flags & PAGE_EXECUTE_READWRITE) != 0);
    return is_exec;
}
```

---

## 11. Chapter Summary

- **Precedence Hierarchy:** Follows standard C hierarchy, but with strict safety guards.
- **Mandatory Parentheses:** Mixing bitwise (`&`, `|`, `^`, `<<`, `>>`) with comparison or arithmetic operators without explicit parentheses is a fatal compile-time error (`E0005`).
- **Content-Based Equality:** `string` and `bytes` compare deep content with `==`.
- **Casting:** Prefix casting `(type)val` is mandatory for all type conversions.

In the next chapter, **[Chapter 14: Annotations & Compiler Directives](ch14_annotations.md)**, we examine JOCKY's annotation system, runtime privilege enforcement, and platform guards.
