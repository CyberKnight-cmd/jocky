# Chapter 9: Structs & Data Modeling

> *"Data structures in forensic engineering must reflect the reality of system telemetries: explicit, unencumbered by inheritance hierarchies, and directly serializable to cryptographically verifiable evidence formats."*

---

## Table of Contents
1. [Struct Definition Syntax & Field Layout](#struct-definition-syntax--field-layout)
2. [Instantiating Structs & Field Initialization](#instantiating-structs--field-initialization)
3. [Attaching Methods to Structs](#attaching-methods-to-structs)
4. [Nested Structs & Complex Data Modeling](#nested-structs--complex-data-modeling)
5. [Structs as Reference Types & Nil References](#structs-as-reference-types--nil-references)
6. [Why Go-Style Composition Over Classical OOP Inheritance](#why-go-style-composition-over-classical-oop-inheritance)
7. [Automatic JSON Serialization & Deserialization](#automatic-json-serialization--deserialization)
8. [Struct Memory Layout & Runtime Representation](#struct-memory-layout--runtime-representation)
9. [Chapter Summary](#chapter-summary)

---

## 1. Struct Definition Syntax & Field Layout

In JOCKY, a `struct` is a composite type that groups zero or more named fields under a unified type identifier. Structs are declared at module scope using the `struct` keyword, followed by the struct name and a list of field declarations enclosed in curly braces.

```jocky
// Defining a forensic process artifact struct
struct ProcessInfo {
    int pid;
    int parent_pid;
    string name;
    string path;
    string command_line;
    bool is_elevated;
    int thread_count;
}
```

### Struct Declaration Rules:
1. **Mandatory Semicolons on Fields:** Every field definition inside the struct must terminate with a semicolon (`;`).
2. **Distinct Field Names:** Field identifiers within the same struct must be unique.
3. **Valid Field Types:** Fields may be any primitive type (`int`, `float`, `byte`, `bool`), string, bytes blob, collection (`list`, `map`), or another defined struct type.

---

## 2. Instantiating Structs & Field Initialization

Structs are instantiated using named field syntax:

```jocky
// Struct initialization with named field assignments
ProcessInfo p = ProcessInfo {
    pid: 1044,
    parent_pid: 1,
    name: "sshd",
    path: "/usr/sbin/sshd",
    command_line: "/usr/sbin/sshd -D",
    is_elevated: true,
    thread_count: 4
};
```

### Accessing and Mutating Fields
Fields are accessed and updated using standard dot notation (`.`):

```jocky
// Reading fields
log.info("Inspecting process: " + p.name + " [PID: " + (string)p.pid + "]");

// Mutating fields
p.is_elevated = false;
p.thread_count = p.thread_count + 1;
```

---

## 3. Attaching Methods to Structs

Methods are functions associated with a specific struct type via a **receiver parameter** placed between the `fn` keyword and the method identifier:

```jocky
struct HostNetworkEndpoint {
    string ip_address;
    int port;
    string protocol;
    bool is_listening;
}

// Method receiver: (HostNetworkEndpoint ep)
fn (HostNetworkEndpoint ep) to_string() -> string {
    return ep.protocol + "://" + ep.ip_address + ":" + (string)ep.port;
}

fn (HostNetworkEndpoint ep) mark_active() -> void {
    ep.is_listening = true;
    log.info("Endpoint marked active: " + ep.to_string());
}
```

### Invoking Struct Methods:
```jocky
fn main() -> void {
    HostNetworkEndpoint endpoint = HostNetworkEndpoint {
        ip_address: "192.168.1.100",
        port: 443,
        protocol: "tcp",
        is_listening: false
    };

    // Invoking the method
    string desc = endpoint.to_string();
    log.info("Target: " + desc);

    endpoint.mark_active();
}
```

---

## 4. Nested Structs & Complex Data Modeling

Structs can contain other struct instances as fields, enabling the creation of rich hierarchical forensic models:

```jocky
struct MemoryRegion {
    int base_address;
    int region_size;
    string protection;
    bool is_executable;
}

struct DriverAnomaly {
    string driver_name;
    string file_path;
    string signature_signer;
    bool is_whitelisted;
    MemoryRegion mapped_memory; // Nested struct
}

fn inspect_driver() -> void {
    DriverAnomaly anomaly = DriverAnomaly {
        driver_name: "vulnerable_ioctl.sys",
        file_path: r"C:\Windows\System32\drivers\vulnerable_ioctl.sys",
        signature_signer: "Unknown Third Party",
        is_whitelisted: false,
        mapped_memory: MemoryRegion {
            base_address: 0xFFFF_8000_0000,
            region_size: 65536,
            protection: "PAGE_EXECUTE_READWRITE",
            is_executable: true
        }
    };

    if anomaly.mapped_memory.is_executable && !anomaly.is_whitelisted {
        log.warn("Unsigned executable driver mapped: " + anomaly.driver_name);
    }
}
```

---

## 5. Structs as Reference Types & Nil References

In JOCKY, struct variables hold **heap reference pointers** to their underlying struct descriptors:

- **Reference Semantics:** Assigning a struct variable to another variable copies the reference pointer, not the underlying memory buffer.
- **Nil Safety:** A struct reference may be `nil`.
- **Nil Dereference Safety:** Attempting to read or write a field on a `nil` struct instance triggers an immediate, controlled runtime panic.

```jocky
ProcessInfo active_proc = nil;

if active_proc == nil {
    log.info("No active process bound.");
}

// RUNTIME PANIC: Attempted to access field 'name' on nil struct 'ProcessInfo'
string n = active_proc.name;
```

---

## 6. Why Go-Style Composition Over Classical OOP Inheritance

Many traditional programming languages (C++, Java, C#) utilize object-oriented class hierarchies with inheritance (`class MalwareAnalyzer extends BaseCollector`).

JOCKY explicitly **rejects class-based inheritance in favor of Go-style composition and method receivers**:

```
+-----------------------------------------------------------------------------+
|                     Classical OOP vs. JOCKY Composition                     |
+-----------------------------------------------------------------------------+
| Classical OOP (C++/Java):                                                   |
|   Object ---> BaseClass ---> AbstractCollector ---> ProcessCollector        |
|   - Complex Virtual Method Tables (vtable) inserted in binary               |
|   - Fragile base-class problem                                              |
|   - Heavy binary signature easily fingerprinted by EDR memory scans         |
|                                                                             |
| JOCKY Composition:                                                          |
|   Struct + Concrete Method Receivers                                        |
|   - Flat, predictable C-struct in memory                                    |
|   - Zero vtable pointers; direct function call resolution                   |
|   - Transparent memory layout with zero hidden overhead                     |
+-----------------------------------------------------------------------------+
```

### Architectural Benefits:
1. **Zero Virtual Table Footprint:** Classical OOP objects require a hidden `_vptr` pointing to a virtual function table (`vtable`). Modern security solutions actively inspect memory for known vtable signatures. JOCKY structs are pure data containers without hidden metadata pointers.
2. **Predictable Memory Alignment:** JOCKY structs map 1:1 to C `struct` representations, enabling zero-copy passing to operating system APIs.
3. **Simplicity for Analysts:** Eliminates complex polymorphism, abstract classes, and constructor chaining.

---

## 7. Automatic JSON Serialization & Deserialization

A critical requirement of the NTRO forensic specification is that all collected artifacts must be seamlessly serializable to structured JSON tables for report generation and evidence sealing.

Every JOCKY struct automatically implements the `.json()` serialization method:

```jocky
struct HostTriageReport {
    string hostname;
    string os;
    int active_process_count;
    bool firewall_enabled;
}

fn export_report() -> void {
    HostTriageReport rep = HostTriageReport {
        hostname: "SEC-SERVER-04",
        os: "linux",
        active_process_count: 142,
        firewall_enabled: true
    };

    // Built-in JSON serialization method
    string json_payload = rep.json();
    log.info("JSON Output:\n" + json_payload);
}
```

### Serialized JSON Output:
```json
{
  "hostname": "SEC-SERVER-04",
  "os": "linux",
  "active_process_count": 142,
  "firewall_enabled": true
}
```

---

## 8. Struct Memory Layout & Runtime Representation

Under the hood, the JOCKY compiler lowers user-defined structs to memory-aligned C structures managed by the runtime heap allocator:

```c
// Generated C representation in memory:
typedef struct {
    JkyHeader header;         // GC metadata and struct type ID
    int64_t   pid;            // 8 bytes
    int64_t   parent_pid;     // 8 bytes
    JkyString *name;          // 8-byte pointer
    JkyString *path;          // 8-byte pointer
    JkyString *command_line;  // 8-byte pointer
    uint8_t   is_elevated;    // 1 byte (+ 7 bytes padding)
    int64_t   thread_count;   // 8 bytes
} JkyStruct_ProcessInfo;
```

---

## 9. Chapter Summary

- **Declarations:** Defined with `struct Name { Type field; }` using mandatory semicolons.
- **Methods:** Attached via receiver syntax `fn (StructName var) method() -> Type`.
- **References:** Structs are heap-allocated reference types; uninitialized references equal `nil`.
- **No Inheritance:** Pure composition over inheritance eliminates vtable memory signatures and improves binary stealth.
- **Forensic Serialization:** Native `.json()` conversion provides out-of-the-box structured reporting for all struct types.

In the next chapter, **[Chapter 10: Control Flow](ch10_control_flow.md)**, we cover branching, loops, loop control mechanics, and expression conditionals.
