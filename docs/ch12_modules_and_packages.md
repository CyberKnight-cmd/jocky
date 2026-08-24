# Chapter 12: Modules, Packages, and Source Organization

> *"Modularity is the antidote to code entropy. In JOCKY, package boundaries are strictly deterministic, imports are fully namespaced, and circular dependencies are eliminated at the compiler frontend."*

---

## Table of Contents
1. [The Package Architecture: Directory Equals Package](#the-package-architecture-directory-equals-package)
2. [The `import` Statement & Syntax](#the-import-statement--syntax)
3. [Namespaced Member Access](#namespaced-member-access)
4. [Multi-File Packages & Shared Lexical Scope](#multi-file-packages--shared-lexical-scope)
5. [The Built-in Standard Library vs. User Packages](#the-built-in-standard-library-vs-user-packages)
6. [Strict Prohibition of Wildcard Imports (`import *`)](#strict-prohibition-of-wildcard-imports-import-)
7. [Circular Import Detection & Resolution](#circular-import-detection--resolution)
8. [Package Visibility & Export Rules](#package-visibility--export-rules)
9. [Structuring Real-World Forensic Projects](#structuring-real-world-forensic-projects)
10. [Chapter Summary](#chapter-summary)

---

## 1. The Package Architecture: Directory Equals Package

In JOCKY, **a directory on the filesystem defines a single logical package**.

```
my_forensic_suite/
├── main.jky                    # Package 'main'
├── collectors/                 # Package 'collectors'
│   ├── memory.jky
│   ├── disk.jky
│   └── network.jky
└── analysis/                   # Package 'analysis'
    ├── heuristics.jky
    └── yara_scanner.jky
```

### Core Architectural Rules:
1. **One Package Per Directory:** All `.jky` source files residing in the same directory belong to the same package and share declarations without requiring explicit imports.
2. **Directory Name as Package Identifier:** The package name is derived from the directory name (or `main` for the project root).
3. **No Nested Package Conflicts:** Subdirectories represent distinct nested packages (e.g. `analysis.yara_scanner`).

---

## 2. The `import` Statement & Syntax

To access functions, structs, and constants declared in external packages or the built-in standard library, files must declare their dependencies using the `import` keyword at the top of the file:

```jocky
// Importing built-in standard library packages:
import host;
import process;
import evidence;
import log;

// Importing user-defined subpackages:
import collectors.memory;
import analysis.heuristics;
```

### Import Rules:
- All `import` statements must appear at the top of the file before any struct, constant, or function definitions.
- Every `import` statement must terminate with a mandatory semicolon (`;`).
- Duplicate imports within the same file generate a compiler warning or are silently deduplicated.

---

## 3. Namespaced Member Access

JOCKY mandates **fully qualified namespaced access** for all imported symbols. When a package is imported, its exported functions and types are accessed exclusively via `packagename.symbol`:

```jocky
import host;
import process;
import crypto;

fn main() -> void {
    // Correct: Fully namespaced calls
    auto sys_info = host.info();
    auto procs = process.list();
    bytes digest = crypto.sha256(sys_info.json());

    log.info("System hash: " + digest.hex());
}
```

```
+-----------------------------------------------------------------------------+
|                          Namespaced Access Example                          |
+-----------------------------------------------------------------------------+
|   import process;                                                           |
|             \                                                               |
|              +--->  process.list()                                          |
|                     \_____/ \____/                                          |
|                    Namespace Symbol                                         |
+-----------------------------------------------------------------------------+
```

---

## 4. Multi-File Packages & Shared Lexical Scope

When a package grows large, its declarations should be divided into multiple cohesive `.jky` files within the same folder.

All files within the same directory share a single package namespace. **Functions, structs, and constants in one file are directly accessible in all other files of that directory without importing**:

```
collectors/
├── process_collector.jky   # Declares: fn collect_processes() -> list
└── network_collector.jky   # Declares: fn collect_sockets() -> list
```

### `process_collector.jky`:
```jocky
// Inside package 'collectors'
fn collect_processes() -> list {
    return process.list();
}
```

### `network_collector.jky`:
```jocky
// Inside package 'collectors'
fn collect_all_telemetry() -> map {
    // Directly calls collect_processes() without import!
    auto procs = collect_processes();
    auto conns = network.connections();

    return {
        "processes": procs,
        "connections": conns
    };
}
```

When invoking `jky build`, the compiler gathers all `.jky` files in the directory, parses them into a unified package AST, and performs semantic analysis across all files simultaneously.

---

## 5. The Built-in Standard Library vs. User Packages

JOCKY's built-in standard library packages are embedded directly inside the compiler binary. They do not depend on external `.jky` files on the host filesystem:

```
+------------------------------------------------------------------------+
|                      Module Resolution Strategy                        |
+------------------------------------------------------------------------+
| 1. Is 'import name' in the Built-in Stdlib Registry?                   |
|    -> Yes: Bind to embedded native C runtime definitions               |
|            (e.g. host, process, network, fs, evidence, crypto, log)    |
|                                                                        |
| 2. Is 'import name' a relative local directory?                        |
|    -> Yes: Scan local directory for all *.jky source files             |
|                                                                        |
| 3. Otherwise:                                                          |
|    -> Raise Compiler Error: E0015: Cannot resolve module 'name'        |
+------------------------------------------------------------------------+
```

---

## 6. Strict Prohibition of Wildcard Imports (`import *`)

In languages like Python (`from math import *`) or Java (`import java.util.*`), wildcard imports dump hundreds of symbols into the global namespace.

In JOCKY, **wildcard imports (`import *`) are strictly forbidden**:

```jocky
// ILLEGAL IN JOCKY:
// import process.*; // COMPILE ERROR: E0002: Wildcard imports not permitted

// CORRECT:
import process;
auto procs = process.list();
```

### Rationale:
1. **Namespace Pollution:** Wildcard imports lead to silent symbol collisions (e.g. `process.open()` vs. `evidence.open()` vs. `fs.open()`).
2. **Auditability:** A security auditor reviewing a forensic triage script must immediately know which package provides every function call.
3. **Dead-Code Elimination:** Fully namespaced imports allow the compiler to accurately strip unused runtime functions from the compiled binary.

---

## 7. Circular Import Detection & Resolution

A circular import occurs when Package A imports Package B, and Package B directly or indirectly imports Package A:

```
+-----------------------------------------------------------------------------+
|                             Circular Import Deadlock                        |
+-----------------------------------------------------------------------------+
|   [ Package collectors ] ------------------------> [ Package analyzer ]     |
|          ^                                                   |              |
|          +---------------------------------------------------+              |
|                     (FATAL COMPILER ERROR: E0003)                           |
+-----------------------------------------------------------------------------+
```

JOCKY's package manager constructs a Directed Acyclic Graph (DAG) of all imports during semantic pass 1. If a cycle is detected, the compiler immediately halts with error `E0003`:

```
[ERROR] E0003: Circular import dependency detected:
  -> package 'collectors' [collectors/memory.jky:2]
  -> package 'analyzer'   [analyzer/rules.jky:1]
  -> package 'collectors'
Compilation aborted.
```

### Resolution Strategy:
Refactor shared data models or structs into a separate leaf package (e.g. `types/` or `models/`) that both packages can import independently.

---

## 8. Package Visibility & Export Rules

In JOCKY v0.1:
- All top-level functions, structs, and constants declared within a package are exported and accessible to any package that imports them.
- Identifiers starting with a leading underscore (e.g. `_internal_helper`) are reserved for internal compiler/runtime use and cannot be declared in user code.

---

## 9. Structuring Real-World Forensic Projects

Here is the standard, production-grade project structure for a complete JOCKY forensic collection suite:

```
triage_suite/
├── main.jky                     # CLI Entry point & orchestrator
├── config/
│   └── settings.jky             # Investigation constants and flags
├── collectors/
│   ├── host_collector.jky       # Host telemetries
│   ├── proc_collector.jky       # Process listings and thread audits
│   └── net_collector.jky        # Socket tables and interface states
└── reporters/
    └── evidence_bundler.jky     # Evidence container creation & sealing
```

### `reporters/evidence_bundler.jky`:
```jocky
import evidence;
import log;

struct TriageBundle {
    string case_id;
    CaseFile container;
}

fn initialize_bundle(string case_id) -> (TriageBundle, Error) {
    auto cf, err = evidence.open(case_id);
    if err != nil {
        return nil, err;
    }

    TriageBundle tb = TriageBundle {
        case_id: case_id,
        container: cf
    };

    return tb, nil;
}

fn (TriageBundle tb) record_artifact(string name, auto data) -> void {
    evidence.add(tb.container, name, data);
}

fn (TriageBundle tb) finalize() -> void {
    tb.container.seal();
    log.info("Bundle " + tb.case_id + " sealed.");
}
```

---

## 10. Chapter Summary

- **Directory as Package:** Every directory represents a single package; all `.jky` files within that directory share a common namespace.
- **Explicit Imports:** Dependencies are imported using `import package;`.
- **Namespaced Calls:** All external symbols must be called with their package prefix (`process.list()`).
- **No Wildcards:** `import *` is strictly forbidden to prevent namespace collisions.
- **DAG Enforcement:** Circular imports are detected during AST construction and rejected immediately (`E0003`).

In the next chapter, **[Chapter 13: Operators & Expressions](ch13_operators_and_expressions.md)**, we examine operator precedence, associativity, casting syntax, and the strict compile-time bitwise disambiguation rule.
