# The JOCKY Language Book

> **The Authoritative Reference for the JOCKY Programming Language and Toolchain**  
> *Built for Covert Digital Forensics and Incident Response*

---

## Welcome to JOCKY

**JOCKY** is a statically typed, compiled domain-specific systems programming language engineered specifically for digital forensics, incident response (DFIR), and threat hunting in high-security, adversary-contested computing environments.

In modern cyber operations, investigators and forensic analysts face a critical dilemma known as the **Forensic Observer Effect**: the very tools used to collect evidence—such as PowerShell scripts, Python interpreters, or standard C/C++ collection binaries—exhibit telemetry, memory signatures, and API call sequences that mirror adversarial post-exploitation activity. Consequently, modern Endpoint Detection and Response (EDR) agents and Antivirus (AV) suites terminate collection processes, corrupt volatile state, generate alerts that tip off adversaries, or outright quarantine forensic utilities.

JOCKY resolves this crisis through a foundational paradigm shift: **the compiler is the first line of forensic evasion and integrity**. JOCKY source files (`.jky`) are compiled by the `jky` toolchain into highly optimized, stripped, position-independent native binaries through an in-memory compilation pipeline. The compiler applies automated compile-time polymorphic mutations, control-flow flattening, per-site string encryption, symbol salting, and stack layout randomization, ensuring that no two compiled instances of the same collection script ever share static signatures or predictable binary footprints.

```jocky
import host;
import process;
import evidence;
import log;

fn main() -> void {
    log.info("Commencing triage sequence...");

    // Initialize cryptographically sealed forensic evidence bundle
    auto case_file, err = evidence.open("CASE-2026-NTRO-884");
    if err != nil {
        log.error("Failed to initialize evidence container: " + err.message);
        return;
    }

    // Collect volatile host telemetries
    auto sys_info = host.info();
    evidence.add(case_file, "host_info", sys_info);

    // Enumerate active processes and inspect anomalies
    auto procs = process.list();
    evidence.add(case_file, "process_inventory", procs);

    for proc in procs {
        if proc.is_elevated && proc.parent_pid == 1 {
            log.warn("Anomalous daemon detected: " + proc.name);
        }
    }

    // Seal evidence bundle with HMAC integrity proof
    case_file.seal();
    log.info("Triage complete. Evidence container sealed.");
}
```

---

## Table of Contents & Chapter Index

This book serves as the comprehensive, normative reference for the JOCKY language specification, standard library, compiler architecture, runtime system, stealth sub-systems, and forensic analysis workflows.

### Part I: Foundation & Getting Started
- **[Chapter 1: Introduction to JOCKY](ch01_introduction.md)** — The modern forensic dilemma, the observer effect in cybersecurity, why standard tooling (Python/PowerShell/C) fails, NTRO operational context, and core architectural goals.
- **[Chapter 2: Design Philosophy & Guiding Principles](ch02_philosophy.md)** — The compiler as an evasion engine, static typing rationale, Result tuples over exceptions, forensic immutability, zero-dependency deployment, and compile-time entropy.
- **[Chapter 3: Installation & Environment Setup](ch03_installation.md)** — Prerequisites, toolchain setup, compiling the `jky` compiler from source on Linux and Windows, build flags, Docker environments, and validation.
- **[Chapter 4: Quick Start: From Zero to Sealed Evidence](ch04_quick_start.md)** — Hello world, script mode vs. agent mode, building your first forensic collector, verifying build entropy across identical compilations, and inspecting evidence seals.

### Part II: The JOCKY Language Specification
- **[Chapter 5: Language Basics & Lexical Structure](ch05_language_basics.md)** — Tokens, mandatory semicolon mechanics, whitespace rules, comment syntax, identifier conventions, source file organization, script mode vs. agent mode.
- **[Chapter 6: Type System](ch06_types.md)** — Complete primitive types (`void`, `int`, `float`, `bool`, `byte`, `bytes`, `string`, `list`, `map`, `Error`), memory representations, nil semantics, explicit casting rules, and conversions.
- **[Chapter 7: Variables, Constants, and Scoping](ch07_variables_and_scope.md)** — Static variable declarations, `auto` type inference, `const` immutability, lexical block scopes, variable shadowing, the `_` blank identifier, memory lifetimes, and GC integration.
- **[Chapter 8: Functions & Method Receivers](ch08_functions.md)** — Function signatures, multi-value returns, error tuple returns, method receiver syntax, two-pass symbol resolution, and recursion mechanics.
- **[Chapter 9: Structs & Data Modeling](ch09_structs_and_methods.md)** — Struct definitions, field initialization, method attachment, nesting, reference semantics, JSON serialization, and comparison with Go/Rust/C.
- **[Chapter 10: Control Flow](ch10_control_flow.md)** — `if`/`else` branching (parentheses-free), `while` loops, C-style indexed `for` loops, collection iteration `for..in`, loop control statements, and ternary expressions.
- **[Chapter 11: Error Handling & Propagation](ch11_error_handling.md)** — The Result pattern, `(T, Error)` multiple return pairs, `nil` error semantics, the `!` try-propagation operator, error chaining, and panic conditions.
- **[Chapter 12: Modules, Packages, and Source Organization](ch12_modules_and_packages.md)** — Directory-as-package architecture, `import` semantics, multi-file compilation units, circular import prevention, and stdlib resolution.
- **[Chapter 13: Operators & Expressions](ch13_operators_and_expressions.md)** — Operator hierarchy, full precedence and associativity tables, the strict bitwise/logical disambiguation compile-time rule, and explicit type casting.
- **[Chapter 14: Annotations & Compiler Directives](ch14_annotations.md)** — Annotation system: `@privileged`, `@platform`, `@deprecated`, `@inline`, `@noreturn`, and their codegen effects.

### Part III: Standard Library & Runtime System
- **[Chapter 15: Standard Library Reference](ch15_stdlib_reference.md)** — Complete, exhaustive reference for all standard modules: `host`, `process`, `network`, `fs`, `evidence`, `report`, `crypto`, and `log`.
- **[Chapter 19: Runtime Internals & Memory Management](ch19_runtime_internals.md)** — `JkyVal` tagged union representation, mark-and-sweep garbage collector, string and byte slice management, evidence packaging format, and OS abstractions.

### Part IV: Compiler Architecture & Stealth Engineering
- **[Chapter 16: Compiler Internals & Code Generation](ch16_compiler_internals.md)** — Lexer design, recursive descent parser, arena-allocated Abstract Syntax Tree (AST), two-pass symbol table resolver, C emitter, and memory-to-memory compiler piping.
- **[Chapter 17: The Stealth Sub-System](ch17_stealth_system.md)** — The 6 core evasion engines: BLAKE2b-seeded string encryption, symbol salting, dead code injection, control-flow flattening, build salting, and stack layout randomization.

### Part V: Practical Operations & Reference
- **[Chapter 18: Real-World Forensic Workflows](ch18_forensic_workflows.md)** — 6 complete end-to-end incident response scripts: host triage, process anomaly detection, network exfiltration hunting, filesystem artifact recovery, module auditing, and comprehensive multi-artifact evidence packaging.
- **[Chapter 20: Toolchain CLI & Compilation Reference](ch20_toolchain_reference.md)** — Command-line interface reference for `jky compile`, `jky run`, `jky check`, `jky build`, `jky fmt`, flags, cross-compilation target triples, and exit codes.
- **[Chapter 21: Compiler Diagnostics & Error Codes](ch21_error_codes.md)** — Exhaustive directory of all compiler diagnostic codes (`E0001` through `E0015`), root causes, problematic code patterns, and remediation guides.
- **[Chapter 22: Future Architecture & Roadmap](ch22_roadmap.md)** — Version 0.1 through Version 1.0+ vision: LLVM IR direct emission, generic type parameters, closures, kernel driver interfaces (BYOVD detection/acquisition), agent beacons, and self-hosting bootstrap.

### Part VI: Appendices
- **[Appendix A: Formal Language Grammar (EBNF)](appendix_a_grammar.md)** — Complete Extended Backus-Naur Form grammar specification for JOCKY.
- **[Appendix B: Keywords & Operator Table](appendix_b_keywords.md)** — Exhaustive lexical keyword list, operator precedence matrices, and syntactic symbols.
- **[Appendix C: Standard Library Quick Reference](appendix_c_stdlib_quick_ref.md)** — Compact lookup tables for all standard library functions, method receivers, parameters, and signatures.

---

## Language Specification Summary

| Attribute | Specification |
| :--- | :--- |
| **Language Name** | JOCKY |
| **File Extension** | `.jky` |
| **CLI Toolchain** | `jky` (`jky compile`, `jky run`, `jky check`, `jky fmt`) |
| **Type System** | Static, strong, explicit (with `auto` local inference), no implicit coercions |
| **Primitives** | `void`, `int` (64-bit signed), `float` (64-bit IEEE 754), `bool`, `byte` (u8), `bytes` (blob), `string` (UTF-8 immutable), `list` (untyped v0.1), `map` (untyped v0.1) |
| **Memory Model** | Automatic heap management via runtime Mark-and-Sweep Garbage Collector; arena-allocated compilation |
| **Error Model** | Explicit multiple return values `(T, Error)` and `!` propagation operator. No exceptions |
| **Control Flow** | `if`/`else` (no parens), `while`, `for item in coll`, `for init; cond; post`, ternary `a ? b : c` |
| **Stealth Engine** | 6-layer compile-time polymorphism: String XOR, Symbol Salting, Dead Code, CFG Flattening, Build Salt, Stack Randomization |
| **Compilation Pipeline** | `.jky` $\rightarrow$ Memory AST $\rightarrow$ Memory C Code $\rightarrow$ GCC/Clang stdin (`-nostdlib`) $\rightarrow$ Native Binary |
| **Target Organization** | National Technical Research Organisation (NTRO), Cyber Intelligence & Forensics |

---

## How to Read This Book

1. **For Forensic Analysts & Incident Responders:**
   Start with [Chapter 1 (Introduction)](ch01_introduction.md), [Chapter 4 (Quick Start)](ch04_quick_start.md), and [Chapter 18 (Real-World Forensic Workflows)](ch18_forensic_workflows.md) to understand how JOCKY executes high-speed, stealthy collection routines without triggering security sensors. Keep [Chapter 15 (Standard Library Reference)](ch15_stdlib_reference.md) and [Appendix C (Quick Ref)](appendix_c_stdlib_quick_ref.md) open as daily workbench references.

2. **For Systems Programmers & Tool Authors:**
   Read [Chapter 5 through Chapter 14](ch05_language_basics.md) sequentially to master JOCKY syntax, type semantics, error handling, method receivers, and annotations.

3. **For Security Engineers & Compiler Enthusiasts:**
   Examine [Chapter 16 (Compiler Internals)](ch16_compiler_internals.md), [Chapter 17 (The Stealth Sub-System)](ch17_stealth_system.md), and [Chapter 19 (Runtime Internals)](ch19_runtime_internals.md) for an in-depth breakdown of how in-memory compilation, code obfuscation, and runtime data structures function under the hood.

---

> [!IMPORTANT]
> The JOCKY programming language and its standard forensic libraries are engineered for authorized forensic investigations, internal incident response operations, and security research under appropriate legal and organizational mandates.
