# Chapter 5: Language Basics & Lexical Structure

> *"Clarity of syntax breeds reliability of execution. In JOCKY, every lexical rule, whitespace boundary, and semicolon requirement has been chosen to eliminate syntactic ambiguity before compilation begins."*

---

## Table of Contents
1. [Lexical Elements & Token Stream](#lexical-elements--token-stream)
2. [Whitespace & Source Formatting](#whitespace--source-formatting)
3. [The Mandatory Semicolon Policy](#the-mandatory-semicolon-policy)
4. [Comments & Documentation](#comments--documentation)
5. [Identifiers & Naming Conventions](#identifiers--naming-conventions)
   - [The Blank Identifier `_`](#the-blank-identifier-_)
   - [Reserved Identifiers & Name Mangling](#reserved-identifiers--name-mangling)
6. [Keywords & Reserved Words](#keywords--reserved-words)
7. [Literals](#literals)
   - [Integer & Floating-Point Literals](#integer--floating-point-literals)
   - [String & Raw String Literals](#string--raw-string-literals)
   - [Byte & Hexadecimal Byte Blobs](#byte--hexadecimal-byte-blobs)
8. [File Structure & Top-Level Declarations](#file-structure--top-level-declarations)
9. [Script Mode vs. Agent Mode Deep Dive](#script-mode-vs-agent-mode-deep-dive)
10. [Chapter Summary](#chapter-summary)

---

## 1. Lexical Elements & Token Stream

A JOCKY source program is a sequence of Unicode characters encoded in UTF-8. The lexical analyzer (lexer) reads the source text and converts it into a linear stream of discrete tokens.

The lexer classifies tokens into four fundamental categories:
1. **Keywords** (`fn`, `struct`, `import`, `if`, `while`, `for`, `return`, `const`, `auto`)
2. **Identifiers** (`case_id`, `process_list`, `SealEvidence`)
3. **Literals** (`42`, `3.14159`, `"CASE-001"`, `x"9090AF"`)
4. **Operators & Punctuation** (`+`, `-`, `*`, `/`, `->`, `!`, `;`, `{`, `}`, `(`, `)`)

```
Source Code:
    int count = 10;

Lexer Output Token Stream:
    [TOKEN_TYPE_INT, "int"]
    [TOKEN_IDENTIFIER, "count"]
    [TOKEN_ASSIGN, "="]
    [TOKEN_INT_LITERAL, "10"]
    [TOKEN_SEMICOLON, ";"]
```

---

## 2. Whitespace & Source Formatting

In JOCKY, whitespace consists of:
- Spaces (`ASCII 0x20`)
- Horizontal tabs (`ASCII 0x09`)
- Carriage returns (`ASCII 0x0D`)
- Newlines (`ASCII 0x0A`)

Whitespace serves solely to separate otherwise adjacent tokens. Except within string literals, multiple consecutive whitespace characters are treated as a single whitespace separator and are ignored by the parser.

Unlike Python, **JOCKY is not indentation-sensitive**. Code blocks are explicitly delimited by curly braces (`{` and `}`).

```jocky
// Both forms are syntactically identical to the JOCKY compiler:

// Standard idiomatic formatting:
if count > 0 {
    log.info("Active count detected");
}

// Compact single-line formatting:
if count > 0 { log.info("Active count detected"); }
```

---

## 3. The Mandatory Semicolon Policy

In JOCKY, **semicolons (`;`) are strictly mandatory** at the termination of every statement, variable declaration, import statement, and struct field definition.

### Why Mandatory Semicolons?
Modern languages such as JavaScript, Python, and Go implement Automatic Semicolon Insertion (ASI) or newline-driven parsing. While this saves keystrokes, it introduces subtle, high-risk parsing ambiguities:

```javascript
// A classic JavaScript ASI gotcha:
return
{
    status: "ok"
};
// Parsed as: return; (returns undefined!)
```

In forensic operations, a misparsed line or hidden statement termination can result in an unexecuted evidence-collection routine or an accidental early return.

By requiring explicit semicolons:
1. **Zero Lexical Ambiguity:** The compiler's recursive descent parser always knows the exact boundary of every statement, regardless of line wrapping or formatting.
2. **Simplified In-Memory Obfuscation:** The control-flow flattening pass can cleanly dissect, extract, and reorder AST statements without risking accidental token concatenation.
3. **Defensive Coding Standard:** Forces investigators to be explicit and deliberate about statement termination.

```jocky
// Correct:
int max_retries = 3;
auto result = host.info();
import network;

// COMPILE ERROR: E0002: Expected ';' after variable declaration
int max_retries = 3
```

---

## 4. Comments & Documentation

JOCKY supports standard C-style single-line and multi-line comments.

### Single-Line Comments (`//`)
Single-line comments begin with two forward slashes `//` and extend to the end of the current line:

```jocky
// This is a single-line comment explaining the triage logic
int timeout_ms = 5000; // Inline comment: 5-second network timeout
```

### Multi-Line Block Comments (`/* ... */`)
Block comments begin with `/*` and terminate with `*/`. They may span multiple lines and can be placed anywhere whitespace is permitted:

```jocky
/*
 * Case File Metadata Aggregator
 * Target Agency: NTRO Cyber Operations
 * Classification: RESTRICTED
 */
fn process_bundle(string case_id) -> void {
    /* inline block comment */ log.info(case_id);
}
```

> [!NOTE]
> Block comments do not nest. A `*/` token encountered inside a comment block will terminate the comment immediately.

---

## 5. Identifiers & Naming Conventions

Identifiers in JOCKY name variables, functions, structs, fields, and packages.

### Lexical Rule:
Identifiers must begin with an ASCII letter (`a-z` or `A-Z`) and may be followed by any number of letters, digits (`0-9`), or underscores (`_`).

$$\text{Identifier} = [a-zA-Z][a-zA-Z0-9\_]*$$

```jocky
// Valid identifiers:
int caseCount = 10;
string host_name_2 = "ALPHA";
auto MemoryDumpBuffer = fs.read("/dev/mem");

// INVALID identifiers:
int 2cases = 5;       // COMPILE ERROR: Cannot start with a digit
int host-name = 10;   // COMPILE ERROR: Hyphens are subtraction operators
```

### The Blank Identifier (`_`)

The single underscore `_` is the **blank identifier**. It acts as a write-only anonymous placeholder to explicitly discard unused return values or error tuples:

```jocky
// Discarding a return value when only the error is needed:
auto _, err = evidence.open("CASE-001");
if err != nil {
    log.error("Failed to open case container");
}

// Discarding index during collection iteration:
for item in process_list {
    log.info("Process: " + item.name);
}
```

> [!WARNING]
> Leading underscores (`_temp`, `_private`) are **strictly reserved for compiler-internal symbols** and runtime generated structures. User-defined identifiers beginning with an underscore will trigger compiler warning or error `E0004`.

---

## 6. Keywords & Reserved Words

The following 28 keywords are reserved by the JOCKY language specification and cannot be used as user-defined identifiers:

| Category | Keywords |
| :--- | :--- |
| **Type Specifiers** | `void`, `int`, `float`, `bool`, `byte`, `bytes`, `string`, `list`, `map`, `Error` |
| **Declarations** | `fn`, `struct`, `import`, `const`, `auto` |
| **Control Flow** | `if`, `else`, `while`, `for`, `in`, `return`, `break`, `continue` |
| **Constants & Literals** | `true`, `false`, `nil` |
| **Special** | `_` (blank identifier) |

---

## 7. Literals

### Integer & Floating-Point Literals

JOCKY provides flexible number literal formatting, including decimal, hexadecimal, binary, octal, and scientific notation. Number separators (`_`) are supported for readability:

```jocky
// Integer literals (all stored as signed 64-bit integers: int)
int dec = 42;
int million = 1_000_000;         // Underscore separator for readability
int hex = 0xFF;                  // Hexadecimal (255)
int bin = 0b1010_1100;           // Binary (172)
int oct = 0o755;                 // Octal (493)

// Floating-point literals (stored as 64-bit IEEE 754: float)
float pi = 3.14159;
float sci = 1.5e10;
float small = 2.5e-4;
```

### String & Raw String Literals

1. **Standard Interpreted Strings (`"..."`):** Enclosed in double quotes. Supports standard escape sequences (`\n`, `\r`, `\t`, `\\`, `\"`, `\xHH`, `\uXXXX`):

```jocky
string path = "C:\\Windows\\System32\\ntdll.dll";
string greeting = "Forensic Triage v0.1\nStatus: READY\tTarget: ALL";
```

2. **Raw String Literals (`r"..."`):** Enclosed in `r"..."`. No escape sequences are processed. Ideal for Windows filesystem paths and regex patterns:

```jocky
// Raw string: backslashes are preserved verbatim
string raw_path = r"C:\Windows\System32\drivers\etc\hosts";
string regex_pattern = r"^[\w\.-]+@[\w\.-]+\.\w+$";
```

### Byte & Hexadecimal Byte Blobs

1. **Byte Literals (`'...'`):** Single 8-bit unsigned character literals:

```jocky
byte header_char = 'M';          // ASCII 'M' (0x4D)
byte null_term = '\0';
```

2. **Hexadecimal Byte Blobs (`x"..."`):** Hex-encoded binary blobs representing raw byte sequences. Essential for YARA signatures, shellcode buffers, and magic byte headers:

```jocky
// Portable Executable (PE) Magic Header: "MZ"
bytes pe_magic = x"4D5A";

// NOP sled followed by INT3 breakpoint
bytes shellcode_stub = x"909090CC";
```

---

## 8. File Structure & Top-Level Declarations

A JOCKY source file (`.jky`) is structured into three standard sections:

```
+-----------------------------------------------------------------------------+
| 1. Imports Section                                                          |
|    import host;                                                             |
|    import process;                                                          |
|    import evidence;                                                         |
+-----------------------------------------------------------------------------+
| 2. Struct Definitions & Global Constants                                    |
|    const int MAX_PROCESS_CACHE = 1024;                                      |
|                                                                             |
|    struct TriageConfig {                                                    |
|        string case_id;                                                      |
|        bool capture_memory;                                                 |
|    }                                                                        |
+-----------------------------------------------------------------------------+
| 3. Functions & Methods                                                      |
|    fn (TriageConfig cfg) execute() -> void {                                |
|        // Method implementation                                             |
|    }                                                                        |
|                                                                             |
|    fn main() -> void {                                                      |
|        // Entry point                                                       |
|    }                                                                        |
+-----------------------------------------------------------------------------+
```

### Two-Pass Symbol Resolution
In JOCKY, functions, methods, and structs can be declared in **any order within the file**. The compiler executes a two-pass symbol collection phase before type checking. You do not need forward declarations or C-style header files.

```jocky
// Valid: helper() is called before it is declared
fn main() -> void {
    helper();
}

fn helper() -> void {
    log.info("Helper executed");
}
```

---

## 9. Script Mode vs. Agent Mode Deep Dive

| Feature | Script Mode (`jky run`) | Agent Mode (`jky compile`) |
| :--- | :--- | :--- |
| **Invocation** | `jky run triage.jky` | `jky compile triage.jky -o agent.exe` |
| **Entry Point** | Top-level statements allowed | `fn main() -> void` strictly mandatory |
| **Compilation** | In-memory JIT execution | In-memory native build $\rightarrow$ disk binary |
| **Stealth Engine** | Minimal (Fast execution) | **Full 6-Layer Polymorphism Enabled** |
| **Intended Use** | Local development, unit testing | Tactical field deployment on target host |

---

## 10. Chapter Summary

- **Tokens & Semicolons:** Semicolons are always mandatory, eliminating ASI ambiguity and facilitating in-memory code restructuring.
- **Identifiers:** Standard alphanumeric rules apply; `_` is the discard placeholder; leading underscores are reserved for the compiler.
- **Literals:** Rich literal syntax includes raw strings (`r"..."`), binary/hex numbers, and raw byte blobs (`x"4D5A"`).
- **Declaration Order:** Two-pass symbol resolution allows functions and structs to be declared in any order without headers.

In the next chapter, **[Chapter 6: Type System](ch06_types.md)**, we examine JOCKY's strong, static type system, memory representations, and nil safety rules in exhaustive detail.
