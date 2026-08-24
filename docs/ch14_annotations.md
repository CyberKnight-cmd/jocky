# Chapter 14: Annotations & Compiler Directives

> *"Annotations provide declarative metadata that bridges user code, compiler code generation, and operating system runtime security assertions without cluttering the core syntax."*

---

## Table of Contents
1. [The JOCKY Annotation Architecture](#the-jocky-annotation-architecture)
2. [Security & Privileges: `@privileged`](#security--privileges-privileged)
   - [Runtime Verification Mechanism](#runtime-verification-mechanism)
   - [Windows Token Escalation (`SeDebugPrivilege`)](#windows-token-escalation-sedebugprivilege)
   - [Linux Capability Checks (`CAP_SYS_ADMIN`, `CAP_SYS_PTRACE`)](#linux-capability-checks-cap_sys_admin-cap_sys_ptrace)
3. [Target Portability: `@platform`](#target-portability-platform)
   - [Platform Guard Semantics](#platform-guard-semantics)
   - [Cross-Platform Fallbacks](#cross-platform-fallbacks)
4. [Lifecycle Management: `@deprecated`](#lifecycle-management-deprecated)
5. [Optimization & Obfuscation: `@inline`](#optimization--obfuscation-inline)
6. [Control Flow Finality: `@noreturn`](#control-flow-finality-noreturn)
7. [Combining Multiple Annotations](#combining-multiple-annotations)
8. [Codegen Impact Matrix](#codegen-impact-matrix)
9. [Chapter Summary](#chapter-summary)

---

## 1. The JOCKY Annotation Architecture

In JOCKY, annotations are metadata specifiers prefixed with the `@` character that attach directly to function definitions, struct declarations, or method receivers.

Annotations serve three critical functions:
1. **Compile-Time Constraints:** Directing the compiler to conditionally compile, inline, or emit deprecation diagnostics.
2. **Runtime Security Assertions:** Injecting automatic privilege verification and OS token assertion routines into the function preamble.
3. **Control-Flow Hinting:** Informing the static analyzer and CFG flattening engine about non-returning functions or specialized inlining targets.

```
+-----------------------------------------------------------------------------+
|                          Annotation Structure                               |
+-----------------------------------------------------------------------------+
|   @annotation_name ( [ optional_arguments ] )                               |
|   fn target_function() -> ReturnType { ... }                                |
+-----------------------------------------------------------------------------+
```

---

## 2. Security & Privileges: `@privileged`

Forensic functions that inspect arbitrary process memory, dump kernel telemetry, or open raw physical drive handles require elevated operating system credentials.

The `@privileged` annotation marks a function as requiring elevated administrative or root privileges:

```jocky
@privileged
fn dump_lsass_memory(int lsass_pid) -> (bytes, Error) {
    log.info("Acquiring LSASS process memory handles...");
    return process.read_memory(lsass_pid, 0x00010000, 65536);
}
```

---

### Runtime Verification Mechanism

When the JOCKY code generator lowers a `@privileged` function to C, it automatically wraps the function entry point with an embedded security assertion preamble:

```c
// Injected C runtime preamble for @privileged:
void _jky_fn_dump_lsass_memory(int64_t lsass_pid) {
    if (!jky_runtime_assert_privilege()) {
        jky_panic("FATAL: Function 'dump_lsass_memory' requires elevated privileges (SYSTEM/root).");
        return;
    }
    // Original function body executes here...
}
```

### Windows Token Escalation (`SeDebugPrivilege`)
On Windows targets, `jky_runtime_assert_privilege()`:
1. Opens the current process access token via `OpenProcessToken`.
2. Inspects `TOKEN_PRIVILEGES` for `SeDebugPrivilege`.
3. Automatically enables `SeDebugPrivilege` and `SeSecurityPrivilege` if available.
4. If running as a standard unelevated user, fails gracefully before triggering an AV alert.

### Linux Capability Checks (`CAP_SYS_ADMIN`, `CAP_SYS_PTRACE`)
On Linux targets, the runtime checks if the effective user ID is `0` (root) or verifies if `cap_get_proc()` contains `CAP_SYS_ADMIN` and `CAP_SYS_PTRACE`.

---

## 3. Target Portability: `@platform`

Different operating systems expose distinct forensic artifacts. For example, Windows utilizes Registry hives and Prefetch files, while Linux utilizes `/proc`, `sysfs`, and `systemd` journal logs.

The `@platform("target")` annotation restricts a function to a specific operating system target:

```jocky
@platform("windows")
fn inspect_windows_registry() -> map {
    log.info("Scanning HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Run...");
    return host.registry_autoruns();
}

@platform("linux")
fn inspect_proc_maps(int pid) -> list {
    log.info("Reading /proc/" + (string)pid + "/maps...");
    return fs.read_lines("/proc/" + (string)pid + "/maps");
}
```

---

### Platform Guard Semantics

During compilation:
1. If the current `--target` matches the argument in `@platform("windows")` (e.g. `x86_64-w64-mingw32`), the function compiles normally.
2. If the current `--target` does **not** match (e.g. building for Linux), the compiler strips the function body and replaces it with a stub that returns `JKY_ERR("Function unsupported on target platform")`.

---

## 4. Lifecycle Management: `@deprecated`

As forensic toolchains evolve, specific collection routines are superseded by more stealthy or comprehensive methods.

The `@deprecated("message")` annotation causes the compiler to emit a diagnostic warning whenever the annotated function or struct is referenced:

```jocky
@deprecated("Use process.find_safe() to avoid EDR hook detection")
fn find_process_legacy(string name) -> ProcessInfo {
    return process.find(name);
}

fn main() -> void {
    // Triggers compiler warning:
    // [WARN] Line 10: Call to deprecated function 'find_process_legacy'. Use process.find_safe() to avoid EDR hook detection
    auto p = find_process_legacy("lsass.exe");
}
```

---

## 5. Optimization & Obfuscation: `@inline`

The `@inline` directive instructs the code generator and backend compiler to eliminate function call overhead by copying the function's body directly into every call site:

```jocky
@inline
fn rotate_left_64(int val, int shift) -> int {
    return (val << shift) | (val >> (64 - shift));
}
```

### Stealth Benefit of Inlining:
In standard binaries, function call sequences (`call target_func`) create visible, easily fingerprinted call-graph structures in reverse engineering tools. By applying `@inline` to critical collection logic, the call graph is completely dissolved into linear machine code.

---

## 6. Control Flow Finality: `@noreturn`

The `@noreturn` annotation informs the semantic analyzer that a function terminates the process or never returns execution to the caller:

```jocky
@noreturn
fn panic_and_purge(string reason) -> void {
    log.error("CRITICAL PURGE: " + reason);
    // Overwrite memory buffers
    crypto.wipe_ephemeral_keys();
    // Immediate hard exit
    host.exit(139);
}
```

### Compiler Effect:
The compiler's unreachable code analyzer recognizes that any statements placed immediately following a call to a `@noreturn` function will never execute, preventing false-positive "missing return statement" errors.

---

## 7. Combining Multiple Annotations

Multiple annotations can be stacked on a single declaration:

```jocky
@privileged
@platform("windows")
@inline
fn direct_kernel_ioctl_read(int device_handle, int ioctl_code) -> bytes {
    // Direct device I/O control read on Windows with elevated privileges
    return host.raw_ioctl(device_handle, ioctl_code);
}
```

---

## 8. Codegen Impact Matrix

| Annotation | Compile-Time Impact | Runtime / Codegen Impact |
| :--- | :--- | :--- |
| **`@privileged`** | None | Injects token/capability verification preamble into function entry |
| **`@platform("os")`** | Conditionally compiles or generates unsupported error stubs | Eliminates incompatible OS code from target binary |
| **`@deprecated("msg")`** | Emits diagnostic warning at all call sites | None |
| **`@inline`** | Replaces function calls with inlined AST statements | Eliminates call instruction in native assembly |
| **`@noreturn`** | Adjusts control-flow reachability analysis | Emits C `__attribute__((noreturn))` or `__declspec(noreturn)` |

---

## 9. Chapter Summary

- **`@privileged`:** Guards sensitive functions with automatic OS token privilege assertions.
- **`@platform`:** Enables clean multi-platform source code by guarding OS-specific features.
- **`@deprecated`:** Guides developers away from outdated forensic routines.
- **`@inline`:** Inlines critical functions to dissolve call-graph signatures.
- **`@noreturn`:** Marks fatal exit routines to optimize compiler flow analysis.

In the next chapter, **[Chapter 15: Standard Library Reference](ch15_stdlib_reference.md)**, we provide the complete, exhaustive reference for all eight standard JOCKY packages.
