# Chapter 16: Compiler Internals & Code Generation

> *"A compiler is not merely a translator—it is a secure transformation pipeline. JOCKY re-engineers every stage from lexical analysis to machine code emission to ensure zero disk exposure and maximum binary stealth."*

---

## Table of Contents
1. [The End-to-End Compiler Pipeline](#the-end-to-end-compiler-pipeline)
2. [Lexical Analysis (The Lexer)](#lexical-analysis-the-lexer)
   - [Token Structure & Token Classification](#token-structure--token-classification)
   - [Literal Parsing (Hex, Binary, Octal, Raw Strings)](#literal-parsing-hex-binary-octal-raw-strings)
   - [Diagnostic Error Tokens](#diagnostic-error-tokens)
3. [Syntactic Analysis (The Recursive Descent Parser)](#syntactic-analysis-the-recursive-descent-parser)
   - [Top-Down Parsing Grammar Rules](#top-down-parsing-grammar-rules)
   - [Operator Precedence Climbing](#operator-precedence-climbing)
   - [Panic-Mode Error Recovery](#panic-mode-error-recovery)
4. [Abstract Syntax Tree (AST) & Arena Memory Allocation](#abstract-syntax-tree-ast--arena-memory-allocation)
   - [AST Node Hierarchy](#ast-node-hierarchy)
   - [The Monolithic Arena Allocator Architecture](#the-monolithic-arena-allocator-architecture)
   - [Why Arena Allocation for Compilers?](#why-arena-allocation-for-compilers)
5. [Semantic Analysis & Two-Pass Symbol Resolution](#semantic-analysis--two-pass-symbol-resolution)
   - [Pass 1: Global Symbol & Type Registration](#pass-1-global-symbol--type-registration)
   - [Pass 2: Type Checking & Expression Resolution](#pass-2-type-checking--expression-resolution)
   - [The Block-Scoped Symbol Table Hierarchy](#the-block-scoped-symbol-table-hierarchy)
6. [Intermediate Code Generation (AST to C)](#intermediate-code-generation-ast-to-c)
   - [AST Node to C Construct Mapping Table](#ast-node-to-c-construct-mapping-table)
   - [Runtime Call Mapping & Tagged Union Desugaring](#runtime-call-mapping--tagged-union-desugaring)
7. [The In-Memory Native Compiler STDIN Pipeline](#the-in-memory-native-compiler-stdin-pipeline)
   - [Why Zero Disk Exposure? (Forensic & Anti-Forensic OPSEC)](#why-zero-disk-exposure-forensic--anti-forensic-opsec)
   - [Piping C Source to GCC/Clang via Stdin](#piping-c-source-to-gccclang-via-stdin)
   - [The `-nostdlib` Rationale & Minimal Linking](#the--nostdlib-rationale--minimal-linking)
8. [The BLAKE2b Cryptographic Key Derivation Engine](#the-blake2b-cryptographic-key-derivation-engine)
9. [Chapter Summary](#chapter-summary)

---

## 1. The End-to-End Compiler Pipeline

The `jky` compiler transforms human-readable JOCKY source code into standalone, polymorphic native machine code via a 7-stage in-memory pipeline:

```
+-----------------------------------------------------------------------------+
|                         JOCKY COMPILER ARCHITECTURE                         |
+-----------------------------------------------------------------------------+
|                                                                             |
|  [ Source File: script.jky ]                                                |
|            |                                                                |
|            v                                                                |
|  1. Lexer (src/lexer.c) ---------> Generates Token Stream                   |
|            |                                                                |
|            v                                                                |
|  2. Parser (src/parser.c) -------> Arena-Allocated AST                      |
|            |                                                                |
|            v                                                                |
|  3. Semantic Analyzer (src/sema.c) -> 2-Pass Symbol Table & Type Validator  |
|            |                                                                |
|            v                                                                |
|  4. Stealth Mutator (src/stealth.c) -> 6-Layer Obfuscation & BLAKE2b Keys   |
|            |                                                                |
|            v                                                                |
|  5. C Code Generator (src/codegen.c) -> Emits In-Memory C Source Buffer    |
|            |                                                                |
|            v                                                                |
|  6. Native IPC Compiler Pipe ----> Forks GCC/Clang with -nostdlib (STDIN)   |
|            |                                                                |
|            v                                                                |
|  7. [ Output Native Binary: agent.exe / agent.elf ]                         |
|                                                                             |
+-----------------------------------------------------------------------------+
```

---

## 2. Lexical Analysis (The Lexer)

The lexer (`src/lexer.c`) scans the raw UTF-8 source buffer and emits discrete tokens without allocating small heap fragments for individual strings.

### Token Structure (`src/lexer.h`)
```c
typedef enum {
    TOKEN_EOF = 0,
    TOKEN_KEYWORD_FN,
    TOKEN_KEYWORD_STRUCT,
    TOKEN_KEYWORD_IMPORT,
    TOKEN_KEYWORD_AUTO,
    TOKEN_KEYWORD_CONST,
    TOKEN_KEYWORD_IF,
    TOKEN_KEYWORD_ELSE,
    TOKEN_KEYWORD_WHILE,
    TOKEN_KEYWORD_FOR,
    TOKEN_KEYWORD_RETURN,
    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_BYTES_LITERAL,
    TOKEN_SEMICOLON,
    TOKEN_ARROW,       // ->
    TOKEN_BANG,        // !
    TOKEN_ERROR        // Diagnostic error token
} JkyTokenType;

typedef struct {
    JkyTokenType type;
    const char  *start;       // Pointer into source buffer
    int          length;      // Token character length
    int          line;        // Source line number (1-indexed)
    int          column;      // Source column number (1-indexed)
    union {
        int64_t  int_val;
        double   float_val;
    } literal;
} JkyToken;
```

---

## 3. Syntactic Analysis (The Recursive Descent Parser)

The parser (`src/parser.c`) is a hand-written, deterministic **recursive descent parser** with operator precedence climbing.

### Operator Precedence Climbing
Binary expressions are parsed using Pratt-style precedence climbing:

```c
static JkyAstNode* parse_expression_precedence(JkyParser *p, int min_precedence) {
    JkyAstNode *left = parse_unary_expression(p);

    while (true) {
        JkyToken op = p->current_token;
        int prec = get_token_precedence(op.type);
        if (prec < min_precedence) break;

        advance_token(p); // Consume operator
        JkyAstNode *right = parse_expression_precedence(p, prec + 1);

        // Check for illegal unparenthesized bitwise/comparison mix!
        if (is_bitwise(op.type) && (is_comparison(right->type) || is_additive(right->type))) {
            report_error(p, "E0005: Ambiguous mixed bitwise and comparison expression. Parentheses mandatory.");
        }

        left = ast_create_binary_op(p->arena, op.type, left, right);
    }
    return left;
}
```

---

## 4. Abstract Syntax Tree (AST) & Arena Memory Allocation

Every node in the AST is allocated from a single contiguous memory arena.

```
+-----------------------------------------------------------------------------+
|                         Arena Memory Architecture                           |
+-----------------------------------------------------------------------------+
| [ Arena Page 1: 4MB ] -> [ Node 1 ][ Node 2 ][ Node 3 ] ...                 |
|                              ^                                              |
|                              | Direct contiguous bump-pointer allocation    |
|                                                                             |
| Total Compilation Complete -> Free Entire Arena Page in ONE Operation (0ms)|
+-----------------------------------------------------------------------------+
```

### Why Arena Allocation?
1. **Extreme Allocation Speed:** Allocating an AST node is a simple pointer bump (`arena->current += size`), taking ~1 CPU cycle.
2. **Zero Memory Fragmentation:** Compiling thousands of lines of JOCKY code creates zero heap fragmentation.
3. **Instant Teardown:** When compilation completes, the entire arena memory is freed in a single `free()` call, eliminating recursive node traversal.

---

## 5. Semantic Analysis & Two-Pass Symbol Resolution

Semantic analysis (`src/sema.c`) verifies type safety and resolves symbols in two distinct passes:

### Pass 1: Global Symbol Registration
- Collects all struct definitions, member offsets, and function signatures across the package.
- Builds the root module scope table.
- Detects duplicate definitions (`E0001`).

### Pass 2: Type Checking & Type Lowering
- Validates variable assignment types.
- Ensures all functions returning a value have valid return statements.
- Verifies that the `!` operator is only used inside functions returning `(..., Error)`.
- Resolves implicit `auto` types to their concrete static representations.

---

## 6. Intermediate Code Generation (AST to C)

The code generator (`src/codegen.c`) walks the validated AST and emits standards-compliant C11 source code into an in-memory dynamic buffer (`JkyBuffer`).

### AST Node to C Mapping Table

| JOCKY AST Node | Generated C Construct | Runtime Function |
| :--- | :--- | :--- |
| `DeclVar(int x = 5)` | `int64_t x = 5;` | Native stack allocation |
| `DeclVar(string s = "A")`| `JkyString *s = _jky_str_new("A");`| `_jky_str_new()` |
| `ExprCall(process.list)` | `_jky_process_list()` | `jky_process.c` runtime provider |
| `ExprTry(call()!)` | `if (_err != NULL) return _err;` | Inlined error check |
| `ExprMethod(cf.seal())` | `_jky_fn_CaseFile_seal(cf)` | Lowered C function call |
| `StructDef(Point)` | `typedef struct { ... } JkyStruct_Point;` | Aligned C struct |

---

## 7. The In-Memory Native Compiler STDIN Pipeline

In traditional compilers, intermediate code is written to disk (e.g. `/tmp/ccX0183.c`), compiled to an object file (`/tmp/ccX0183.o`), and linked.

In contested forensic environments, **writing intermediate `.c` or `.o` files to disk leaves forensic artifacts on target media**.

JOCKY implements a **Direct Memory-to-Memory Compiler Pipe**:

```
+-------------------------------------------------------------------------+
|                  In-Memory Compiler Piping (Zero Disk)                  |
+-------------------------------------------------------------------------+
| [ JOCKY Compiler (jky) ]                                                |
|       |                                                                 |
|       | 1. Synthesizes complete C source in RAM buffer                  |
|       | 2. Forks child compiler: gcc -O2 -nostdlib -x c - -o agent.exe  |
|       | 3. Writes C source directly to child process STDIN pipe         |
|       v                                                                 |
| [ GCC / Clang Backend ]                                                 |
|       |                                                                 |
|       | 4. Reads source from STDIN, compiles, links                     |
|       v                                                                 |
| [ Standalone Native Binary: agent.exe ]                                 |
+-------------------------------------------------------------------------+
```

### The `-nostdlib` Rationale
By supplying `-nostdlib` and linking only against fundamental OS entry points:
- Eliminates standard GCC / MSVC runtime boilerplate and compiler version strings.
- Strips predictable C-runtime startup routines (`crt0.o`).
- Reduces compiled binary size by over **90%** (from 4.5MB down to ~300KB).

---

## 8. The BLAKE2b Cryptographic Key Derivation Engine

Every string literal encountered during compilation is encrypted using a unique per-site XOR key derived via the **BLAKE2b** cryptographic hash function:

$$\text{SiteKey} = \text{BLAKE2b-256}(\text{BuildSalt} \;\|\; \text{FileID} \;\|\; \text{ASTNodeOffset})$$

```c
// BLAKE2b Per-Site Key Derivation in src/stealth.c
void jky_derive_site_key(const uint8_t build_salt[32], uint32_t site_id, uint8_t out_key[32]) {
    blake2b_state S;
    blake2b_init(&S, 32);
    blake2b_update(&S, build_salt, 32);
    blake2b_update(&S, (uint8_t*)&site_id, sizeof(site_id));
    blake2b_final(&S, out_key, 32);
}
```

---

## 9. Chapter Summary

- **Pipeline:** 7-stage in-memory pipeline: Lexer $\rightarrow$ Parser $\rightarrow$ Semantic Analyzer $\rightarrow$ Stealth Mutator $\rightarrow$ C Emitter $\rightarrow$ STDIN Pipe $\rightarrow$ Native Binary.
- **Arena Allocator:** Contiguous memory arena guarantees single-cycle node allocation and instant 0ms teardown.
- **Zero Disk Footprint:** Intermediate C code is piped directly via STDIN, leaving zero disk artifacts.
- **Minimal CRT:** `-nostdlib` emission eliminates compiler fingerprints and produces ultra-compact native binaries.

In the next chapter, **[Chapter 17: The Stealth Sub-System](ch17_stealth_system.md)**, we examine the mathematics and implementation of JOCKY's six core obfuscation engines.
