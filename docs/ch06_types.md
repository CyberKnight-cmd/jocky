# Chapter 6: Type System

> *"A type system is not merely a mechanism for memory allocation—it is a formal mathematical contract. In forensic engineering, strict type invariants prevent silent data corruption, buffer overruns, and catastrophic analytical errors."*

---

## Table of Contents
1. [Type System Overview & Taxonomy](#type-system-overview--taxonomy)
2. [Primitive Numeric Types](#primitive-numeric-types)
   - [The `int` Type (64-Bit Signed Integer)](#the-int-type-64-bit-signed-integer)
   - [The `float` Type (64-Bit IEEE 754 Floating Point)](#the-float-type-64-bit-ieee-754-floating-point)
   - [The `byte` Type (8-Bit Unsigned Integer)](#the-byte-type-8-bit-unsigned-integer)
   - [The `bool` Type (Boolean Logic)](#the-bool-type-boolean-logic)
   - [The `void` Type](#the-void-type)
3. [Binary Data: The `bytes` Type](#binary-data-the-bytes-type)
4. [Textual Data: The `string` Type](#textual-data-the-string-type)
5. [Collection Types](#collection-types)
   - [The `list` Type (Dynamic Array)](#the-list-type-dynamic-array)
   - [The `map` Type (Associative Key-Value Store)](#the-map-type-associative-key-value-store)
6. [The `Error` Type & Diagnostic Semantics](#the-error-type--diagnostic-semantics)
7. [The `nil` Value & Reference Safety](#the-nil-value--reference-safety)
   - [Which Types Can Be Nil?](#which-types-can-be-nil)
   - [Which Types Can NEVER Be Nil?](#which-types-can-never-be-nil)
   - [Nil Dereference Semantics](#nil-dereference-semantics)
8. [Type Casting & Explicit Conversion Rules](#type-casting--explicit-conversion-rules)
9. [The Absolute Prohibition of Implicit Conversions](#the-absolute-prohibition-of-implicit-conversions)
10. [Chapter Summary](#chapter-summary)

---

## 1. Type System Overview & Taxonomy

JOCKY implements a **statically typed, strongly checked, monomorphic type system** (with untyped generic collections in v0.1). Every expression and variable in JOCKY has a known, fixed type at compile time.

```
+-----------------------------------------------------------------------------+
|                             JOCKY TYPE HIERARCHY                            |
+-----------------------------------------------------------------------------+
|                                                                             |
|  +---------------------------+             +-----------------------------+  |
|  |     Value Types (Stack)   |             |   Reference Types (Heap)    |  |
|  +---------------------------+             +-----------------------------+  |
|  | int   (i64: 8 bytes)      |             | string   (UTF-8, immutable) |  |
|  | float (f64: 8 bytes)      |             | bytes    (Raw blob buffer)  |  |
|  | byte  (u8:  1 byte)       |             | list     (Dynamic vector)   |  |
|  | bool  (1 byte)            |             | map      (Hash table)       |  |
|  | void  (0 bytes)           |             | struct   (User-defined)     |  |
|  |                           |             | Error    (Diagnostic pair)  |  |
|  +---------------------------+             +-----------------------------+  |
|  (Can NEVER be nil)                        (Can be nil)                     |
+-----------------------------------------------------------------------------+
```

---

## 2. Primitive Numeric Types

### The `int` Type (64-Bit Signed Integer)

In JOCKY, integer calculations use a unified 64-bit signed representation (`int`), mapping directly to `int64_t` in C and `i64` in Rust.

- **Storage:** 8 bytes, two's complement.
- **Range:** $-9,223,372,036,854,775,808$ to $+9,223,372,036,854,775,807$ ($-2^{63}$ to $2^{63}-1$).
- **Default / Zero Value:** `0`.
- **Overflow Behavior:** Defined two's complement modular arithmetic wraparound in release mode; checked overflow in `--debug` builds.

```jocky
int pid = 1482;
int max_val = 9223372036854775807;
int memory_offset = 0x7FFF_FFFF_0000;
```

> [!NOTE]
> By unifying all integers to 64-bit `int`, JOCKY eliminates the classic C integer truncation vulnerabilities (e.g. `short` to `int` sign extension, `uint32` overflow) that plague forensic telemetry parsing.

---

### The `float` Type (64-Bit IEEE 754 Floating Point)

Floating-point operations use 64-bit double-precision IEEE 754 values, mapping to `double` in C and `f64` in Rust.

- **Storage:** 8 bytes (1 sign bit, 11 exponent bits, 52 fraction bits).
- **Range:** Approximately $\pm 1.7976931348623157 \times 10^{308}$.
- **Default / Zero Value:** `0.0`.
- **Special States:** Supports standard IEEE positive infinity (`+Inf`), negative infinity (`-Inf`), and Not-a-Number (`NaN`).

```jocky
float cpu_utilization = 84.75;
float time_delta = 0.00042;
```

---

### The `byte` Type (8-Bit Unsigned Integer)

The `byte` type represents an 8-bit unsigned octet, mapping directly to `uint8_t` in C and `u8` in Rust.

- **Storage:** 1 byte (8 bits).
- **Range:** `0` to `255` (`0x00` to `0xFF`).
- **Default / Zero Value:** `0`.
- **Primary Use:** Single byte offsets, ASCII characters, protocol opcodes, and bitwise mask operations.

```jocky
byte header = 'M';             // ASCII 'M' (0x4D)
byte opcode = 0x90;            // x86 NOP opcode
byte mask = 0b0000_1111;
```

---

### The `bool` Type (Boolean Logic)

The `bool` type represents logical truth states.

- **Storage:** 1 byte.
- **Valid Literals:** `true`, `false`.
- **Default / Zero Value:** `false`.

```jocky
bool is_elevated = true;
bool has_anomaly = false;
```

> [!WARNING]
> In JOCKY, numbers are **never truthy or falsy**. Writing `if count { ... }` or `if 1 { ... }` is a compile-time error (`E0005`). You must explicitly evaluate a boolean comparison: `if count > 0 { ... }`.

---

### The `void` Type

The `void` type signifies the complete absence of a value or return type. It cannot be used as a variable type and is only permitted as a function return specifier.

```jocky
fn print_separator() -> void {
    log.info("----------------------------------------");
}
```

---

## 3. Binary Data: The `bytes` Type

In digital forensics, analysts constantly interact with unformatted binary blobs: raw memory sectors, network packet payloads, encrypted registry values, and executable headers.

The `bytes` type represents a heap-allocated, mutable byte array with length tracking.

```jocky
// Initializing bytes from a hex literal
bytes pe_signature = x"4D5A90000300000004000000FFFF0000";

// Reading raw binary data from disk
auto payload, err = fs.read("/tmp/implant.bin");
if err != nil { return; }

// Inspecting length
int length = payload.len();

// Accessing individual bytes (0-indexed)
byte first_byte = payload[0]; // 0x4D ('M')
byte second_byte = payload[1]; // 0x5A ('Z')
```

### The YARA & Signature Connection
Hexadecimal byte literals (`x"..."`) allow direct copy-pasting of YARA hex strings and disassembler opcodes into JOCKY code:

```jocky
// Match Metasploit reverse TCP stager shellcode header
bytes msf_pattern = x"FC4883E4F0E8C0000000415141505251564831D2";
```

---

## 4. Textual Data: The `string` Type

The `string` type represents an **immutable sequence of UTF-8 encoded bytes**.

- **Immutability:** Once created, string buffers cannot be mutated in place. Concatenation (`+`) or slicing allocates a new string descriptor.
- **Zero Termination:** Internally, the runtime maintains null-terminated UTF-8 buffers, allowing zero-copy passage to underlying operating system APIs.
- **Equality Comparison (`==`):** Evaluates deep content equality (byte-by-byte comparison), not pointer identity.

```jocky
string case_name = "CASE-2026";
string suffix = "-NTRO";
string full_name = case_name + suffix; // "CASE-2026-NTRO"

if full_name == "CASE-2026-NTRO" {
    log.info("Case names match identically.");
}
```

### String Methods:
```jocky
string s = "svchost.exe";
int len = s.len();              // 11
bool has_exe = s.contains(".exe"); // true
string sub = s.slice(0, 7);     // "svchost"
```

---

## 5. Collection Types

### The `list` Type (Dynamic Array)

In JOCKY v0.1, `list` is a dynamically growing vector capable of storing arbitrary values (tagged unions).

```jocky
// Initializing a list
list pids = [1024, 2048, 4096];

// Appending elements
pids.append(8192);

// Indexing elements (0-indexed)
int first = (int)pids[0]; // 1024

// Iterating with for..in
for pid in pids {
    log.info("Processing PID: " + (string)pid);
}

// Length check
int total = pids.len(); // 4
```

---

### The `map` Type (Associative Key-Value Store)

In JOCKY v0.1, `map` is a dynamic hash table providing fast $O(1)$ key-value associations.

```jocky
// Initializing a map
map metadata = {
    "hostname": "DC-PRIMARY",
    "ip": "10.0.0.1",
    "is_compromised": true
};

// Accessing fields
string host = (string)metadata["hostname"];

// Mutating keys
metadata["triage_status"] = "COMPLETE";

// Checking key existence
if metadata.has("ip") {
    log.info("IP address present: " + (string)metadata["ip"]);
}
```

---

## 6. The `Error` Type & Diagnostic Semantics

The `Error` type is a structured reference type encapsulating diagnostic failure information:

```jocky
struct Error {
    int code;          // Numeric error code
    string message;    // Descriptive error message
}
```

Standard error constructor:
```jocky
fn validate_port(int port) -> (bool, Error) {
    if port < 1 || port > 65535 {
        return false, JKY_ERR("Invalid TCP port number: " + (string)port);
    }
    return true, nil;
}
```

---

## 7. The `nil` Value & Reference Safety

The literal keyword `nil` represents an unassigned, empty, or absent reference.

### Which Types Can Be Nil?
Only heap-allocated reference types may hold `nil`:
- `string`
- `bytes`
- `list`
- `map`
- `struct` instances (e.g. `CaseFile`, `ProcessInfo`)
- `Error`

### Which Types Can NEVER Be Nil?
Primitive stack value types can **never** be assigned `nil`:
- `int`
- `float`
- `bool`
- `byte`

```jocky
// Valid nil assignments:
string s = nil;
list l = nil;
Error err = nil;

// COMPILE ERROR: E0006: Cannot assign 'nil' to primitive value type 'int'
int invalid_number = nil;
```

### Nil Dereference Semantics

Attempting to access a field, call a method, or index into a `nil` reference triggers a controlled **runtime panic**:

```
[PANIC] [2026-08-24T02:30:00Z] FATAL: Nil pointer dereference in module 'main' at line 42.
Stack trace:
  -> main() [triage.jky:42]
  -> _jky_runtime_entry()
Process terminated with exit code 139.
```

---

## 8. Type Casting & Explicit Conversion Rules

JOCKY mandates explicit type casting via C-style prefix syntax: `(target_type)expression`.

| Source Type | Target Type | Syntax | Behavior / Semantic |
| :--- | :--- | :--- | :--- |
| `int` | `float` | `(float)i` | Converts integer to 64-bit IEEE floating point |
| `float` | `int` | `(int)f` | Truncates fractional part towards zero |
| `int` | `byte` | `(byte)i` | Masks lowest 8 bits (`i & 0xFF`) |
| `byte` | `int` | `(int)b` | Zero-extends 8-bit unsigned byte to 64-bit integer |
| `int` | `string` | `(string)i` | Formats integer to decimal ASCII string |
| `float` | `string` | `(string)f` | Formats float to standard decimal string |
| `bool` | `string` | `(string)b` | Yields `"true"` or `"false"` |
| `auto` | `ConcreteType`| `(ConcreteType)val` | Dynamic downcast from generic container |

```jocky
int count = 42;
string count_str = (string)count; // "42"

float ratio = 9.87;
int rounded = (int)ratio;         // 9

byte b = (byte)300;               // 300 & 0xFF = 44
```

---

## 9. The Absolute Prohibition of Implicit Conversions

In languages like C or JavaScript, implicit type coercions are responsible for countless security vulnerabilities and forensic logic bugs:

```c
// Dangerous C implicit coercion:
unsigned int len = -1;
if (len < 10) { ... } // Evaluates FALSE because -1 becomes 4,294,967,295!
```

```javascript
// Dangerous JavaScript implicit coercion:
"5" + 3 // Yields "53" (string)
"5" - 3 // Yields 2 (number)
```

In JOCKY, **no implicit conversions exist under any circumstances**:

```jocky
int a = 10;
float b = 20.5;

// COMPILE ERROR: E0007: Binary operator '+' cannot be applied to 'int' and 'float'
auto c = a + b;

// Correct explicit cast:
auto c = (float)a + b; // Result is float: 30.5
```

---

## 10. Chapter Summary

- **Prims vs References:** `int`, `float`, `byte`, `bool` are value types on the stack. `string`, `bytes`, `list`, `map`, `struct`, `Error` are heap references.
- **Nil Rules:** Only reference types can be `nil`. Value primitives can never be `nil`.
- **Binary Power:** `bytes` and `x"..."` hex literals provide native tooling for raw payload analysis and YARA signatures.
- **Zero Implicit Coercions:** Every type conversion must be explicitly stated using `(type)val`, preventing silent calculation bugs during forensic triage.

In the next chapter, **[Chapter 7: Variables, Constants, and Scoping](ch07_variables_and_scope.md)**, we examine variable declarations, `auto` inference, block lifetimes, and garbage collection integration.
