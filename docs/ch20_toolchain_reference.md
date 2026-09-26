# Chapter 20: Toolchain CLI & Compilation Reference

> *"A professional toolchain must be predictable, scriptable, and transparent. The `jky` command-line interface provides comprehensive control over parsing, static analysis, polymorphic mutation, cross-compilation, and execution."*

---

## Table of Contents
1. [CLI Architecture & Overview](#cli-architecture--overview)
2. [Command Reference](#command-reference)
   - [`jky compile`](#jky-compile)
   - [`jky run`](#jky-run)
   - [`jky check`](#jky-check)
   - [`jky build`](#jky-build)
   - [`jky fmt`](#jky-fmt)
   - [`jky version`](#jky-version)
   - [`jky help`](#jky-help)
3. [Global Flags & Options](#global-flags--options)
4. [Cross-Compilation Target Triples](#cross-compilation-target-triples)
5. [Environment Variables](#environment-variables)
6. [Toolchain Exit Codes](#toolchain-exit-codes)
7. [Makefile Targets Reference](#makefile-targets-reference)
8. [Cross-Compiling Windows Binaries from Linux](#cross-compiling-windows-binaries-from-linux)
9. [Chapter Summary](#chapter-summary)

---

## 1. CLI Architecture & Overview

The `jky` driver binary is the universal entry point for all developer, analyst, and build automation workflows:

```bash
jky <subcommand> [arguments...] [flags...]
```

```
+-----------------------------------------------------------------------------+
|                            JOCKY CLI WORKFLOWS                              |
+-----------------------------------------------------------------------------+
| Development & Testing:                                                      |
|   jky check script.jky       ---> Parse & typecheck only (fast feedback)     |
|   jky run script.jky         ---> In-memory compile and immediate execution |
|   jky fmt script.jky         ---> Standard source code auto-formatter       |
|                                                                             |
| Production Deployment:                                                      |
|   jky compile main.jky -o agent.exe ---> Standalone polymorphic native agent|
|   jky build                  ---> Multi-file package compilation            |
+-----------------------------------------------------------------------------+
```

---

## 2. Command Reference

---

### `jky compile`
Compiles a JOCKY source file (`.jky`) into a standalone, stripped native executable with full 6-layer polymorphic mutations enabled.

```bash
jky compile <file.jky> -o <output_path> [flags...]
```

#### Arguments & Flags:
- `<file.jky>`: Path to the primary source file.
- `-o <output_path>`, `--output <output_path>`: (Mandatory) Destination path for the compiled native executable.
- `--salt <hex_string>`: (Optional) 64-character hexadecimal build salt seed. If omitted, a random cryptographic 32-byte salt is generated.
- `--target <triple>`: (Optional) Cross-compilation target triple (e.g. `x86_64-w64-mingw32`, `x86_64-linux-gnu`).
- `--debug`: (Optional) Emits verbose compiler diagnostics, AST statistics, and intermediate C source sizes to `stderr`.
- `-O0`, `-O2`, `-Os`: (Optional) Backend optimization level (default: `-O2`).

#### Example:
```bash
jky compile triage.jky -o triage_agent.exe --target x86_64-w64-mingw32 --salt a8f910e4b2...
```

---

### `jky run`
Compiles and executes a JOCKY script immediately in-memory on the local analyst workstation.

```bash
jky run <file.jky> [arguments...]
```

#### Notes:
- Supports **Script Mode**: top-level statements outside of `fn main()` are permitted.
- Executes via in-memory JIT or transient execution pipeline without creating permanent binary files on disk.

#### Example:
```bash
jky run scripts/quick_inspect.jky
```

---

### `jky check`
Performs lexical analysis, syntactic parsing, and two-pass semantic type checking on the specified source file without generating machine code.

```bash
jky check <file.jky>
```

#### Output on Success:
```
[+] Syntax and semantic check passed: triage.jky (0 errors, 0 warnings)
```

---

### `jky build`
Builds all `.jky` source files in the current working directory into a unified native binary package.

```bash
jky build [-o output_binary]
```

---

### `jky fmt`
Automatically formats JOCKY source code files according to canonical style rules (standard indentation, semicolon alignment, brace placement).

```bash
jky fmt <file.jky> [--write]
```

- `--write`, `-w`: Overwrites the file in place with formatted source.

---

### `jky version`
Displays toolchain version, target architecture, build date, and compiler hash.

```bash
jky version
```

---

### `jky help`
Prints usage instructions, command lists, and syntax summaries.

```bash
jky help [subcommand]
```

---

## 3. Global Flags & Options

| Flag | Short | Description |
| :--- | :--- | :--- |
| `--verbose` | `-v` | Enable detailed compiler debug logging |
| `--quiet` | `-q` | Suppress all non-error console output |
| `--no-color` | | Disable ANSI terminal color codes |
| `--help` | `-h` | Display help dialog |

---

## 4. Cross-Compilation Target Triples

| Target Triple | Target Platform | Generated Binary Format |
| :--- | :--- | :--- |
| `x86_64-linux-gnu` | 64-bit Linux (Ubuntu, RHEL, Debian) | ELF 64-bit LSB executable |
| `x86_64-w64-mingw32` | 64-bit Windows (10, 11, Server) | PE32+ 64-bit executable (`.exe`) |
| `i686-w64-mingw32` | 32-bit Legacy Windows | PE32 32-bit executable (`.exe`) |
| `aarch64-linux-gnu` | 64-bit ARM Linux (Raspberry Pi, AWS Graviton) | ELF 64-bit ARM |

---

## 5. Environment Variables

| Variable | Default Value | Description |
| :--- | :--- | :--- |
| `JOCKY_HOME` | `/usr/local/share/jky` or `C:\jocky` | Path to JOCKY root directory containing `runtime/` |
| `JOCKY_TARGET` | Host architecture | Default target triple if `--target` is omitted |
| `JOCKY_SALT` | Random | Global default build salt if `--salt` is omitted |
| `JOCKY_CC` | `gcc` or `clang` | Override host backend C compiler binary name |

---

## 6. Toolchain Exit Codes

| Exit Code | Meaning / Cause |
| :--- | :--- |
| `0` | **Success:** Operation completed with 0 errors |
| `1` | **Syntax / Parser Error:** Invalid token or malformed grammar |
| `2` | **Semantic / Type Error:** Type mismatch, undefined symbol, circular import |
| `3` | **Backend Compiler Error:** GCC/Clang failed during STDIN pipe compilation |
| `4` | **I/O Error:** Cannot open source file or cannot write destination binary |
| `139` | **Runtime Panic:** Nil pointer dereference during `jky run` execution |

---

## 7. Makefile Targets Reference

When maintaining the JOCKY compiler source repository, the root `Makefile` provides the following standardized targets:

```bash
make release      # Compiles optimized bin/jky executable
make debug        # Compiles bin/jky with GDB symbols (-g) and AddressSanitizer
make test         # Runs full integration and unit test suite in tests/
make clean        # Removes all build artifacts and bin/ contents
make install      # Copies bin/jky to /usr/local/bin and headers to /usr/local/share
```

---

## 8. Cross-Compiling Windows Binaries from Linux

To produce stealthy Windows `.exe` agents directly from your Linux analyst workstation:

```bash
# 1. Install MinGW cross-compiler
sudo apt-get install -y gcc-mingw-w64-x86-64

# 2. Compile JOCKY source targeting Windows PE
jky compile triage.jky -o triage_agent.exe --target x86_64-w64-mingw32

# 3. Verify PE header with file utility
file triage_agent.exe
# Output: triage_agent.exe: PE32+ executable (GUI) x86-64, for MS Windows
```

---

## 9. Chapter Summary

- **Core Commands:** `compile`, `run`, `check`, `build`, `fmt`, `version`, `help`.
- **Target Triples:** Full cross-compilation support for Windows PE and Linux ELF.
- **Exit Codes:** Deterministic return codes enable integration into CI/CD security pipelines.
- **Environment Variables:** `JOCKY_HOME` and `JOCKY_CC` allow full runtime localization.

In the next chapter, **[Chapter 21: Compiler Diagnostics & Error Codes](ch21_error_codes.md)**, we provide the complete directory of compiler diagnostic codes from `E0001` through `E0015`.
