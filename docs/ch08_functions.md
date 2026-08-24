# Chapter 8: Functions & Method Receivers

> *"Functions in JOCKY are the atomic units of execution. Built with explicit return types, strict signature matching, and method receiver bindings, they ensure that every computational path is fully verifiable."*

---

## Table of Contents
1. [Function Declaration Syntax & Semantics](#function-declaration-syntax--semantics)
2. [Parameter Lists & Passing Semantics](#parameter-lists--passing-semantics)
3. [Return Types & The `void` Specifier](#return-types--the-void-specifier)
4. [Multi-Value Returns & The `(T, Error)` Idiom](#multi-value-returns--the-t-error-idiom)
5. [Method Receivers & Struct Binding Syntax](#method-receivers--struct-binding-syntax)
6. [Declaration Order Independence & Two-Pass Resolution](#declaration-order-independence--two-pass-resolution)
7. [Function Annotations: `@privileged`, `@inline`, `@noreturn`](#function-annotations-privileged-inline-noreturn)
8. [Recursion Mechanics & Stack Safety](#recursion-mechanics--stack-safety)
9. [Design Constraints in v0.1: No Overloading, No Function Pointers](#design-constraints-in-v01-no-overloading-no-function-pointers)
10. [Chapter Summary](#chapter-summary)

---

## 1. Function Declaration Syntax & Semantics

In JOCKY, functions are declared using the `fn` keyword. Every function requires an identifier, a parenthesized parameter list, an explicit return type arrow (`-> Type`), and an enclosing curly-brace body (`{ ... }`).

```jocky
// Basic function definition
fn add_integers(int a, int b) -> int {
    return a + b;
}

// Function taking no parameters and returning void
fn log_triage_banner() -> void {
    log.info("========================================");
    log.info("NTRO Covert Forensic Collector Active");
    log.info("========================================");
}
```

```
+-----------------------------------------------------------------------------+
|                        Function Syntax Breakdown                            |
+-----------------------------------------------------------------------------+
|   fn   calculate_hash ( bytes payload, string algo ) -> string  { ... }    |
|   |         |           \________________________/      |       \_____/     |
| Keyword Identifier              Parameter List       Return Type  Body      |
+-----------------------------------------------------------------------------+
```

---

## 2. Parameter Lists & Passing Semantics

Function parameters are declared as a comma-separated list of `Type name` pairs:

```jocky
fn filter_processes(list procs, int min_pid, bool require_elevated) -> list {
    list filtered = [];
    for p in procs {
        if p.pid >= min_pid {
            if !require_elevated || p.is_elevated {
                filtered.append(p);
            }
        }
    }
    return filtered;
}
```

### Parameter Passing Semantics:
1. **Pass-by-Value (Stack Primitives):** Types such as `int`, `float`, `byte`, and `bool` are passed by value. Modifying a primitive parameter inside a function does not alter the caller's variable.
2. **Pass-by-Reference (Heap Objects):** Reference types (`bytes`, `list`, `map`, `struct`, `Error`) pass reference handles. Modifying the contents of a passed `list` or `map` mutates the underlying heap data structure. Strings, while reference types, are **immutable**.

```jocky
fn mutate_test(int val, list items) -> void {
    val = val + 100;         // Caller's integer remains unaffected
    items.append("NEW_ITEM"); // Caller's list IS modified!
}
```

---

## 3. Return Types & The `void` Specifier

Every JOCKY function must explicitly specify its return type following the `->` token:

- If a function returns a value, the return path must culminate in a `return expression;` statement matching the declared type.
- If a function does not return a value, it must specify `-> void`. In `void` functions, `return;` is optional at the end of the function body.

```jocky
// Correct: Explicit void return
fn cleanup_temporary_state() -> void {
    log.info("Flushing caches...");
    return; // Optional
}

// COMPILE ERROR: E0012: Function declared to return 'int' has missing return path
fn calculate_metric(int a) -> int {
    if a > 0 {
        return a * 2;
    }
    // Missing return on else branch!
}
```

---

## 4. Multi-Value Returns & The `(T, Error)` Idiom

JOCKY natively supports multiple return values, primarily utilized for returning result and diagnostic tuples:

```jocky
// Returning a tuple of (int result, Error err)
fn safe_divide(int numerator, int denominator) -> (int, Error) {
    if denominator == 0 {
        return 0, JKY_ERR("Division by zero in forensic calculation");
    }
    return numerator / denominator, nil;
}

fn execute_calculation() -> void {
    auto res, err = safe_divide(100, 0);
    if err != nil {
        log.warn("Calculation aborted: " + err.message);
        return;
    }
    log.info("Result: " + (string)res);
}
```

Multi-value returns are not restricted to errors; functions may return arbitrary multiple values:

```jocky
fn get_min_max(list numbers) -> (int, int) {
    int min_val = (int)numbers[0];
    int max_val = (int)numbers[0];
    for n in numbers {
        int v = (int)n;
        if v < min_val { min_val = v; }
        if v > max_val { max_val = v; }
    }
    return min_val, max_val;
}
```

---

## 5. Method Receivers & Struct Binding Syntax

JOCKY adopts a clean, explicit **method receiver syntax** (similar to Go), binding functions directly to struct types without requiring complex class hierarchies or object-oriented inheritance:

```jocky
// 1. Define the struct
struct CaseFile {
    string case_id;
    string created_at;
    bool is_sealed;
    list artifacts;
}

// 2. Define a method with receiver (CaseFile cf)
fn (CaseFile cf) seal() -> void {
    if cf.is_sealed {
        log.warn("Case " + cf.case_id + " is already sealed.");
        return;
    }
    cf.is_sealed = true;
    log.info("Case " + cf.case_id + " sealed successfully.");
}

// 3. Define a query method returning an integer
fn (CaseFile cf) count_artifacts() -> int {
    return cf.artifacts.len();
}
```

### Invoking Methods:
Methods are invoked using standard dot notation (`object.method()`):

```jocky
fn main() -> void {
    auto cf, err = evidence.open("CASE-883");
    if err != nil { return; }

    int count = cf.count_artifacts();
    log.info("Artifacts collected: " + (string)count);

    // Call receiver method
    cf.seal();
}
```

---

## 6. Declaration Order Independence & Two-Pass Resolution

In C and C++, functions must be declared or prototyped before they can be called. In JOCKY, **declaration order is completely independent**:

```jocky
// Caller appears before callee
fn initiate_investigation() -> void {
    validate_credentials();
    collect_payload();
}

fn validate_credentials() -> void {
    log.info("Credentials verified.");
}

fn collect_payload() -> void {
    log.info("Payload acquired.");
}
```

The compiler accomplishes this through a **two-pass symbol collection**:
1. **Pass 1 (Symbol Registration):** Scans the entire file, recording all function signatures, struct layouts, and method receivers in the module symbol table.
2. **Pass 2 (Semantic Analysis & Type Checking):** Type-checks function bodies, resolves function invocations, and verifies parameter matching against the registered signatures.

---

## 7. Function Annotations: `@privileged`, `@inline`, `@noreturn`

Annotations prefix a function definition with the `@` symbol, providing compile-time directives and runtime security assertions.

### 1. `@privileged`
Marks a function as requiring elevated operating system rights (e.g. `SeDebugPrivilege` / Administrator on Windows, or `CAP_SYS_ADMIN` / root on Linux). At runtime, the JOCKY preamble asserts privileges before executing the function body:

```jocky
@privileged
fn inspect_kernel_memory_ranges() -> list {
    // Requires root / SeDebugPrivilege
    return host.kernel_ranges();
}
```

### 2. `@inline`
Instructs the code generator and backend compiler to inline the function body directly at call sites, eliminating function call overhead and obfuscating call graphs:

```jocky
@inline
fn xor_decrypt_byte(byte b, byte key) -> byte {
    return (byte)(b ^ key);
}
```

### 3. `@noreturn`
Informs the semantic analyzer and control-flow generator that the function will never return execution to the caller (e.g., process termination or emergency panic):

```jocky
@noreturn
fn emergency_wipe_and_exit(int exit_code) -> void {
    log.error("Emergency wipe triggered!");
    // Terminate process immediately
    host.exit(exit_code);
}
```

---

## 8. Recursion Mechanics & Stack Safety

JOCKY fully supports recursive function invocations:

```jocky
fn recursive_factorial(int n) -> int {
    if n <= 1 {
        return 1;
    }
    return n * recursive_factorial(n - 1);
}
```

> [!WARNING]
> Because JOCKY targets covert forensic environments where stack size may be constrained to prevent memory footprint detection, deep recursion should be avoided in favor of iterative loops.

---

## 9. Design Constraints in v0.1: No Overloading, No Function Pointers

To ensure deterministic compilation, predictable binary generation, and simplicity of analysis:

1. **No Function Overloading:** You cannot define two functions with the same identifier in the same scope, even if their parameter types differ.
2. **No First-Class Function Pointers / Closures (v0.1):** Functions cannot be assigned to variables, passed as callbacks, or stored in structs in version 0.1. (Higher-order functions and closures are scheduled for v1.0).

---

## 10. Chapter Summary

- **Syntax:** `fn name(params) -> ReturnType { ... }` with mandatory return type arrows.
- **Method Receivers:** Go-style `fn (StructType obj) method_name() -> void` binds functions directly to structs.
- **Multi-Return:** Native support for `(Value, Error)` pairs powers explicit error handling.
- **Annotations:** `@privileged`, `@inline`, and `@noreturn` alter code generation and runtime privilege assertions.
- **Order Independence:** Two-pass symbol collection allows functions to be declared in any sequence.

In the next chapter, **[Chapter 9: Structs & Data Modeling](ch09_structs_and_methods.md)**, we explore struct data modeling, field layouts, nested references, and JSON serialization.
