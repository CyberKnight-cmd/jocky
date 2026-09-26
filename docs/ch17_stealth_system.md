# Chapter 17: The Stealth Sub-System

> *"Stealth in modern cybersecurity is not about magic tricks or packed binary shells; it is about mathematical entropy, structural variation, and the total elimination of predictable compiler artifacts."*

---

## Table of Contents
1. [The EDR/AV Threat Model & Detection Mechanisms](#the-edrav-threat-model--detection-mechanisms)
2. [The 6-Layer Stealth Architecture](#the-6-layer-stealth-architecture)
3. [Feature 1: Per-Site BLAKE2b String XOR Encryption](#feature-1-per-site-blake2b-string-xor-encryption)
   - [What It Defeats: Static String Extraction & YARA Rules](#what-it-defeats-static-string-extraction--yara-rules)
   - [Algorithm & Key Derivation Mathematics](#algorithm--key-derivation-mathematics)
   - [Inline Runtime Decryptor Stubs](#inline-runtime-decryptor-stubs)
4. [Feature 2: Cryptographic Symbol Salting](#feature-2-cryptographic-symbol-salting)
   - [What It Defeats: Symbol Table & Import Reconstruction](#what-it-defeats-symbol-table--import-reconstruction)
   - [Hash Name Mangling Scheme](#hash-name-mangling-scheme)
5. [Feature 3: Dead Code Injection & Opaque Predicates](#feature-3-dead-code-injection--opaque-predicates)
   - [What It Defeats: Linear Disassembly & Heuristic Hashes](#what-it-defeats-linear-disassembly--heuristic-hashes)
   - [Mathematical Construction of Opaque Predicates](#mathematical-construction-of-opaque-predicates)
6. [Feature 4: Control-Flow Graph (CFG) Flattening](#feature-4-control-flow-graph-cfg-flattening)
   - [What It Defeats: Graph Isomorphism & Decompiler Analysis](#what-it-defeats-graph-isomorphism--decompiler-analysis)
   - [State Machine Dispatcher Architecture](#state-machine-dispatcher-architecture)
7. [Feature 5: The 32-Byte Build Salt Engine](#feature-5-the-32-byte-build-salt-engine)
   - [Global Entropy Injection](#global-entropy-injection)
   - [Deterministic Lab Builds via `--salt`](#deterministic-lab-builds-via---salt)
8. [Feature 6: Stack Frame Layout Randomization](#feature-6-stack-frame-layout-randomization)
   - [What It Defeats: Stack Scanning & Fixed Offset Fingerprinting](#what-it-defeats-stack-scanning--fixed-offset-fingerprinting)
   - [Synthetic Stack Variable Padding](#synthetic-stack-variable-padding)
9. [Composition of the 6 Layers](#composition-of-the-6-layers)
10. [Disassembly Comparison: Build A vs. Build B in Ghidra/IDA Pro](#disassembly-comparison-build-a-vs-build-b-in-ghidraidapro)
11. [Limitations & The Reality of Behavioral Detection](#limitations--the-reality-of-behavioral-detection)
12. [Chapter Summary](#chapter-summary)

---

## 1. The EDR/AV Threat Model & Detection Mechanisms

Modern Endpoint Detection and Response (EDR) and Antivirus (AV) solutions inspect running binaries across three primary detection vectors:

```
+-----------------------------------------------------------------------------+
|                         Modern EDR Detection Matrix                         |
+-----------------------------------------------------------------------------+
| 1. Static Analysis (Disk / Memory Image)                                    |
|    - Plaintext string searches (e.g., "CASE-01", "lsass", "SeDebug")       |
|    - YARA signature rules matching fixed byte sequences                      |
|    - PE section headers, import hash (Imphash), and rich header fingerprints|
|                                                                             |
| 2. Structural & Decompilation Analysis (Heuristics)                          |
|    - Control Flow Graph (CFG) graph isomorphism matching                    |
|    - Function prologue/epilogue fingerprinting                              |
|    - Linear basic block sequence hashing                                    |
|                                                                             |
| 3. Dynamic / Behavioral Analysis (Runtime Hooks)                            |
|    - Stack walking and return-address inspection                            |
|    - User-mode API hooking inside ntdll.dll                                 |
|    - Process memory scanning for unencrypted payloads                      |
+-----------------------------------------------------------------------------+
```

To defeat these detection vectors without relying on suspicious binary packers, JOCKY embeds **six native compiler transformations** directly into the AST lowering pass.

---

## 2. The 6-Layer Stealth Architecture

```
+-----------------------------------------------------------------------------+
|                         6-LAYER COMPILER STEALTH ENGINE                     |
+-----------------------------------------------------------------------------+
|                                                                             |
|  [ Layer 1: Per-Site String XOR ]  ---> Encrypts strings with BLAKE2b keys  |
|                                                                             |
|  [ Layer 2: Symbol Salting ]       ---> Mangles function & variable names   |
|                                                                             |
|  [ Layer 3: Dead Code Injection ]  ---> Synthesizes opaque dummy blocks     |
|                                                                             |
|  [ Layer 4: CFG Flattening ]       ---> Transforms branches to state machines|
|                                                                             |
|  [ Layer 5: Build Salt Engine ]    ---> 32-byte cryptographic entropy seed  |
|                                                                             |
|  [ Layer 6: Stack Randomization ]  ---> Injects dynamic frame padding bytes |
|                                                                             |
+-----------------------------------------------------------------------------+
```

---

## 3. Feature 1: Per-Site BLAKE2b String XOR Encryption

### What It Defeats:
Static string extraction (`strings.exe`), YARA pattern matching on string tables, and `.rdata` section inspection.

### How It Works:
In a standard C binary, string literals like `"CASE-2026-NTRO"` reside in plaintext in the read-only data section (`.rdata`).

In JOCKY:
1. Every string literal encountered in the AST is assigned a unique `SiteID` based on its source file location and AST node index.
2. The compiler derives a 32-byte cryptographic key using **BLAKE2b**:
   $$\text{Key} = \text{BLAKE2b-256}(\text{BuildSalt} \parallel \text{SiteID})$$
3. The string bytes are XOR-encrypted at compile-time and emitted as a hex byte array in C.
4. An inline stack decryptor reconstructs the string on the CPU stack only at the exact moment of execution and immediately wipes the decryptor buffer after use.

```c
// Emitted C code for encrypted string "CASE-001":
// Compile-time XOR with derived key 0x4B...
static inline JkyString* _jky_decrypt_str_site_42(void) {
    uint8_t enc[] = { 0x08, 0x0A, 0x18, 0x0E, 0x66, 0x7B, 0x7B, 0x7A };
    uint8_t key[] = { 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B, 0x4B };
    char dec[9];
    for (int i = 0; i < 8; i++) dec[i] = (char)(enc[i] ^ key[i]);
    dec[8] = '\0';
    return _jky_str_new(dec);
}
```

---

## 4. Feature 2: Cryptographic Symbol Salting

### What It Defeats:
Symbol name matching, static function export inspection, and reverse engineering function identification.

### How It Works:
All user-defined functions, helper routines, and internal variables are mangled using the 32-byte build salt:

$$\text{MangledName} = \text{"\_jky\_fn\_"} \parallel \text{TruncatedHex}(\text{BLAKE2b}(\text{BuildSalt} \parallel \text{OriginalName}))$$

```c
// Original JOCKY function:
fn inspect_kernel() -> void { ... }

// Generated C function name in Build 1:
void _jky_fn_a8f941c2(void) { ... }

// Generated C function name in Build 2 (different salt):
void _jky_fn_3b1109e7(void) { ... }
```

---

## 5. Feature 3: Dead Code Injection & Opaque Predicates

### What It Defeats:
Linear disassembly, signature-based machine code pattern matching, and function length heuristics.

### How It Works:
The compiler randomly injects 1 to 4 unreachable basic blocks into every compiled function. To prevent the backend compiler's optimizer (e.g. GCC `-O2`) from stripping the unreachable code, JOCKY utilizes **Mathematically Opaque Predicates**:

$$\forall x \in \mathbb{Z}, \quad (x^2 + x) \pmod 2 = 0 \quad (\text{Always True})$$
$$7y^2 - 1 = z^2 \quad (\text{Has no integer solutions: Always False})$$

```c
// Injected opaque predicate in generated C:
int _jky_x = (int)(uintptr_t)&_jky_x; // Address of local variable
if ((_jky_x * _jky_x + _jky_x) % 2 != 0) {
    // Unreachable synthetic dead code block:
    volatile int _dummy = 0x9090;
    _dummy += 42;
    __builtin_trap();
}
```

Because the C compiler cannot mathematically prove that `_jky_x` will always satisfy the quadratic congruence at compile time, it is forced to emit the dead instructions into the final machine code.

---

## 6. Feature 4: Control-Flow Graph (CFG) Flattening

### What It Defeats:
Graph isomorphism matching, decompilation into clean high-level loops/branches, and structural heuristic scoring.

### How It Works:
Control-flow flattening breaks a linear function body into a collection of basic blocks and places them inside a flat `switch` dispatcher governed by a state variable:

```
+-----------------------------------------------------------------------------+
|                        Control-Flow Graph Flattening                        |
+-----------------------------------------------------------------------------+
| Standard CFG:                                                               |
|   [ Block A ] ----> [ Block B ] ----> [ Block C ]                           |
|                                                                             |
| Flattened CFG (State Machine Dispatcher):                                   |
|                          +--------------------+                             |
|                          |  State Dispatcher  |<-----------+                |
|                          |    switch(state)   |            |                |
|                          +----+----+----+-----+            |                |
|                               |    |    |                  |                |
|                     +---------+    |    +---------+        |                |
|                     v              v              v        |                |
|               [ Block A ]    [ Block B ]    [ Block C ]    |                |
|               state = 0x4B;  state = 0x91;  state = 0xFF;  |                |
|                     |              |              |        |                |
|                     +--------------+--------------+--------+                |
+-----------------------------------------------------------------------------+
```

In Ghidra or IDA Pro, the decompiled output appears as an impenetrable, tangled state machine loop rather than clear, sequential logic.

---

## 7. Feature 5: The 32-Byte Build Salt Engine

### Global Entropy Injection
When `jky compile` is executed without flags, the compiler queries the operating system's cryptographic random number generator (`/dev/urandom` on Linux; `BCryptGenRandom` on Windows) to generate a 32-byte (256-bit) **Build Salt**:

```
Build Salt: 8f4a10c92b7700e1f3d891aa56123490bfde3128905541cbaa78901234567890
```

Every subsequent stealth pass (string encryption, symbol mangling, CFG state constants, opaque predicate variables, and stack padding) is derived deterministically from this master build salt.

### Deterministic Lab Builds via `--salt`
When formal laboratory reproducibility or debugging is required:
```bash
jky compile triage.jky -o agent.exe --salt 0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef
```

---

## 8. Feature 6: Stack Frame Layout Randomization

### What It Defeats:
In-memory stack scanners that search for predictable pointer alignments, return addresses, and fixed local variable offsets.

### How It Works:
The code generator injects synthetic, randomized byte padding buffers between local stack variables:

```c
void _jky_fn_triage(void) {
    uint8_t _pad_0[24]; // 24 bytes random stack padding
    int64_t case_id;
    uint8_t _pad_1[40]; // 40 bytes random stack padding
    JkyString *target_name;
    uint8_t _pad_2[16];
    // Function body...
}
```

The exact size and arrangement of padding arrays vary across every compilation build, shifting local variable offsets unpredictably in memory.

---

## 9. Composition of the 6 Layers

The power of JOCKY's stealth system lies in the **multiplicative composition** of all six layers:

$$\text{Total Entropy Space} \approx 2^{256} \; (\text{Salt}) \times \prod (\text{String Keys}) \times (\text{CFG Permutations}) \times (\text{Dead Block Layouts})$$

No single static signature, hash, or structural template can ever identify a JOCKY binary across distinct builds.

---

## 10. Disassembly Comparison: Build A vs. Build B in Ghidra/IDA Pro

| Binary Attribute | Standard C Binary | JOCKY Build A (Salt 1) | JOCKY Build B (Salt 2) |
| :--- | :--- | :--- | :--- |
| **SHA-256 Hash** | Constant | `4a8b...` | `9d31...` |
| **Imphash** | Identical | Unique | Unique |
| **`.rdata` Strings** | Plaintext (`"CASE-01"`) | High Entropy XOR blob | Distinct XOR blob |
| **Functions** | `main`, `collect_host` | `_jky_fn_8f4a`, `_jky_fn_10c9` | `_jky_fn_99a1`, `_jky_fn_44df` |
| **CFG Shape** | Linear Call Hierarchy | Flattened Switch (State 0x41) | Flattened Switch (State 0x8E) |
| **Stack Frame Size**| 64 bytes | 128 bytes | 96 bytes |

---

## 11. Limitations & The Reality of Behavioral Detection

> [!CAUTION]
> While JOCKY's stealth sub-system completely neutralizes static signature matching, string extraction, and structural heuristics, **it cannot alter the fundamental physics of operating system telemetry**.

If a compiled JOCKY binary invokes an OS syscall to dump the memory of `lsass.exe`, an EDR kernel callback (`PsSetCreateProcessNotifyRoutine` / `ObRegisterCallbacks`) will still observe that process handle request.

To maintain operational stealth in the field:
1. Prefer read-only queries (`process.list()`, `network.connections()`, `fs.read()`).
2. Utilize `@privileged` guard blocks to ensure credentials exist before querying restricted handles.
3. Keep dwell time and polling intervals realistic.

---

## 12. Chapter Summary

- **6 Stealth Layers:** String XOR, Symbol Salting, Dead Code, CFG Flattening, 32-Byte Build Salt, and Stack Frame Randomization.
- **Dynamic Entropy:** Every build yields unique binary hashes, import tables, and disassembly graphs.
- **Opaque Predicates:** Mathematical tautologies prevent compiler optimizers from stripping dead code.
- **Zero Disk Exposure:** The compilation pipe feeds the backend compiler entirely in memory via STDIN.

In the next chapter, **[Chapter 18: Real-World Forensic Workflows](ch18_forensic_workflows.md)**, we walk through six complete, end-to-end incident response triage scripts.
