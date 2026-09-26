# Chapter 18: Real-World Forensic Workflows

> *"A programming language proves its worth on the operational front line. In this chapter, we explore six complete, production-ready JOCKY forensic scripts designed for rapid, stealthy incident triage and tactical evidence preservation."*

---

## Table of Contents
1. [Forensic Workflow Architecture](#forensic-workflow-architecture)
2. [Workflow 1: Basic Host Triage & Baseline Assessment](#workflow-1-basic-host-triage--baseline-assessment)
3. [Workflow 2: Covert Process Anomaly & Injection Detection](#workflow-2-covert-process-anomaly--injection-detection)
4. [Workflow 3: Network Exfiltration Indicator Hunting](#workflow-3-network-exfiltration-indicator-hunting)
5. [Workflow 4: Targeted Filesystem Artifact Collection & Integrity Hashing](#workflow-4-targeted-filesystem-artifact-collection--integrity-hashing)
6. [Workflow 5: Loaded Kernel & Userland Module Auditing](#workflow-5-loaded-kernel--userland-module-auditing)
7. [Workflow 6: Comprehensive Multi-Artifact Evidence Bundle](#workflow-6-comprehensive-multi-artifact-evidence-bundle)
8. [Evidence Verification & Judicial Packaging](#evidence-verification--judicial-packaging)
9. [Chapter Summary](#chapter-summary)

---

## 1. Forensic Workflow Architecture

When responding to an active cyber intrusion on a compromised host, the forensic investigator must balance three conflicting imperatives:
1. **Speed:** Collecting volatile indicators before an adversary terminates connections or deletes logs.
2. **Stealth:** Avoiding noisy API sequences that trigger resident EDR behavioral alarms.
3. **Integrity:** Ensuring that all collected telemetries are mathematically hashed and cryptographically sealed for chain-of-custody compliance.

JOCKY scripts combine high-level operational clarity with compiler-level stealth execution.

---

## 2. Workflow 1: Basic Host Triage & Baseline Assessment

### Purpose:
Establish an immediate baseline of the host operating system, hardware architecture, hostname, system uptime, and core environment configuration.

### JOCKY Source (`workflows/01_host_triage.jky`):
```jocky
import host;
import evidence;
import log;

fn main() -> void {
    log.info("Commencing Workflow 1: Host Triage & Baseline");

    // 1. Initialize case evidence container
    auto case_bundle, err = evidence.open("CASE-2026-BASELINE-01");
    if err != nil {
        log.error("Failed to initialize evidence container: " + err.message);
        return;
    }

    // 2. Query host metrics
    auto host_info = host.info();
    log.info("Target Endpoint: " + (string)host_info["hostname"]);
    log.info("Operating System: " + (string)host_info["os"] + " (" + (string)host_info["arch"] + ")");
    log.info("System Uptime: " + (string)host.uptime() + " seconds");

    // 3. Attach artifact to bundle
    auto add_err = evidence.add(case_bundle, "host_baseline", host_info);
    if add_err != nil {
        log.error("Failed to add host baseline artifact: " + add_err.message);
        return;
    }

    // 4. Seal evidence container
    case_bundle.seal();
    log.info("Workflow 1 concluded. Evidence container sealed.");
}
```

### Line-by-Line Technical Breakdown:
- **Lines 1–3:** Imports required packages (`host`, `evidence`, `log`).
- **Lines 8–12:** Opens a new cryptographic evidence container bound to case identifier `"CASE-2026-BASELINE-01"`. Returns `(CaseFile, Error)`.
- **Lines 15–18:** Invokes `host.info()` to extract hostname, OS, CPU count, and architecture.
- **Lines 21–25:** Records the `host_baseline` dictionary into the evidence container.
- **Line 28:** Invokes `.seal()`, computing SHA-256 digests and sealing the manifest with HMAC integrity.

### Sample Execution Output:
```
[INFO] [2026-08-24T03:00:01Z] Commencing Workflow 1: Host Triage & Baseline
[INFO] [2026-08-24T03:00:01Z] Target Endpoint: SEC-GATEWAY-01
[INFO] [2026-08-24T03:00:01Z] Operating System: linux (x86_64)
[INFO] [2026-08-24T03:00:01Z] System Uptime: 148920 seconds
[INFO] [2026-08-24T03:00:01Z] Workflow 1 concluded. Evidence container sealed.
```

---

## 3. Workflow 2: Covert Process Anomaly & Injection Detection

### Purpose:
Enumerate active processes and detect common intrusion indicators:
- Processes running from suspicious directories (`/tmp`, `%TEMP%`, `C:\Users\Public`).
- Parent-process spoofing or orphaned daemon anomalies (`parent_pid == 1` with elevated rights).
- Rogue binaries masquerading as critical system executables (`svchost.exe`, `lsass.exe`).

### JOCKY Source (`workflows/02_process_anomalies.jky`):
```jocky
import process;
import evidence;
import log;

fn is_suspicious_path(string path) -> bool {
    if path.contains("/tmp") || path.contains(r"C:\Users\Public") || path.contains(r"\AppData\Local\Temp") {
        return true;
    }
    return false;
}

fn main() -> void {
    log.info("Commencing Workflow 2: Process Anomaly Scan");

    auto cf, err = evidence.open("CASE-2026-PROC-AUDIT");
    if err != nil { return; }

    auto procs = process.list();
    list anomalies = [];

    for p in procs {
        bool suspicious = false;

        // Check 1: Suspicious execution path
        if is_suspicious_path(p.path) {
            log.warn("Anomalous path: " + p.name + " -> " + p.path);
            suspicious = true;
        }

        // Check 2: Masquerading system binary
        if p.name == "svchost.exe" && !p.path.contains(r"C:\Windows\System32") {
            log.warn("Rogue svchost binary: " + p.path);
            suspicious = true;
        }

        if suspicious {
            anomalies.append(p);
        }
    }

    log.info("Total anomalous processes identified: " + (string)anomalies.len());

    evidence.add(cf, "raw_process_list", procs);
    evidence.add(cf, "flagged_anomalies", anomalies);

    cf.seal();
    evidence.export(cf, "./proc_evidence");
}
```

---

## 4. Workflow 3: Network Exfiltration Indicator Hunting

### Purpose:
Scan active socket connections for outbound connections to suspicious ports, non-standard listening daemons, and potential Command and Control (C2) beacons.

### JOCKY Source (`workflows/03_network_hunt.jky`):
```jocky
import network;
import evidence;
import log;

fn is_threat_port(int port) -> bool {
    // Common adversary C2 and raw shell ports
    if port == 4444 || port == 1337 || port == 6667 || port == 8888 || port == 9001 {
        return true;
    }
    return false;
}

fn main() -> void {
    log.info("Commencing Workflow 3: Network Connection Hunt");

    auto cf, err = evidence.open("CASE-2026-NET-HUNT");
    if err != nil { return; }

    auto conns = network.connections();
    list flagged_conns = [];

    for c in conns {
        if c.state == "ESTABLISHED" && is_threat_port(c.remote_port) {
            log.warn("Active C2 beacon detected: " + c.local_ip + " -> " + c.remote_ip + ":" + (string)c.remote_port);
            flagged_conns.append(c);
        }
    }

    evidence.add(cf, "all_connections", conns);
    evidence.add(cf, "flagged_c2_sockets", flagged_conns);

    cf.seal();
    log.info("Network triage complete. Sockets archived.");
}
```

---

## 5. Workflow 4: Targeted Filesystem Artifact Collection & Integrity Hashing

### Purpose:
Traverse sensitive configuration and log directories, calculate SHA-256 digests of suspicious executables, and verify file integrity without alerting file-integrity monitors.

### JOCKY Source (`workflows/04_filesystem_triage.jky`):
```jocky
import fs;
import evidence;
import log;

fn main() -> void {
    log.info("Commencing Workflow 4: Filesystem Artifact Collector");

    auto cf, err = evidence.open("CASE-2026-FS-AUDIT");
    if err != nil { return; }

    list target_paths = [
        "/etc/passwd",
        "/etc/shadow",
        "/var/log/auth.log",
        r"C:\Windows\System32\drivers\etc\hosts"
    ];

    list file_records = [];

    for path in target_paths {
        if fs.exists(path) {
            auto file_hash, h_err = fs.hash(path);
            auto meta, m_err = fs.metadata(path);

            if h_err == nil && m_err == nil {
                log.info("Audited: " + path + " -> SHA256: " + file_hash);
                file_records.append({
                    "path": path,
                    "sha256": file_hash,
                    "size_bytes": meta.size_bytes
                });
            }
        } else {
            log.warn("Artifact absent: " + path);
        }
    }

    evidence.add(cf, "critical_file_audits", file_records);
    cf.seal();
}
```

---

## 6. Workflow 5: Loaded Kernel & Userland Module Auditing

### Purpose:
Inspect all dynamically loaded DLLs or shared `.so` libraries inside critical host processes to identify unbacked memory sections, reflective DLL injection, and unsigned third-party drivers.

### JOCKY Source (`workflows/05_module_audit.jky`):
```jocky
import process;
import evidence;
import log;

fn audit_process_modules(int target_pid, CaseFile cf) -> void {
    auto mods, err = process.modules(target_pid);
    if err != nil {
        log.warn("Unable to query modules for PID " + (string)target_pid);
        return;
    }

    log.info("Inspecting " + (string)mods.len() + " modules for PID " + (string)target_pid);
    evidence.add(cf, "modules_pid_" + (string)target_pid, mods);
}

fn main() -> void {
    log.info("Commencing Workflow 5: Loaded Module Audit");

    auto cf, err = evidence.open("CASE-2026-MODULES");
    if err != nil { return; }

    // Inspect critical system processes
    auto targets = process.find("lsass.exe");
    for t in targets {
        audit_process_modules(t.pid, cf);
    }

    cf.seal();
}
```

---

## 7. Workflow 6: Comprehensive Multi-Artifact Evidence Bundle

### Purpose:
Orchestrate an end-to-end multi-artifact investigation that combines host metadata, process hierarchies, active network connections, and filesystem hashes into a single master evidence bundle.

### JOCKY Source (`workflows/06_master_bundle.jky`):
```jocky
import host;
import process;
import network;
import fs;
import evidence;
import log;

fn main() -> void {
    log.info("=== JOCKY FULL SPECTRUM FORENSIC TRIAGE INITIATED ===");

    auto cf, err = evidence.open("CASE-2026-SOVEREIGN-MASTER");
    if err != nil {
        log.error("Fatal: Cannot initialize master evidence container: " + err.message);
        return;
    }

    // Step 1: Host Profile
    log.info("[1/4] Acquiring host hardware and OS profile...");
    evidence.add(cf, "host_info", host.info());

    // Step 2: Full Process Inventory
    log.info("[2/4] Capturing full process tree snapshot...");
    evidence.add(cf, "process_inventory", process.list());

    // Step 3: Network Socket State
    log.info("[3/4] Enumerating all active socket connections...");
    evidence.add(cf, "network_connections", network.connections());

    // Step 4: System Host File Integrity
    log.info("[4/4] Verifying network hosts mapping file integrity...");
    string hosts_file = (host.os() == "windows") ? 
        r"C:\Windows\System32\drivers\etc\hosts" : "/etc/hosts";

    if fs.exists(hosts_file) {
        auto h_hash, _ = fs.hash(hosts_file);
        evidence.add(cf, "hosts_file_hash", h_hash);
    }

    // Finalize: Cryptographic Seal & Export
    cf.seal();
    evidence.export(cf, "./master_evidence_bundle");

    log.info("=== MASTER TRIAGE COMPLETE: CONTAINER SEALED & EXPORTED ===");
}
```

---

## 8. Evidence Verification & Judicial Packaging

When Workflow 6 exports to `./master_evidence_bundle`, the resulting structure is cryptographically auditable:

```
master_evidence_bundle/
├── manifest.json            # Master artifact index & individual SHA-256 sums
├── host_info.json           # Host baseline data
├── process_inventory.json   # Process tree snapshot
├── network_connections.json # Active socket connections
├── hosts_file_hash.json     # Hash of system hosts file
├── sha256sums.txt           # Standard checksum list
└── evidence.seal            # HMAC-SHA256 digital seal
```

To verify the seal independently using standard command-line tools:

```bash
# Verify all individual files match manifest hashes
sha256sum -c sha256sums.txt
```

---

## 9. Chapter Summary

- **Workflow 1:** Basic host baseline assessment.
- **Workflow 2:** Process anomaly detection and path validation.
- **Workflow 3:** Outbound C2 socket hunting.
- **Workflow 4:** Filesystem artifact hashing and verification.
- **Workflow 5:** Loaded dynamic module auditing.
- **Workflow 6:** Unified multi-artifact master triage container.

In the next chapter, **[Chapter 19: Runtime Internals & Memory Management](ch19_runtime_internals.md)**, we examine the `JkyVal` tagged union representation, the mark-and-sweep garbage collector, and OS abstraction mechanics.
