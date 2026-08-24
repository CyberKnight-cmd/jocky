# Chapter 15: Standard Library Reference

> *"A domain-specific forensic language is only as powerful as its standard library. The JOCKY standard library provides an exhaustive, low-noise suite of native telemetry collectors, evidence bundlers, and cryptographic primitives."*

---

## Table of Contents
1. [Standard Library Architecture](#standard-library-architecture)
2. [Package `host`](#package-host)
   - [`host.info()`](#hostinfo)
   - [`host.os()`](#hostos)
   - [`host.arch()`](#hostarch)
   - [`host.hostname()`](#hosthostname)
   - [`host.uptime()`](#hostuptime)
3. [Package `process`](#package-process)
   - [`process.list()`](#processlist)
   - [`process.info(int pid)`](#processinfoint-pid)
   - [`process.modules(int pid)`](#processmodulesint-pid)
   - [`process.find(string name)`](#processfindstring-name)
4. [Package `network`](#package-network)
   - [`network.connections()`](#networkconnections)
   - [`network.interfaces()`](#networkinterfaces)
5. [Package `fs`](#package-fs)
   - [`fs.list(string path)`](#fsliststring-path)
   - [`fs.metadata(string path)`](#fsmetadatastring-path)
   - [`fs.hash(string path)`](#fshashstring-path)
   - [`fs.exists(string path)`](#fsexistsstring-path)
   - [`fs.read(string path)`](#fsreadstring-path)
6. [Package `evidence`](#package-evidence)
   - [`evidence.open(string case_id)`](#evidenceopenstring-case_id)
   - [`evidence.add(CaseFile cf, string name, auto data)`](#evidenceaddcasefile-cf-string-name-auto-data)
   - [`evidence.seal(CaseFile cf)`](#evidencesealcasefile-cf)
   - [`evidence.export(CaseFile cf, string path)`](#evidenceexportcasefile-cf-string-path)
7. [Package `report`](#package-report)
   - [`report.add(string key, auto val)`](#reportaddstring-key-auto-val)
   - [`report.save(string path)`](#reportsavestring-path)
   - [`report.json()`](#reportjson)
8. [Package `crypto`](#package-crypto)
   - [`crypto.sha256(bytes data)`](#cryptosha256bytes-data)
   - [`crypto.hmac(bytes key, bytes data)`](#cryptohmacbytes-key-bytes-data)
   - [`crypto.xor(bytes data, bytes key)`](#cryptoxorbytes-data-bytes-key)
9. [Package `log`](#package-log)
   - [`log.info(string msg)`](#loginfo_string-msg)
   - [`log.warn(string msg)`](#logwarnstring-msg)
   - [`log.error(string msg)`](#logerrorstring-msg)
10. [Chapter Summary](#chapter-summary)

---

## 1. Standard Library Architecture

The JOCKY standard library is built on three core design principles:
1. **Low-Noise Telemetry:** Collects system telemetry using native OS APIs or direct filesystem interfaces (`/proc` on Linux; `ntdll.dll` / `kernel32.dll` on Windows), bypassing noisy management services (WMI, PowerShell, shell spawning).
2. **Deterministic Return Values:** Every function returns either a strongly typed value, an Ok-tuple `(T, bool)`, or a Result tuple `(T, Error)`.
3. **Embedded Native Implementation:** Built-in standard packages do not require external runtime files or dynamic libraries; their C implementations are compiled directly into the binary agent.

---

## 2. Package `host`

The `host` package provides system-level telemetry, host identification, architecture metadata, and uptime metrics.

---

### `host.info()`
- **Signature:** `fn info() -> map`
- **Description:** Collects a comprehensive snapshot of host machine metadata in a single structured dictionary.
- **Parameters:** None.
- **Return Value:** `map` containing keys: `"hostname"`, `"os"`, `"arch"`, `"kernel_version"`, `"uptime_seconds"`, `"cpu_count"`.
- **Errors:** None. Always returns valid map.
- **Platform Notes:**
  - *Linux:* Parses `/proc/sys/kernel/hostname`, `/proc/version`, `/proc/uptime`.
  - *Windows:* Invokes `GetComputerNameExW`, `RtlGetVersion`, `GetTickCount64`.
- **Example:**
```jocky
import host;
import log;

auto sys = host.info();
log.info("Host: " + (string)sys["hostname"] + " on " + (string)sys["os"]);
```

---

### `host.os()`
- **Signature:** `fn os() -> string`
- **Description:** Returns the canonical operating system name.
- **Parameters:** None.
- **Return Value:** `string` (`"windows"`, `"linux"`, or `"darwin"`).
- **Errors:** None.
- **Example:**
```jocky
import host;

if host.os() == "windows" {
    log.info("Running on Windows endpoint");
}
```

---

### `host.arch()`
- **Signature:** `fn arch() -> string`
- **Description:** Returns the host CPU architecture.
- **Parameters:** None.
- **Return Value:** `string` (`"x86_64"`, `"arm64"`, or `"x86"`).
- **Errors:** None.
- **Example:**
```jocky
import host;

string architecture = host.arch();
```

---

### `host.hostname()`
- **Signature:** `fn hostname() -> string`
- **Description:** Returns the configured network hostname of the target endpoint.
- **Parameters:** None.
- **Return Value:** `string`.
- **Errors:** None.
- **Example:**
```jocky
import host;

string name = host.hostname();
```

---

### `host.uptime()`
- **Signature:** `fn uptime() -> int`
- **Description:** Returns total system uptime in seconds since last system boot.
- **Parameters:** None.
- **Return Value:** `int` (64-bit integer).
- **Errors:** None.
- **Example:**
```jocky
import host;

int uptime_secs = host.uptime();
int uptime_hours = uptime_secs / 3600;
```

---

## 3. Package `process`

The `process` package provides stealthy process enumeration, thread counting, module inspection, and anomaly discovery.

---

### `process.list()`
- **Signature:** `fn list() -> list`
- **Description:** Enumerates all active processes on the system and returns a list of `ProcessInfo` structs.
- **Parameters:** None.
- **Return Value:** `list` of `ProcessInfo` objects.
- **Struct Definition:**
```jocky
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
- **Errors:** None. Returns an empty list if enumeration fails.
- **Platform Notes:**
  - *Linux:* Reads `/proc/[pid]/stat`, `/proc/[pid]/cmdline`, `/proc/[pid]/exe`.
  - *Windows:* Uses `CreateToolhelp32Snapshot` or direct `NtQuerySystemInformation(SystemProcessInformation)`.
- **Example:**
```jocky
import process;
import log;

auto procs = process.list();
for p in procs {
    log.info("PID: " + (string)p.pid + " -> " + p.name);
}
```

---

### `process.info(int pid)`
- **Signature:** `fn info(int pid) -> (ProcessInfo, Error)`
- **Description:** Retrieves detailed forensic metadata for a single specific process identifier.
- **Parameters:** `int pid` — The target process ID.
- **Return Value:** `(ProcessInfo, Error)`.
- **Errors:** Returns `Error` if the PID does not exist or access is denied.
- **Example:**
```jocky
import process;

auto p_info, err = process.info(1024);
if err != nil {
    log.warn("Could not query PID 1024: " + err.message);
    return;
}
```

---

### `process.modules(int pid)`
- **Signature:** `fn modules(int pid) -> (list, Error)`
- **Description:** Enumerates all shared dynamic libraries / DLLs loaded into the target process memory space.
- **Parameters:** `int pid` — Target process ID.
- **Return Value:** `(list, Error)` where each list item is a `ModuleInfo` struct (`name`, `path`, `base_address`, `size`).
- **Errors:** Returns `Error` if process cannot be opened for query.
- **Example:**
```jocky
import process;

auto mods, err = process.modules(4912);
if err == nil {
    for m in mods {
        log.info("Loaded module: " + m.name + " at 0x" + (string)m.base_address);
    }
}
```

---

### `process.find(string name)`
- **Signature:** `fn find(string name) -> list`
- **Description:** Searches active processes and returns all instances matching the specified binary name.
- **Parameters:** `string name` — Target binary name (e.g. `"lsass.exe"`, `"sshd"`).
- **Return Value:** `list` of matching `ProcessInfo` structs.
- **Errors:** None. Returns empty list if no matches found.
- **Example:**
```jocky
import process;

auto matches = process.find("explorer.exe");
```

---

## 4. Package `network`

The `network` package enumerates active TCP/UDP socket connections and local network interfaces.

---

### `network.connections()`
- **Signature:** `fn connections() -> list`
- **Description:** Enumerates all active TCP and UDP network connections on the system.
- **Parameters:** None.
- **Return Value:** `list` of `ConnectionInfo` structs:
```jocky
struct ConnectionInfo {
    string local_ip;
    int local_port;
    string remote_ip;
    int remote_port;
    string state;      // "ESTABLISHED", "LISTENING", "TIME_WAIT"
    string protocol;   // "tcp", "udp"
    int pid;
}
```
- **Example:**
```jocky
import network;
import log;

auto conns = network.connections();
for c in conns {
    if c.remote_port == 4444 {
        log.warn("Suspicious connection: " + c.local_ip + " -> " + c.remote_ip + ":" + (string)c.remote_port);
    }
}
```

---

### `network.interfaces()`
- **Signature:** `fn interfaces() -> list`
- **Description:** Enumerates all local network interfaces and their configuration.
- **Parameters:** None.
- **Return Value:** `list` of `InterfaceInfo` structs (`name`, `mac_address`, `ip_addresses`, `is_up`).
- **Example:**
```jocky
import network;

auto ifaces = network.interfaces();
for iface in ifaces {
    log.info("Interface: " + iface.name + " MAC: " + iface.mac_address);
}
```

---

## 5. Package `fs`

The `fs` package provides forensic filesystem operations, cryptographic file hashing, directory traversal, and raw byte reads.

---

### `fs.list(string path)`
- **Signature:** `fn list(string path) -> (list, Error)`
- **Description:** Enumerates files and directories residing within the specified path.
- **Parameters:** `string path` — Target directory path.
- **Return Value:** `(list, Error)` where each entry is a `FileInfo` struct (`name`, `path`, `is_dir`, `size_bytes`, `modified_at`).
- **Example:**
```jocky
import fs;

auto files, err = fs.list("/etc");
if err == nil {
    for f in files {
        log.info("File: " + f.name + " (" + (string)f.size_bytes + " bytes)");
    }
}
```

---

### `fs.metadata(string path)`
- **Signature:** `fn metadata(string path) -> (FileInfo, Error)`
- **Description:** Retrieves timestamp and size metadata for a single file or directory.
- **Parameters:** `string path`.
- **Return Value:** `(FileInfo, Error)`.
- **Example:**
```jocky
import fs;

auto meta, err = fs.metadata(r"C:\Windows\System32\cmd.exe");
```

---

### `fs.hash(string path)`
- **Signature:** `fn hash(string path) -> (string, Error)`
- **Description:** Computes the SHA-256 cryptographic hash of a file on disk.
- **Parameters:** `string path`.
- **Return Value:** `(string, Error)` — 64-character lowercase hex SHA-256 digest.
- **Example:**
```jocky
import fs;

auto file_hash, err = fs.hash("/bin/bash");
if err == nil {
    log.info("SHA256: " + file_hash);
}
```

---

### `fs.exists(string path)`
- **Signature:** `fn exists(string path) -> bool`
- **Description:** Checks if a file or directory exists at the given path.
- **Parameters:** `string path`.
- **Return Value:** `bool`.
- **Example:**
```jocky
import fs;

if fs.exists("/var/log/auth.log") {
    log.info("Authentication log present.");
}
```

---

### `fs.read(string path)`
- **Signature:** `fn read(string path) -> (bytes, Error)`
- **Description:** Reads an entire file into a heap-allocated `bytes` buffer.
- **Parameters:** `string path`.
- **Return Value:** `(bytes, Error)`.
- **Example:**
```jocky
import fs;

auto data, err = fs.read("/etc/hosts");
if err == nil {
    log.info("Read " + (string)data.len() + " bytes.");
}
```

---

## 6. Package `evidence`

The `evidence` package manages the creation, artifact appending, cryptographic sealing, and exporting of forensic evidence bundles.

---

### `evidence.open(string case_id)`
- **Signature:** `fn open(string case_id) -> (CaseFile, Error)`
- **Description:** Initializes a new evidence container bound to a unique investigation Case ID.
- **Parameters:** `string case_id` — Alphanumeric case identifier.
- **Return Value:** `(CaseFile, Error)`.
- **Example:**
```jocky
import evidence;

auto cf, err = evidence.open("CASE-2026-ALPHA");
```

---

### `evidence.add(CaseFile cf, string name, auto data)`
- **Signature:** `fn add(CaseFile cf, string name, auto data) -> Error`
- **Description:** Serializes and appends an artifact to the evidence bundle, calculating its SHA-256 digest immediately.
- **Parameters:**
  - `CaseFile cf` — Target evidence bundle.
  - `string name` — Unique artifact identifier name.
  - `auto data` — Data payload (struct, list, map, bytes, string).
- **Return Value:** `Error` (`nil` on success; returns `Error` if container is already sealed).
- **Example:**
```jocky
evidence.add(cf, "host_telemetry", host.info());
```

---

### `evidence.seal(CaseFile cf)`
- **Signature:** `fn seal(CaseFile cf) -> Error` (Also available as method: `cf.seal()`)
- **Description:** Locks the container against further mutation and calculates the final HMAC-SHA256 integrity seal over `manifest.json`.
- **Parameters:** `CaseFile cf`.
- **Return Value:** `Error`.
- **Example:**
```jocky
cf.seal();
```

---

### `evidence.export(CaseFile cf, string path)`
- **Signature:** `fn export(CaseFile cf, string path) -> Error`
- **Description:** Writes the sealed evidence bundle to the designated filesystem path.
- **Parameters:**
  - `CaseFile cf`.
  - `string path` — Target directory path.
- **Return Value:** `Error`.
- **Example:**
```jocky
auto err = evidence.export(cf, "./bundle_output");
```

---

## 7. Package `report`

The `report` package provides key-value telemetry aggregation and standalone JSON report generation.

---

### `report.add(string key, auto val)`
- **Signature:** `fn add(string key, auto val) -> void`
- **Description:** Appends a key-value metric to the global telemetry report buffer.
- **Parameters:** `string key`, `auto val`.
- **Example:**
```jocky
import report;

report.add("scan_version", "1.0.4");
report.add("threat_level", "ELEVATED");
```

---

### `report.save(string path)`
- **Signature:** `fn save(string path) -> Error`
- **Description:** Serializes the global report to a JSON file on disk.
- **Parameters:** `string path`.
- **Return Value:** `Error`.
- **Example:**
```jocky
report.save("triage_report.json");
```

---

### `report.json()`
- **Signature:** `fn json() -> string`
- **Description:** Returns the global report buffer as a formatted JSON string in memory.
- **Return Value:** `string`.
- **Example:**
```jocky
string json_str = report.json();
```

---

## 8. Package `crypto`

The `crypto` package provides cryptographic hashing, HMAC integrity authentication, and low-level XOR operations.

---

### `crypto.sha256(bytes data)`
- **Signature:** `fn sha256(bytes data) -> bytes`
- **Description:** Computes the SHA-256 cryptographic hash over a byte buffer.
- **Parameters:** `bytes data`.
- **Return Value:** `bytes` (32 raw digest bytes).
- **Example:**
```jocky
import crypto;

bytes hash_bytes = crypto.sha256(x"4D5A9000");
```

---

### `crypto.hmac(bytes key, bytes data)`
- **Signature:** `fn hmac(bytes key, bytes data) -> bytes`
- **Description:** Computes an HMAC-SHA256 authentication tag over `data` using secret `key`.
- **Parameters:** `bytes key`, `bytes data`.
- **Return Value:** `bytes` (32 raw HMAC bytes).
- **Example:**
```jocky
bytes tag = crypto.hmac(secret_key, manifest_bytes);
```

---

### `crypto.xor(bytes data, bytes key)`
- **Signature:** `fn xor(bytes data, bytes key) -> bytes`
- **Description:** Performs multi-byte repeated-key XOR encryption/decryption on a byte buffer.
- **Parameters:** `bytes data`, `bytes key`.
- **Return Value:** `bytes`.
- **Example:**
```jocky
bytes encrypted = crypto.xor(raw_payload, x"3FA1BC");
```

---

## 9. Package `log`

The `log` package provides thread-safe, structured console logging.

---

### `log.info(string msg)`
- **Signature:** `fn info(string msg) -> void`
- **Description:** Emits an informational log message with an ISO 8601 timestamp.
- **Example:** `log.info("Triage started");`

---

### `log.warn(string msg)`
- **Signature:** `fn warn(string msg) -> void`
- **Description:** Emits a warning log message.
- **Example:** `log.warn("Process running with unexpected parent PID");`

---

### `log.error(string msg)`
- **Signature:** `fn error(string msg) -> void`
- **Description:** Emits an error log message to `stderr`.
- **Example:** `log.error("Unable to open device handle");`

---

## 10. Chapter Summary

- **`host`:** Complete system, platform, and uptime enumeration.
- **`process`:** In-memory process tables, modules, and process matching.
- **`network`:** Active socket connection tables and network interface metadata.
- **`fs`:** Safe file traversal, metadata querying, and SHA-256 calculation.
- **`evidence` & `report`:** Structured artifact aggregation and HMAC cryptographic sealing.
- **`crypto` & `log`:** Cryptographic building blocks and standardized operational logging.

In the next chapter, **[Chapter 16: Compiler Internals & Code Generation](ch16_compiler_internals.md)**, we examine the inner workings of the lexer, arena AST allocator, symbol resolver, and in-memory C compilation pipeline.
