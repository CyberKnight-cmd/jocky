# Chapter 4: Quick Start: From Zero to Sealed Evidence

> *"The fastest way to understand JOCKY is to write a script, compile it twice, observe the mutating binary entropy, and inspect the resulting cryptographic evidence seal."*

---

## Table of Contents
1. [Your First JOCKY Script: Hello, Operator](#your-first-jocky-script-hello-operator)
2. [Script Mode vs. Agent Mode](#script-mode-vs-agent-mode)
3. [Your First Forensic Collection Agent](#your-first-forensic-collection-agent)
4. [Executing with `jky run`](#executing-with-jky-run)
5. [Compiling to Native Binary with `jky compile`](#compiling-to-native-binary-with-jky-compile)
6. [Demonstrating Compile-Time Polymorphism](#demonstrating-compile-time-polymorphism)
7. [Inspecting the Sealed Evidence Container](#inspecting-the-sealed-evidence-container)
8. [Debugging and Verbose Compilation](#debugging-and-verbose-compilation)
9. [Chapter Summary](#chapter-summary)

---

## 1. Your First JOCKY Script: Hello, Operator

Let us begin with the simplest possible JOCKY program. Create a new file named `hello.jky` in your working directory.

```jocky
// hello.jky - The fundamental JOCKY introductory script
import log;

fn main() -> void {
    log.info("NTRO Sovereign Cyber Forensics Engine Initialized.");
}
```

### Key Observations:
- **`import log;`** imports the built-in standard logging package.
- **`fn main() -> void`** defines the program entry point in Agent Mode. Notice the explicit return type `-> void`.
- **Mandatory Semicolons:** Every statement must terminate with a semicolon (`;`).

---

## 2. Script Mode vs. Agent Mode

JOCKY operates in two distinct execution modes depending on operational requirements:

```
+-----------------------------------------------------------------------------+
|                             JOCKY EXECUTION MODES                           |
+-----------------------------------------------------------------------------+
| 1. SCRIPT MODE (jky run script.jky)                                         |
|    - For interactive debugging, local script verification, and rapid triage.|
|    - Top-level statements allowed directly without an enclosing fn main().  |
|    - Compiles and executes in-memory on the local analyst workstation.      |
|                                                                             |
| 2. AGENT MODE (jky compile agent.jky -o agent.exe)                          |
|    - For operational deployment to contested target endpoints.              |
|    - Explicit 'fn main() -> void' entry point is strictly MANDATORY.        |
|    - Full 6-layer polymorphic stealth engine enabled by default.            |
|    - Emits a standalone, stripped native binary.                            |
+-----------------------------------------------------------------------------+
```

### Script Mode Example (`quick_test.jky`):
```jocky
// Top-level statements directly in the file
import host;
import log;

auto info = host.info();
log.info("Executing on host: " + info.hostname + " (" + info.os + ")");
```

Execute immediately with:
```bash
jky run quick_test.jky
```

Output:
```
[INFO] [2026-08-24T02:15:00Z] Executing on host: DESKTOP-FORENSIC (windows)
```

---

## 3. Your First Forensic Collection Agent

Now let us build a complete, real-world forensic triage agent. This program will:
1. Initialize a cryptographically bound `CaseFile` evidence bundle.
2. Collect host telemetry (hostname, operating system, kernel architecture, uptime).
3. Enumerate all running processes.
4. Detect suspicious or elevated processes.
5. Append all data to the evidence container.
6. Seal the container with an HMAC-SHA256 integrity signature.

Create `triage_demo.jky`:

```jocky
// triage_demo.jky - High-Speed Covert Triage Agent
import host;
import process;
import evidence;
import log;

fn main() -> void {
    log.info("Starting live endpoint triage sequence...");

    // 1. Initialize case evidence container
    auto case_bundle, err = evidence.open("CASE-2026-ALPHA-01");
    if err != nil {
        log.error("Fatal: Could not initialize evidence container: " + err.message);
        return;
    }

    // 2. Gather host telemetry
    log.info("Collecting host telemetry...");
    auto host_telemetry = host.info();
    evidence.add(case_bundle, "host_info", host_telemetry);

    // 3. Gather process listing
    log.info("Enumerating active processes...");
    auto procs = process.list();
    evidence.add(case_bundle, "process_table", procs);

    // 4. Perform in-memory heuristics
    int suspicious_count = 0;
    for p in procs {
        if p.is_elevated && p.parent_pid == 1 {
            log.warn("Anomalous daemon detected: " + p.name + " (PID: " + (string)p.pid + ")");
            suspicious_count++;
        }
    }

    log.info("Process inspection complete. Anomalies flagged: " + (string)suspicious_count);

    // 5. Seal the evidence container
    case_bundle.seal();
    log.info("Evidence container successfully sealed with HMAC signature.");

    // 6. Export the evidence bundle to disk
    auto export_err = evidence.export(case_bundle, "./evidence_bundle");
    if export_err != nil {
        log.error("Failed to export evidence bundle: " + export_err.message);
        return;
    }

    log.info("Triage operation concluded successfully.");
}
```

---

## 4. Executing with `jky run`

To test the script directly on your local workstation without producing a persistent binary artifact, use the `run` subcommand:

```bash
jky run triage_demo.jky
```

### Terminal Output:

```
[INFO] [2026-08-24T02:16:10Z] Starting live endpoint triage sequence...
[INFO] [2026-08-24T02:16:10Z] Collecting host telemetry...
[INFO] [2026-08-24T02:16:10Z] Enumerating active processes...
[WARN] [2026-08-24T02:16:10Z] Anomalous daemon detected: svchost_fake.exe (PID: 4912)
[INFO] [2026-08-24T02:16:10Z] Process inspection complete. Anomalies flagged: 1
[INFO] [2026-08-24T02:16:10Z] Evidence container successfully sealed with HMAC signature.
[INFO] [2026-08-24T02:16:10Z] Triage operation concluded successfully.
```

---

## 5. Compiling to Native Binary with `jky compile`

To generate a deployable, standalone native executable, use the `compile` subcommand:

```bash
jky compile triage_demo.jky -o triage_agent.exe
```

```
[+] Parsing triage_demo.jky...
[+] AST lower and semantic analysis complete.
[+] Applying 6-layer stealth mutations (Salt: a9f810e7b42c9081...)...
[+] Generating in-memory C compilation unit...
[+] Invoking backend native compiler pipeline via STDIN pipe...
[+] Successfully compiled native forensic agent: triage_agent.exe (294,400 bytes)
```

The resulting `triage_agent.exe` is completely freestanding. It contains no debug symbols, no plaintext strings, and no references to `triage_demo.jky`.

---

## 6. Demonstrating Compile-Time Polymorphism

Now let us demonstrate the most powerful feature of the JOCKY compiler: **Automated Compile-Time Polymorphism**.

Compile the exact same source file (`triage_demo.jky`) two consecutive times into two different output files:

```bash
# Compilation 1
jky compile triage_demo.jky -o agent_build_1.exe

# Compilation 2
jky compile triage_demo.jky -o agent_build_2.exe
```

Now compare the cryptographic SHA-256 hashes of both generated binaries:

### On Linux:
```bash
sha256sum agent_build_1.exe agent_build_2.exe
```

### On Windows PowerShell:
```powershell
Get-FileHash agent_build_1.exe, agent_build_2.exe | Format-Table Algorithm, Hash, Path
```

### Output:

```
Algorithm Hash                                                             Path
--------- ----                                                             ----
SHA256    D8E9F108C751BA825F4C7581B98AE92B104F5E602A9801BC3829140B8F9A1054 agent_build_1.exe
SHA256    3F4A7B88C025E6A77128DF9B01456E99AA120B876C449E2D808162A5E12E9901 agent_build_2.exe
```

### What Just Happened?
Although the source code was 100% identical:
1. **Build 1** generated a random 32-byte salt `0x94A1...`, derived unique XOR encryption keys for all string literals (e.g. `"CASE-2026-ALPHA-01"`), renamed internal C functions to `_jky_fn_94a1_main`, and inserted 3 synthetic opaque predicate dead-code blocks.
2. **Build 2** generated a completely different salt `0x7B2F...`, resulting in completely different XOR keys, completely different symbol hashes, flattened the control-flow graph with different state switch constants, and inserted 2 distinct dead-code blocks.

> [!TIP]
> If you require deterministic, reproducible builds for formal lab validation, you can explicitly specify the build salt using the `--salt` flag:
> ```bash
> jky compile triage_demo.jky -o agent_fixed.exe --salt 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
> ```

---

## 7. Inspecting the Sealed Evidence Container

When our triage script executed, it produced an evidence bundle at `./evidence_bundle/`. Let us inspect the internal structure of this directory:

```
evidence_bundle/
├── manifest.json       # Metadata, artifact index, and individual SHA-256 hashes
├── host_info.json      # Structured JSON table of host telemetry
├── process_table.json  # Complete snapshot of running processes
├── sha256sums.txt      # Standard unix-compatible sha256 checksum list
└── evidence.seal       # Cryptographic HMAC-SHA256 signature seal
```

### 1. `manifest.json`
```json
{
  "case_id": "CASE-2026-ALPHA-01",
  "sealed_at": "2026-08-24T02:16:10Z",
  "collector": "JOCKY v0.1.0-alpha",
  "build_salt": "a9f810e7b42c9081...",
  "host_hardware_id": "e9b204f8-1123-4819-b681-90cfa82138a1",
  "artifacts": [
    {
      "name": "host_info",
      "filename": "host_info.json",
      "sha256": "4b825dc642cb6eb9a060e54b3c579e0f007157a90c3237302c0619930f836611",
      "size_bytes": 348
    },
    {
      "name": "process_table",
      "filename": "process_table.json",
      "sha256": "18a9fc4c7711200b8e72c0d9a65f8841029ab0e922c104e7b8921dc807494532",
      "size_bytes": 14208
    }
  ]
}
```

### 2. `evidence.seal`
The `evidence.seal` file contains the raw HMAC-SHA256 calculated over the canonical bytes of `manifest.json`. Any modification to `host_info.json`, `process_table.json`, or `manifest.json` invalidates the seal immediately upon forensic verification.

---

## 8. Debugging and Verbose Compilation

During script development and testing on your analyst workstation, you can inspect the intermediate C code and AST transformations by supplying the `--debug` flag:

```bash
jky compile triage_demo.jky -o triage_agent.exe --debug
```

### Debug Output:
```
[DEBUG] Lexer token count: 184
[DEBUG] Parser AST node count: 72
[DEBUG] Symbol Table Dump:
  - [Module] host (builtin)
  - [Module] process (builtin)
  - [Module] evidence (builtin)
  - [Module] log (builtin)
  - [Function] main() -> void
  - [Local] case_bundle: CaseFile
  - [Local] err: Error
  - [Local] host_telemetry: map
  - [Local] procs: list
  - [Local] suspicious_count: int
[DEBUG] Stealth Passes Applied:
  - String literals encrypted: 8
  - Control flow blocks flattened: 2
  - Dead code blocks synthesized: 3
  - Local stack padding: 48 bytes
[DEBUG] Generated C Source Size: 24,192 bytes
[DEBUG] Native Compiler Command: gcc -O2 -nostdlib -fno-ident -x c - -o triage_agent.exe
```

> [!NOTE]
> In `--debug` mode, the compiler prints intermediate diagnostics to stderr, but still adheres to the strict rule of never writing intermediate `.c` source files to disk.

---

## 9. Chapter Summary

- **Script vs. Agent Mode:** Use `jky run` for instant script-mode execution; use `jky compile` for production agent deployment with mandatory `fn main() -> void`.
- **First Forensic Workflow:** Initializing, appending, and sealing a `CaseFile` is accomplished in fewer than twenty lines of code.
- **Polymorphic Verification:** Two consecutive builds of identical source code yield completely distinct cryptographic binary hashes.
- **Evidence Immutability:** Sealing an evidence container creates a cryptographically auditable manifest with HMAC integrity proofs.

In the next chapter, **[Chapter 5: Language Basics & Lexical Structure](ch05_language_basics.md)**, we begin our deep dive into the formal grammar, lexical tokens, and syntactic rules of the JOCKY language.
