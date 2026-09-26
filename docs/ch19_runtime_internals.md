# Chapter 19: Runtime Internals & Memory Management

> *"The runtime is the beating heart of an execution environment. In JOCKY, the runtime is engineered as a lightweight, zero-dependency C engine that provides automatic memory safety without the heavy signature of traditional virtual machines."*

---

## Table of Contents
1. [Runtime Architecture Overview](#runtime-architecture-overview)
2. [The `JkyVal` Tagged Union Representation](#the-jkyval-tagged-union-representation)
3. [Memory Management & The Mark-and-Sweep Garbage Collector](#memory-management--the-mark-and-sweep-garbage-collector)
   - [Object Header & Allocation Strategy](#object-header--allocation-strategy)
   - [GC Roots & Stack Tracking](#gc-roots--stack-tracking)
   - [Mark Phase & Sweep Phase](#mark-phase--sweep-phase)
   - [Collection Triggers & Allocation Thresholds](#collection-triggers--allocation-thresholds)
4. [Internal String Architecture (`JkyString`)](#internal-string-architecture-jkystring)
5. [Raw Byte Blobs & Hexadecimal Parsing (`JkyBytes`)](#raw-byte-blobs--hexadecimal-parsing-jkybytes)
6. [Dynamic Collections: `JkyList` and `JkyMap`](#dynamic-collections-jkylist-and-jkymap)
   - [`JkyList` Vector Growth Mechanics](#jkylist-vector-growth-mechanics)
   - [`JkyMap` Hash Table & Robin Hood Probing](#jkymap-hash-table--robin-hood-probing)
7. [The Evidence Container Internal Format](#the-evidence-container-internal-format)
   - [`manifest.json` Formal JSON Schema](#manifestjson-formal-json-schema)
   - [`sha256sums.txt` Generation](#sha256sumstxt-generation)
   - [`evidence.seal` HMAC-SHA256 Cryptographic Construction](#evidenceseal-hmac-sha256-cryptographic-construction)
8. [Low-Level Platform Abstraction Layer (PAL)](#low-level-platform-abstraction-layer-pal)
9. [Chapter Summary](#chapter-summary)

---

## 1. Runtime Architecture Overview

The JOCKY runtime (`runtime/jky_runtime.c`) is an embedded C library compiled directly into every generated binary. It operates with **zero external DLL dependencies**, providing core system abstractions, memory tracking, dynamic collections, and cryptographic hashing.

```
+-----------------------------------------------------------------------------+
|                          JOCKY RUNTIME ARCHITECTURE                         |
+-----------------------------------------------------------------------------+
|                                                                             |
|  +-----------------------------------------------------------------------+  |
|  | User JOCKY Code (Lowered to C)                                        |  |
|  +-----------------------------------------------------------------------+  |
|                                     |                                       |
|  +----------------------------------v------------------------------------+  |
|  | Tagged Union Engine (JkyVal) & Dynamic Type System                    |  |
|  +-----------------------------------------------------------------------+  |
|  | Mark-and-Sweep Garbage Collector (Object Tracking & Root Registry)    |  |
|  +-----------------------------------------------------------------------+  |
|  | Built-In Primitives: JkyString, JkyBytes, JkyList, JkyMap, JkyStruct  |  |
|  +-----------------------------------------------------------------------+  |
|  | Platform Abstraction Layer (PAL: Windows Win32/NTDLL vs. Linux POSIX) |  |
|  +-----------------------------------------------------------------------+  |
|                                                                             |
+-----------------------------------------------------------------------------+
```

---

## 2. The `JkyVal` Tagged Union Representation

In generic containers (`list`, `map`) and `auto` dynamic bindings, values are represented in the runtime as a 16-byte **Tagged Union (`JkyVal`)**:

```c
// runtime/jky_runtime.h
typedef enum {
    VAL_NIL = 0,
    VAL_INT,
    VAL_FLOAT,
    VAL_BOOL,
    VAL_BYTE,
    VAL_STRING,
    VAL_BYTES,
    VAL_LIST,
    VAL_MAP,
    VAL_STRUCT,
    VAL_ERROR
} JkyValType;

typedef struct {
    JkyValType type;     // 4 bytes (+ 4 bytes alignment padding)
    union {
        int64_t   as_int;
        double    as_float;
        uint8_t   as_byte;
        bool      as_bool;
        void     *as_obj; // Pointer to heap-allocated JkyHeader
    } as;                // 8 bytes
} JkyVal;
```

### Memory Footprint:
Every `JkyVal` occupies exactly 16 bytes on 64-bit architectures, enabling fast cache-line alignment and uniform array indexing.

---

## 3. Memory Management & The Mark-and-Sweep Garbage Collector

### Object Header & Allocation Strategy
Every heap-allocated object (`JkyString`, `JkyBytes`, `JkyList`, `JkyMap`, `JkyStruct`) begins with a standard `JkyHeader`:

```c
typedef struct JkyHeader {
    uint8_t           type;          // Object type tag
    uint8_t           marked;        // GC mark bit (0 = unmarked, 1 = reachable)
    uint16_t          flags;         // GC metadata flags
    uint32_t          size;          // Payload allocation size in bytes
    struct JkyHeader *next;          // Intrusive linked list of all active allocations
} JkyHeader;
```

---

### GC Roots & Stack Tracking

The runtime maintains a thread-local stack of GC roots. When a function creates local reference variables, it pushes their addresses to the root registry:

```c
// Emitted function prologue:
JkyGcFrame _gc_frame;
jky_gc_push_frame(&_gc_frame);

// Registering local reference variable:
JkyString *case_name = _jky_str_new("CASE-001");
jky_gc_register_root(&_gc_frame, (void**)&case_name);

// Emitted function epilogue:
jky_gc_pop_frame(&_gc_frame);
```

---

### Mark Phase & Sweep Phase

```
+-----------------------------------------------------------------------------+
|                         Mark-and-Sweep Cycle                                |
+-----------------------------------------------------------------------------+
| 1. MARK PHASE:                                                              |
|    - Scan all registered stack roots in active frames.                      |
|    - Recursively traverse child pointers in lists, maps, and structs.       |
|    - Set header->marked = 1 for all reachable nodes.                        |
|                                                                             |
| 2. SWEEP PHASE:                                                             |
|    - Iterate through the global intrusive allocation linked list.           |
|    - If header->marked == 0: Unlink and call free(header).                  |
|    - If header->marked == 1: Reset header->marked = 0 for next cycle.       |
+-----------------------------------------------------------------------------+
```

### Collection Triggers:
The garbage collector runs when total heap allocations exceed `gc_threshold` (default: 4 MB). Because forensic triage scripts are concise and short-lived, full GC cycles complete in under **100 microseconds**.

---

## 4. Internal String Architecture (`JkyString`)

```c
typedef struct {
    JkyHeader header;
    uint32_t  length;     // Character / byte count (excluding null terminator)
    uint32_t  hash;       // Cached 32-bit FNV-1a hash
    char      chars[];    // Null-terminated UTF-8 flexible array member
} JkyString;
```

- **Immutability:** Strings cannot be modified in place.
- **Fast Hashing:** The 32-bit FNV-1a hash is calculated at string construction and cached in `header.hash` for instant map lookups.

---

## 5. Raw Byte Blobs & Hexadecimal Parsing (`JkyBytes`)

```c
typedef struct {
    JkyHeader header;
    uint32_t  capacity;
    uint32_t  length;
    uint8_t   data[];     // Raw byte payload
} JkyBytes;
```

Hex literals (`x"4D5A9000"`) are parsed at compile time and emitted directly as raw byte arrays, eliminating runtime hex decoding overhead.

---

## 6. Dynamic Collections: `JkyList` and `JkyMap`

### `JkyList` Vector Growth Mechanics
`JkyList` represents a dynamically growing array of `JkyVal` items:
- Initial capacity: 8 elements.
- Growth factor: 2.0x amortized reallocation (`capacity = capacity * 2`).

### `JkyMap` Hash Table & Robin Hood Probing
`JkyMap` is implemented as an open-addressing hash table utilizing **Robin Hood linear probing** to minimize search variance and guarantee fast $O(1)$ lookups.

---

## 7. The Evidence Container Internal Format

When `case_file.seal()` is invoked, the runtime outputs an evidence directory containing three core files:

```
evidence_output/
├── manifest.json
├── sha256sums.txt
└── evidence.seal
```

---

### `manifest.json` Formal JSON Schema

```json
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "title": "JockyEvidenceManifest",
  "type": "object",
  "properties": {
    "case_id": { "type": "string" },
    "sealed_at": { "type": "string", "format": "date-time" },
    "collector": { "type": "string" },
    "build_salt": { "type": "string" },
    "host_hardware_id": { "type": "string" },
    "artifacts": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "name": { "type": "string" },
          "filename": { "type": "string" },
          "sha256": { "type": "string" },
          "size_bytes": { "type": "integer" }
        },
        "required": ["name", "filename", "sha256", "size_bytes"]
      }
    }
  },
  "required": ["case_id", "sealed_at", "collector", "build_salt", "artifacts"]
}
```

---

### `evidence.seal` HMAC-SHA256 Cryptographic Construction

$$\text{EvidenceSeal} = \text{HMAC-SHA256}(\text{Key} = \text{BuildSalt}, \; \text{Data} = \text{CanonicalBytes}(\text{manifest.json}))$$

---

## 8. Low-Level Platform Abstraction Layer (PAL)

The PAL bridges JOCKY standard library functions to native operating system APIs:

| Feature | Windows Implementation (`jky_win32.c`) | Linux Implementation (`jky_linux.c`) |
| :--- | :--- | :--- |
| **Process Listing** | `CreateToolhelp32Snapshot` / `NtQuerySystemInformation` | `/proc` directory scanning |
| **Host Info** | `GetComputerNameExW`, `RtlGetVersion` | `/proc/sys/kernel/hostname`, `uname()` |
| **Sockets** | `GetExtendedTcpTable`, `GetExtendedUdpTable` | `/proc/net/tcp`, `/proc/net/udp` |
| **File Hashing** | Win32 `CreateFileW`, internal SHA-256 | POSIX `open()`, internal SHA-256 |
| **Entropy Source**| `BCryptGenRandom` | `/dev/urandom` |

---

## 9. Chapter Summary

- **`JkyVal`:** 16-byte tagged union for efficient dynamic value representation.
- **Garbage Collector:** Fast mark-and-sweep collector with stack root tracking (<100µs cycles).
- **Core Structures:** Dedicated C structures for `JkyString`, `JkyBytes`, `JkyList`, and `JkyMap`.
- **Evidence Integrity:** Auditable `manifest.json` and HMAC-SHA256 cryptographic seal.
- **PAL:** Clean abstraction layer separating Linux POSIX interfaces from Windows NT internals.

In the next chapter, **[Chapter 20: Toolchain CLI & Compilation Reference](ch20_toolchain_reference.md)**, we cover every CLI flag, command, exit code, and environment variable in the `jky` toolchain.
