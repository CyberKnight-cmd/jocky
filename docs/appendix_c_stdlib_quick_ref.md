# Appendix C: Standard Library Quick Reference

> *"A compact, one-page desk reference of every standard library package, function signature, method receiver, and return type in JOCKY v0.1."*

---

## Table of Contents
1. [Package `host`](#package-host)
2. [Package `process`](#package-process)
3. [Package `network`](#package-network)
4. [Package `fs`](#package-fs)
5. [Package `evidence`](#package-evidence)
6. [Package `report`](#package-report)
7. [Package `crypto`](#package-crypto)
8. [Package `log`](#package-log)

---

## 1. Package `host`

| Function Signature | Return Type | Platform | One-Line Description |
| :--- | :--- | :--- | :--- |
| `host.info()` | `map` | All | Collects comprehensive OS, architecture, kernel, and CPU telemetry. |
| `host.os()` | `string` | All | Returns canonical operating system name (`"windows"`, `"linux"`, `"darwin"`). |
| `host.arch()` | `string` | All | Returns CPU architecture (`"x86_64"`, `"arm64"`, `"x86"`). |
| `host.hostname()` | `string` | All | Returns configured network hostname of the target endpoint. |
| `host.uptime()` | `int` | All | Returns total system uptime in seconds since last boot. |
| `host.sleep(int ms)` | `void` | All | Suspends execution thread for specified milliseconds. |
| `host.exit(int code)` | `void` | All | Terminates process immediately with specified exit code. |

---

## 2. Package `process`

| Function Signature | Return Type | Platform | One-Line Description |
| :--- | :--- | :--- | :--- |
| `process.list()` | `list` | All | Enumerates all active processes and returns a list of `ProcessInfo` structs. |
| `process.info(int pid)` | `(ProcessInfo, Error)` | All | Retrieves detailed telemetry for a specific process identifier. |
| `process.modules(int pid)` | `(list, Error)` | All | Enumerates all DLLs / shared libraries loaded into process memory space. |
| `process.find(string name)` | `list` | All | Searches running processes matching binary name (e.g. `"lsass.exe"`). |
| `process.read_memory(int pid, int addr, int len)` | `(bytes, Error)` | All | Reads raw memory bytes from target process space (requires `@privileged`). |

---

## 3. Package `network`

| Function Signature | Return Type | Platform | One-Line Description |
| :--- | :--- | :--- | :--- |
| `network.connections()` | `list` | All | Enumerates all active TCP and UDP socket connections (`ConnectionInfo`). |
| `network.interfaces()` | `list` | All | Enumerates local network interface adapters, MACs, and IP addresses. |

---

## 4. Package `fs`

| Function Signature | Return Type | Platform | One-Line Description |
| :--- | :--- | :--- | :--- |
| `fs.list(string path)` | `(list, Error)` | All | Enumerates files and subdirectories residing within specified path. |
| `fs.metadata(string path)` | `(FileInfo, Error)` | All | Retrieves size, modified timestamp, and attribute metadata. |
| `fs.hash(string path)` | `(string, Error)` | All | Computes lowercase hex SHA-256 cryptographic hash of target file. |
| `fs.exists(string path)` | `bool` | All | Checks if file or directory exists at specified path. |
| `fs.read(string path)` | `(bytes, Error)` | All | Reads entire binary content of target file into a `bytes` buffer. |
| `fs.read_lines(string path)` | `(list, Error)` | All | Reads text file line-by-line into a list of strings. |

---

## 5. Package `evidence`

| Function / Method Signature | Return Type | Platform | One-Line Description |
| :--- | :--- | :--- | :--- |
| `evidence.open(string case_id)` | `(CaseFile, Error)` | All | Initializes new evidence container bound to unique case identifier. |
| `evidence.add(CaseFile cf, string name, auto data)` | `Error` | All | Appends artifact to container and computes SHA-256 digest immediately. |
| `evidence.seal(CaseFile cf)` / `cf.seal()` | `Error` | All | Locks container and calculates final HMAC-SHA256 integrity seal. |
| `evidence.export(CaseFile cf, string path)` | `Error` | All | Writes sealed evidence bundle (`manifest.json`, `sha256sums.txt`) to disk. |

---

## 6. Package `report`

| Function Signature | Return Type | Platform | One-Line Description |
| :--- | :--- | :--- | :--- |
| `report.add(string key, auto val)` | `void` | All | Appends key-value telemetry entry to global report buffer. |
| `report.save(string path)` | `Error` | All | Serializes accumulated report buffer to JSON file on disk. |
| `report.json()` | `string` | All | Returns formatted JSON string representation of global report. |

---

## 7. Package `crypto`

| Function Signature | Return Type | Platform | One-Line Description |
| :--- | :--- | :--- | :--- |
| `crypto.sha256(bytes data)` | `bytes` | All | Computes raw 32-byte SHA-256 cryptographic digest. |
| `crypto.hmac(bytes key, bytes data)` | `bytes` | All | Computes 32-byte HMAC-SHA256 authentication tag. |
| `crypto.xor(bytes data, bytes key)` | `bytes` | All | Performs multi-byte repeated-key XOR encryption / decryption. |

---

## 8. Package `log`

| Function Signature | Return Type | Platform | One-Line Description |
| :--- | :--- | :--- | :--- |
| `log.info(string msg)` | `void` | All | Emits informational log message with ISO 8601 timestamp to `stdout`. |
| `log.warn(string msg)` | `void` | All | Emits warning log message with ISO 8601 timestamp to `stdout`. |
| `log.error(string msg)` | `void` | All | Emits error log message with ISO 8601 timestamp to `stderr`. |
