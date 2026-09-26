# Chapter 10: Control Flow

> *"Control flow in forensic systems must be crystal clear to the analyst, yet structurally opaque in compiled machine code. JOCKY provides clean, unencumbered branching primitives that the compiler subsequently transforms into flattened state machines."*

---

## Table of Contents
1. [Conditional Branching: `if` and `else`](#conditional-branching-if-and-else)
   - [Parentheses-Free Condition Syntax](#parentheses-free-condition-syntax)
   - [Nested `if` / `else if` / `else` Chains](#nested-if--else-if--else-chains)
2. [Repetition: The `while` Loop](#repetition-the-while-loop)
3. [Iteration: The `for` Loop Forms](#iteration-the-for-loop-forms)
   - [C-Style Indexed `for` Loops](#c-style-indexed-for-loops)
   - [Collection Range Iteration (`for item in collection`)](#collection-range-iteration-for-item-in-collection)
4. [Loop Control: `break` and `continue`](#loop-control-break-and-continue)
5. [The Ternary Conditional Expression (`cond ? a : b`)](#the-ternary-conditional-expression-cond--a--b)
6. [Statements vs. Expressions: Why `if` Is Not an Expression](#statements-vs-expressions-why-if-is-not-an-expression)
7. [Design Choices: The Deliberate Omission of `goto` and `switch`](#design-choices-the-deliberate-omission-of-goto-and-switch)
8. [Practical Forensic Control Flow Patterns](#practical-forensic-control-flow-patterns)
9. [Chapter Summary](#chapter-summary)

---

## 1. Conditional Branching: `if` and `else`

JOCKY provides conditional branching through the `if` and `else` statements.

### Parentheses-Free Condition Syntax
Following modern systems language conventions (like Go and Rust), **parentheses around the condition expression are not required and are omitted by convention**:

```jocky
// Idiomatic JOCKY conditional:
if is_admin {
    log.info("Elevated administrator privileges detected.");
} else {
    log.warn("Running in standard user security context.");
}
```

```
+-----------------------------------------------------------------------------+
|                          Conditional Syntax Rules                           |
+-----------------------------------------------------------------------------+
|  Correct:     if active_connections > 0 { ... }                             |
|  Optional:    if (active_connections > 0) { ... }  (Parentheses permitted)  |
|  MANDATORY:   Opening brace '{' MUST be present. No single-statement ifs.   |
+-----------------------------------------------------------------------------+
```

> [!WARNING]
> Unlike C, JOCKY strictly forbids single-line statements without curly braces:
> ```c
> // In C (Permitted, but dangerous and error-prone):
> if (status == 0) do_cleanup();
> ```
> In JOCKY, curly braces `{ ... }` are **always mandatory**.

---

### Nested `if` / `else if` / `else` Chains

Multiple conditional branches are constructed using standard `else if` syntax:

```jocky
fn evaluate_threat_severity(int threat_score) -> string {
    if threat_score >= 90 {
        return "CRITICAL";
    } else if threat_score >= 70 {
        return "HIGH";
    } else if threat_score >= 40 {
        return "MEDIUM";
    } else if threat_score >= 10 {
        return "LOW";
    } else {
        return "INFORMATIONAL";
    }
}
```

---

## 2. Repetition: The `while` Loop

The `while` statement executes its enclosed block repeatedly as long as its boolean condition evaluates to `true`:

```jocky
fn poll_target_process(int target_pid, int timeout_seconds) -> bool {
    int elapsed = 0;

    while elapsed < timeout_seconds {
        auto p, err = process.info(target_pid);
        if err == nil && p != nil {
            log.info("Process " + (string)target_pid + " is still active.");
            return true;
        }

        host.sleep(1000); // Sleep 1 second
        elapsed = elapsed + 1;
    }

    log.warn("Process polling timed out.");
    return false;
}
```

### Infinite Loops:
An infinite loop is written simply as `while true { ... }`:

```jocky
while true {
    auto event = network.poll_packet();
    if event == nil {
        break; // Exit loop on empty stream
    }
    process_event(event);
}
```

---

## 3. Iteration: The `for` Loop Forms

JOCKY provides two explicit forms of the `for` loop:
1. **C-Style 3-Clause Indexed Loop**
2. **Collection Range Iterator (`for..in`)**

---

### C-Style Indexed `for` Loops

The 3-clause `for` loop contains an initialization clause, a continuation condition, and a post-iteration increment statement:

```jocky
// Iterating over a numeric range
for int i = 0; i < 10; i++ {
    log.info("Executing triage pass: " + (string)i);
}

// Stepping through a raw byte buffer
bytes buffer = fs.read_raw(file_path);
int buf_len = buffer.len();

for int offset = 0; offset < buf_len; offset = offset + 16 {
    inspect_hex_row(buffer, offset);
}
```

---

### Collection Range Iteration (`for item in collection`)

When traversing items in a `list` or keys in a `map`, the `for..in` loop provides high-level, bounds-safe iteration:

```jocky
// Iterating over a list of processes
list procs = process.list();
for proc in procs {
    if proc.name == "cmd.exe" || proc.name == "powershell.exe" {
        log.warn("Command interpreter active: PID " + (string)proc.pid);
    }
}

// Iterating over network connections
list conns = network.connections();
for conn in conns {
    if conn.remote_port == 4444 || conn.remote_port == 1337 {
        log.warn("Suspicious outbound connection detected: " + conn.remote_ip);
    }
}
```

---

## 4. Loop Control: `break` and `continue`

JOCKY supports standard loop interruption statements:
- **`break;`** terminates the innermost enclosing `while` or `for` loop immediately.
- **`continue;`** skips the remainder of the current loop iteration and advances directly to the loop condition or post-iteration clause.

```jocky
fn find_process_by_name(string target_name) -> ProcessInfo {
    auto procs = process.list();
    ProcessInfo match = nil;

    for p in procs {
        // Skip irrelevant processes immediately
        if p.name != target_name {
            continue;
        }

        // Found target process!
        match = p;
        break; // Stop scanning further
    }

    return match;
}
```

---

## 5. The Ternary Conditional Expression (`cond ? a : b`)

In JOCKY, statements cannot be assigned to variables. However, inline conditional assignments are frequently needed for compact value selection.

The **ternary operator (`? :`) is the sole expression-level conditional in the language**:

$$\text{Expression} = \text{Condition} \; \mathbf{?} \; \text{TrueValue} \; \mathbf{:} \; \text{FalseValue}$$

```jocky
int status_code = is_authorized ? 200 : 403;
string mode_label = is_stealth ? "COVERT_AGENT" : "VERBOSE_TRIAGE";
int timeout = is_fast_scan ? 1000 : 30000;
```

### Type Invariant:
Both the true-branch and false-branch expressions in a ternary operator **must evaluate to the identical static type**. Mixing types (e.g. `is_ok ? "success" : 0`) is a fatal compile-time error (`E0013`).

---

## 6. Statements vs. Expressions: Why `if` Is Not an Expression

In languages like Rust or Kotlin, `if` is an expression that yields a value (`let x = if c { 1 } else { 2 };`).

In JOCKY, **`if`, `while`, and `for` are strictly statements, not expressions**:

```jocky
// ILLEGAL IN JOCKY:
// auto x = if cond { 1 } else { 2 }; // SYNTAX ERROR

// IDIOMATIC IN JOCKY:
// Use ternary for expressions:
auto x = cond ? 1 : 2;

// Or use standard block statements:
int x = 0;
if cond {
    x = 1;
} else {
    x = 2;
}
```

### Rationale:
Treating control flow blocks strictly as statements simplifies the AST lowering pass and guarantees that the compiler's **Control-Flow Graph (CFG) Flattening Engine** can reliably restructure all branches into uniform state machines without having to manage implicit stack-expression return values.

---

## 7. Design Choices: The Deliberate Omission of `goto` and `switch`

### Why No `goto`?
1. **Unstructured Control Flow:** Arbitrary jumps create "spaghetti code" that complicates static verification and memory lifetime tracking for the garbage collector.
2. **Obfuscation Integrity:** The compiler already performs sophisticated control-flow obfuscation internally. User-level `goto` constructs interfere with the compiler's internal state machine generation.

### Why No `switch` / `match` in v0.1?
1. **Minimizing Language Surface Area:** `if / else if / else` covers 100% of multi-way branching requirements with zero syntactic ambiguity.
2. **Predictable Code Generation:** Avoids compiler jump-table artifacts in the final binary, which are easily fingerprinted by reverse engineering tools (e.g. IDA Pro / Ghidra jump table pattern matchers).

---

## 8. Practical Forensic Control Flow Patterns

### Pattern 1: Safe Resource Inspection with Guard Clauses
```jocky
fn inspect_sensitive_directory(string dir_path) -> void {
    // 1. Guard check for directory existence
    if !fs.exists(dir_path) {
        log.warn("Directory does not exist: " + dir_path);
        return;
    }

    // 2. Guard check for read permissions
    auto files, err = fs.list(dir_path);
    if err != nil {
        log.error("Failed to enumerate directory: " + err.message);
        return;
    }

    // 3. Process items cleanly
    for file in files {
        log.info("Found artifact: " + file.path);
    }
}
```

### Pattern 2: Bounded Polling Loop with Graceful Degradation
```jocky
fn await_evidence_sync(int max_retries) -> bool {
    int attempts = 0;

    while attempts < max_retries {
        bool is_ready = check_evidence_status();
        if is_ready {
            log.info("Evidence sync completed after " + (string)attempts + " attempts.");
            return true;
        }

        attempts++;
        host.sleep(500);
    }

    log.error("Evidence sync failed after max retries.");
    return false;
}
```

---

## 9. Chapter Summary

- **Syntax:** `if` and `while` require no parentheses around conditions, but curly braces `{}` are strictly mandatory.
- **Loops:** Full support for both C-style `for int i = 0; i < N; i++` and collection iteration `for item in list`.
- **Ternary Operator:** `cond ? a : b` is the sole expression-level conditional and requires identical operand types.
- **Statement Primacy:** `if` is a statement, not an expression, facilitating compiler-level CFG flattening.
- **No `goto` or `switch`:** Eliminates jump-table signatures and guarantees structured control flow.

In the next chapter, **[Chapter 11: Error Handling & Propagation](ch11_error_handling.md)**, we examine JOCKY's Result tuple architecture, the `!` try-operator, and failure recovery patterns.
