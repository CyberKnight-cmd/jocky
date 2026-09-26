# Chapter 7: Variables, Constants, and Scoping

> *"State management in high-stakes environments requires absolute predictability. In JOCKY, every variable has a deterministic lifecycle, global mutable state is strictly prohibited, and uninitialized memory is rendered syntactically impossible."*

---

## Table of Contents
1. [Variable Declaration Syntax & Forms](#variable-declaration-syntax--forms)
   - [Explicitly Typed Declarations](#explicitly-typed-declarations)
   - [Automatic Type Inference via `auto`](#automatic-type-inference-via-auto)
   - [Multi-Variable Tuple Assignments](#multi-variable-tuple-assignments)
2. [Constants & Compile-Time Immutability (`const`)](#constants--compile-time-immutability-const)
3. [The Zero-Uninitialized-Variables Policy](#the-zero-uninitialized-variables-policy)
4. [Lexical Block Scoping & Shadowing](#lexical-block-scoping--shadowing)
5. [The Blank Identifier `_`](#the-blank-identifier-_)
6. [Global Scope & The Prohibition of Mutable State](#global-scope--the-prohibition-of-mutable-state)
7. [Variable Lifetimes & Garbage Collector Interaction](#variable-lifetimes--garbage-collector-interaction)
8. [Edge Cases, Common Pitfalls, & Best Practices](#edge-cases-common-pitfalls--best-practices)
9. [Chapter Summary](#chapter-summary)

---

## 1. Variable Declaration Syntax & Forms

In JOCKY, variables are named storage bindings associated with a specific static type and lexical scope.

### Explicitly Typed Declarations
The most direct form of variable declaration specifies the explicit type name followed by the identifier, an assignment operator (`=`), an initial expression, and a mandatory semicolon (`;`):

```jocky
// Explicitly typed variable bindings
int max_threads = 8;
float detection_threshold = 0.95;
bool enable_deep_scan = true;
string case_reference = "CASE-2026-NTRO";
byte status_flag = 0x01;
bytes header_magic = x"4D5A";
```

---

### Automatic Type Inference via `auto`

When the initial assignment expression provides unambiguous type information, JOCKY allows the `auto` keyword. The compiler infers the exact static type during semantic analysis pass 1, preserving 100% static safety without boilerplate repetition.

```jocky
// 'info' is statically inferred as map / Struct HostInfo
auto info = host.info();

// 'procs' is statically inferred as list
auto procs = process.list();

// 'msg' is statically inferred as string
auto msg = "Triage sequence active";
```

```
+------------------------------------------------------------------------+
|                      Type Inference Mechanism                          |
+------------------------------------------------------------------------+
| Source Code:    auto count = 100;                                      |
| Parser AST:     DeclVar(name="count", type=TYPE_AUTO, init=Int(100))   |
| Semantic Pass:  TypeCheck(init) -> TYPE_INT                            |
| Symbol Table:   Symbol("count", type=TYPE_INT, offset=0x18)            |
| Final Machine:  int64_t count = 100; (Native stack allocation)         |
+------------------------------------------------------------------------+
```

---

### Multi-Variable Tuple Assignments

When invoking functions that return multiple values (such as `(Result, Error)` tuples), JOCKY supports multi-variable declaration syntax:

```jocky
// Declare value and error simultaneously
auto case_file, err = evidence.open("CASE-2026-881");
if err != nil {
    log.error("Failed to open case: " + err.message);
    return;
}

// Explicit types in multi-variable declaration
CaseFile cf, Error open_err = evidence.open("CASE-2026-882");
```

---

## 2. Constants & Compile-Time Immutability (`const`)

Constants are immutable values evaluated and locked at compile time. They are declared using the `const` keyword:

```jocky
// Module-level compile-time constants
const int MAX_BUFFER_SIZE = 65536;
const string DEFAULT_CASE_PREFIX = "NTRO-TRIAGE-";
const float TIMEOUT_SECONDS = 30.0;
const byte PE_MAGIC_BYTE = 'M';
```

### Constant Rules:
1. **Compile-Time Evaluation:** The right-hand side of a `const` declaration must be a constant expression composed strictly of literal values, arithmetic operations on other constants, or built-in compile-time intrinsics.
2. **Immutability:** Reassigning to a `const` identifier generates compiler error `E0008`:

```jocky
const int LIMIT = 50;

// COMPILE ERROR: E0008: Cannot assign to constant variable 'LIMIT'
LIMIT = 100;
```

---

## 3. The Zero-Uninitialized-Variables Policy

In C and C++, declaring a variable without an explicit initializer leaves uninitialized "garbage" data on the stack. Reading an uninitialized pointer or buffer is one of the most common causes of memory corruption, analytical inaccuracies, and program crashes:

```c
// Dangerous C code:
int target_pid; // Contains arbitrary stack garbage!
if (condition) {
    target_pid = 1024;
}
kill(target_pid, SIGTERM); // Might kill an arbitrary system process!
```

JOCKY eliminates this entire class of bugs through the **Zero-Uninitialized-Variables Policy**:

> **Compiler Invariant:** Every variable in JOCKY MUST have an explicit initializer at declaration. Declaring a variable without assigning an initial value is a fatal compile-time syntax error (`E0009`).

```jocky
// Valid:
int target_pid = 0;
string active_user = "";
list connections = [];

// COMPILE ERROR: E0009: Variable 'target_pid' declared without initializer
int target_pid;
```

---

## 4. Lexical Block Scoping & Shadowing

JOCKY uses strict **lexical block scoping**. A block is defined by an opening curly brace `{` and its corresponding closing curly brace `}`.

### Scoping Rules:
- Variables declared inside a block are visible only within that block and any nested child blocks.
- Once execution exits the closing brace `}`, the inner variables go out of scope and become eligible for memory reclamation.

```jocky
fn execute_scan() -> void {
    int outer_val = 10;

    if outer_val > 5 {
        int inner_val = 20;
        log.info("Sum: " + (string)(outer_val + inner_val)); // Valid
    }

    // COMPILE ERROR: E0010: Undefined identifier 'inner_val' in scope
    log.info((string)inner_val);
}
```

### Variable Shadowing
JOCKY permits variable shadowing in nested scopes, allowing an inner block to declare a variable with the same name as an outer variable:

```jocky
fn demonstrate_shadowing() -> void {
    string status = "PENDING";
    log.info("Outer status: " + status); // Prints "PENDING"

    if true {
        // Shadowing outer 'status' with a local integer
        int status = 200;
        log.info("Inner status code: " + (string)status); // Prints "200"
    }

    log.info("Outer status restored: " + status); // Prints "PENDING"
}
```

---

## 5. The Blank Identifier `_`

When calling functions that return multiple values, analysts frequently need only a subset of the returned tuple. Using a dummy variable creates unused variable warnings or wastes memory.

The **blank identifier** (`_`) acts as an anonymous, write-only discard target:

```jocky
// Function returns (int bytes_written, Error err)
auto _, write_err = file.write(payload);
if write_err != nil {
    log.error("Write failed: " + write_err.message);
}

// Discarding loop index
for item in process_list {
    log.info("Inspecting process: " + item.name);
}
```

> [!NOTE]
> The blank identifier cannot be read or used in expressions. Writing `auto x = _ + 5;` is a compile-time error.

---

## 6. Global Scope & The Prohibition of Mutable State

In multi-threaded or covert forensic operations, global mutable variables create race conditions, non-deterministic bugs, and conspicuous `.data` section footprints that security sensors flag.

JOCKY strictly enforces the **Zero Mutable Global State Policy**:

1. **Only Constants Permitted at Top-Level:** At the file/module level, only `const` declarations, `struct` definitions, `fn` definitions, and `import` statements are allowed.
2. **No Top-Level Mutable Variables:** Writing `int global_counter = 0;` outside of a function or method in Agent Mode is a fatal compile-time error (`E0011`).

```jocky
// Module level:

// Allowed: Immutable constant
const string REPORT_VERSION = "1.0.4";

// COMPILE ERROR: E0011: Global mutable variables are prohibited in Agent Mode
int active_case_count = 0;
```

---

## 7. Variable Lifetimes & Garbage Collector Interaction

JOCKY manages memory via a hybrid allocation strategy:
- **Stack Allocation:** Primitive value types (`int`, `float`, `byte`, `bool`) are allocated directly on the CPU stack frame. Their memory is automatically reclaimed the instant the enclosing stack frame pops.
- **Heap Allocation with Mark-and-Sweep GC:** Dynamic reference types (`string`, `bytes`, `list`, `map`, `struct`, `Error`) allocate memory descriptors in the runtime heap.

```
+--------------------------------------------------------------------+
|                         Memory Lifecycles                          |
+--------------------------------------------------------------------+
| CPU Stack Frame:                                                   |
|   [ int pid = 1042        ] -> Instant push/pop (Zero GC cost)     |
|   [ float threshold = 0.8 ] -> Instant push/pop (Zero GC cost)     |
|   [ String Descriptor Ptr ] ----+                                  |
|                                 |                                  |
| Heap (Managed by GC):           |                                  |
|   [ "CASE-2026-ALPHA-01"  ] <---+ Traced by Mark-and-Sweep GC      |
+--------------------------------------------------------------------+
```

### GC Root Registration
When a local reference variable is declared, the JOCKY code generator emits a local root registration macro (`JKY_GC_ROOT_PUSH(&var)`). When the function returns, roots are unregistered (`JKY_GC_ROOT_POP()`). If memory allocation pressure triggers a garbage collection cycle, active references are preserved while dead allocations are swept silently in microseconds.

---

## 8. Edge Cases, Common Pitfalls, & Best Practices

### Pitfall 1: Attempting to Use Uninitialized Variables in Conditional Branches
```jocky
// WRONG: Syntax error at declaration
int port;
if is_ssl { port = 443; } else { port = 80; }

// CORRECT: Initialize with a safe default or use ternary operator
int port = is_ssl ? 443 : 80;
```

### Pitfall 2: Accidental Identifier Shadowing in Error Checks
```jocky
auto case_file, err = evidence.open("CASE-01");
if err != nil { return; }

if true {
    // Declaring a NEW 'err' shadows outer 'err'
    auto procs, err = process.list();
    if err != nil { log.warn("Process listing failed"); }
}
// Outer 'case_file' is still safe and valid
```

---

## 9. Chapter Summary

- **Declarations:** Explicit typing or `auto` local inference with mandatory initializers and semicolons.
- **Constants:** Declared with `const`, evaluated at compile-time, strictly immutable.
- **Safety Guarantee:** Zero uninitialized variables allowed; compilation fails if any declaration lacks an initializer.
- **No Global Mutable State:** Top-level scope supports only `const`, `struct`, `import`, and `fn`.
- **Memory Management:** Stack values pop with zero overhead; reference values are tracked by a lightweight mark-and-sweep garbage collector.

In the next chapter, **[Chapter 8: Functions & Method Receivers](ch08_functions.md)**, we cover function signatures, multi-return tuples, error propagation, method syntax, and compiler annotations.
