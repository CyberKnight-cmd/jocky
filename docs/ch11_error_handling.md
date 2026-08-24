# Chapter 11: Error Handling & Propagation

> *"In mission-critical forensic investigations, silent failures and unhandled runtime exceptions are equally catastrophic. JOCKY enforces explicit error handling at every failure point, turning resilience into a compile-time certainty."*

---

## Table of Contents
1. [The Philosophy of Explicit Failure](#the-philosophy-of-explicit-failure)
2. [The Result Tuple Idiom: `(T, Error)`](#the-result-tuple-idiom-t-error)
3. [Evaluating Errors: The `nil` Invariant](#evaluating-errors-the-nil-invariant)
4. [The `!` Error Propagation Operator](#the--error-propagation-operator)
   - [How `!` Desugars at Compile Time](#how--desugars-at-compile-time)
   - [When `!` Triggers a Compile-Time Error](#when--triggers-a-compile-time-error)
5. [Constructing Errors: `JKY_ERR` and Custom Errors](#constructing-errors-jky_err-and-custom-errors)
6. [Error Chaining & Diagnostic Context Wrapping](#error-chaining--diagnostic-context-wrapping)
7. [Expected Absence vs. Hard Failure: The `(T, bool)` Pattern](#expected-absence-vs-hard-failure-the-t-bool-pattern)
8. [Controlled Panics vs. Recoverable Errors](#controlled-panics-vs-recoverable-errors)
9. [Forensic Error Handling Best Practices](#forensic-error-handling-best-practices)
10. [Chapter Summary](#chapter-summary)

---

## 1. The Philosophy of Explicit Failure

When operating in an incident response scenario, system calls fail constantly:
- An endpoint process terminates in the middle of being queried.
- An operating system ACL restricts read access to a locked registry hive.
- An adversarial rootkit hooks and blocks a memory acquisition call.
- A network socket closes abruptly due to firewall reset packets.

In traditional languages that rely on exceptions (`try / catch / throw`), errors are frequently dropped, unhandled exceptions crash the process, and hidden stack unwinding code bloats the binary footprint.

JOCKY rejects exceptions entirely. Instead, **error handling is explicit, local, and encoded directly into function signatures as multiple return tuples**.

```
+-----------------------------------------------------------------------------+
|                          Exception vs. Result Model                         |
+-----------------------------------------------------------------------------+
| Exception Approach (C++ / Python):                                          |
|   Call() ----> [ Unwind Tables (.eh_frame) ] ----> Crash or Hidden Catch    |
|   - Binary footprint bloated with unwind metadata                           |
|   - Control flow jumps non-locally across stack frames                      |
|                                                                             |
| JOCKY Result Tuple Approach:                                                |
|   Call() ----> Returns (Value, Error) ----> Explicit Local Verification    |
|   - Zero metadata bloat; minimal native instructions                        |
|   - Control flow is 100% linear, transparent, and auditable                 |
+-----------------------------------------------------------------------------+
```

---

## 2. The Result Tuple Idiom: `(T, Error)`

Any function that can encounter a recoverable failure returns a multi-value tuple consisting of the expected return type `T` and an `Error` object:

```jocky
// Function returning a tuple of (CaseFile, Error)
fn open_case_bundle(string case_id) -> (CaseFile, Error) {
    if case_id.len() == 0 {
        return nil, JKY_ERR("Invalid case identifier: ID cannot be empty");
    }

    CaseFile cf = CaseFile {
        case_id: case_id,
        created_at: host.time_iso(),
        is_sealed: false,
        artifacts: []
    };

    return cf, nil; // Return valid object and nil error
}
```

---

## 3. Evaluating Errors: The `nil` Invariant

In JOCKY, **`nil` represents the total absence of an error**. If the returned `Error` instance is `nil`, the preceding value is guaranteed to be valid and initialized.

```jocky
fn execute_triage() -> void {
    auto case_bundle, err = open_case_bundle("CASE-2026-NTRO");

    // Standard idiomatic error check:
    if err != nil {
        log.error("Aborting triage: " + err.message + " (Code: " + (string)err.code + ")");
        return;
    }

    // 'case_bundle' is guaranteed to be non-nil here
    log.info("Case opened successfully: " + case_bundle.case_id);
}
```

---

## 4. The `!` Error Propagation Operator

Writing `if err != nil { return nil, err; }` across every nested function call can introduce repetitive boilerplate.

JOCKY provides the **`!` try-propagation operator** as a concise postfix syntactic sugar.

```jocky
// Using '!' for concise inline propagation:
fn triage_system(string case_id) -> (CaseFile, Error) {
    // If evidence.open fails, it returns early with the error automatically!
    auto case_file = evidence.open(case_id)!;

    // Collect host info with propagation
    auto host_data = host.info_safe()!;
    evidence.add(case_file, "host", host_data);

    return case_file, nil;
}
```

---

### How `!` Desugars at Compile Time

When the JOCKY parser encounters an expression suffixed with `!`, it lowers the AST node into an explicit error check and early return:

```jocky
// Source code written by analyst:
auto cf = evidence.open(case_id)!;
```

```c
// Lowered AST / Generated C equivalent:
auto _tmp_val, _tmp_err = evidence.open(case_id);
if (_tmp_err != nil) {
    return nil, _tmp_err; // Early return to caller!
}
auto cf = _tmp_val;
```

---

### When `!` Triggers a Compile-Time Error

The `!` operator can **only be used within a function whose return type signature includes `Error` as its final return value**.

If an analyst attempts to use `!` inside a function returning `void`, `int`, or any signature lacking `Error`, the compiler halts with error `E0014`:

```jocky
// ILLEGAL: main() returns void, not (..., Error)
fn main() -> void {
    // COMPILE ERROR: E0014: Cannot use '!' operator in function returning 'void'
    // Enclosing function must return 'Error' in its return signature.
    auto cf = evidence.open("CASE-01")!;
}

// CORRECT: Handle error explicitly in void functions
fn main() -> void {
    auto cf, err = evidence.open("CASE-01");
    if err != nil {
        log.error("Triage initialization failed: " + err.message);
        return;
    }
}
```

---

## 5. Constructing Errors: `JKY_ERR` and Custom Errors

Errors are instantiated using the built-in constructor macro `JKY_ERR(message)` or by creating an `Error` struct explicitly:

```jocky
// 1. Using standard constructor macro:
return nil, JKY_ERR("Access denied to target process memory");

// 2. Specifying explicit numeric error code:
return nil, Error {
    code: 403,
    message: "Insufficient privilege: Requires SeDebugPrivilege"
};
```

---

## 6. Error Chaining & Diagnostic Context Wrapping

When an error bubbles up through multiple subsystem layers, adding operational context (e.g. which PID or file path caused the failure) is critical for forensic post-mortems:

```jocky
fn dump_process_memory(int pid) -> (bytes, Error) {
    auto mem, err = process.read_memory(pid, 0x00400000, 4096);
    if err != nil {
        // Wrap the error with high-level contextual details
        return nil, Error {
            code: err.code,
            message: "Failed to dump memory for PID " + (string)pid + ": " + err.message
        };
    }
    return mem, nil;
}
```

---

## 7. Expected Absence vs. Hard Failure: The `(T, bool)` Pattern

Not every missing item constitutes an exceptional error condition. For example, querying whether an optional configuration key exists or searching for a non-existent process by name is an expected scenario.

For expected absences, JOCKY standard libraries adopt the **Ok-Tuple `(T, bool)` Pattern**:

```jocky
// Function returns (ProcessInfo match, bool found)
fn find_daemon(string name) -> (ProcessInfo, bool) {
    auto procs = process.list();
    for p in procs {
        if p.name == name {
            return p, true;
        }
    }
    return nil, false; // Not found, but NOT an Error
}

fn check_agent() -> void {
    auto proc, found = find_daemon("edr_sensor.exe");
    if !found {
        log.info("EDR sensor not present on target host.");
    } else {
        log.warn("Active EDR sensor detected: PID " + (string)proc.pid);
    }
}
```

```
+-------------------------------------------------------------------------+
|                  (T, Error) vs. (T, bool) Guidelines                    |
+-------------------------------------------------------------------------+
| Use (T, Error) when:                                                    |
|   - The operation failed unexpectedly (I/O error, permission denied).   |
|   - Diagnostic context / error codes are required.                      |
|                                                                         |
| Use (T, bool) when:                                                     |
|   - Checking for optional existence (map key lookup, search query).     |
|   - Absence is a completely normal, expected operational outcome.       |
+-------------------------------------------------------------------------+
```

---

## 8. Controlled Panics vs. Recoverable Errors

| Category | Mechanism | Scenario | Recovery / Outcome |
| :--- | :--- | :--- | :--- |
| **Recoverable Error** | Return `(T, Error)` | API failure, locked file, network timeout | Handled explicitly via `if err != nil` or `!` |
| **Fatal Panic** | `panic(message)` | Nil pointer dereference, corrupted memory invariant | Immediate process abort; exit code 139 |

When a runtime panic occurs (such as attempting to dereference a `nil` struct pointer), JOCKY:
1. Immediately flushes any unsealed active evidence bundles to disk in emergency mode.
2. Emits a minimal diagnostic panic message to `stderr`.
3. Terminates the process immediately to prevent volatile state corruption.

---

## 9. Forensic Error Handling Best Practices

1. **Never Discard Errors Silently:** Avoid using `auto cf, _ = evidence.open(...);` unless you are deliberately writing a non-critical probe.
2. **Use `!` for Clean Pipelines:** In functions that return `(..., Error)`, chain calls cleanly with the `!` operator.
3. **Log Before Exiting:** Always emit structured log messages with `log.error()` before returning early from top-level triage routines.

```jocky
// Idiomatic, resilient triage pipeline:
fn run_full_triage(string case_id) -> (CaseFile, Error) {
    auto cf = evidence.open(case_id)!;
    
    auto host_telemetry = host.info_safe()!;
    evidence.add(cf, "host_info", host_telemetry);

    auto proc_list = process.list_safe()!;
    evidence.add(cf, "processes", proc_list);

    cf.seal();
    return cf, nil;
}
```

---

## 10. Chapter Summary

- **Result Tuples:** Functions return explicit `(T, Error)` pairs rather than throwing exceptions.
- **Nil Error Invariant:** If `err == nil`, the operation succeeded.
- **Propagation (`!`):** The `!` operator provides concise propagation of errors in functions that return `Error`.
- **Compile-Time Safety:** Using `!` in functions returning `void` is caught at compile time (`E0014`).
- **Ok-Tuples:** Use `(T, bool)` for querying optional data where absence is normal.

In the next chapter, **[Chapter 12: Modules, Packages, and Source Organization](ch12_modules_and_packages.md)**, we examine JOCKY's module resolution, package namespaces, and compilation unit architecture.
