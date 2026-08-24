# Chapter 3: Installation & Environment Setup

> *"A reliable forensic toolchain must be trivial to construct from source in verified environments, resilient across target platforms, and entirely free of hidden upstream supply chain vulnerabilities."*

---

## Table of Contents
1. [System Prerequisites & Toolchain Dependencies](#system-prerequisites--toolchain-dependencies)
2. [Building the JOCKY Toolchain from Source](#building-the-jocky-toolchain-from-source)
   - [Building on Linux (Ubuntu/Debian/RHEL)](#building-on-linux-ubuntudebianrhel)
   - [Building on Windows (MSYS2/MinGW-w64 & Native Clang)](#building-on-windows-msys2mingw-w64--native-clang)
3. [Environment Configuration & PATH Setup](#environment-configuration--path-setup)
4. [Verifying the Installation](#verifying-the-installation)
5. [Cross-Compilation Setup](#cross-compilation-setup)
   - [Linux to Windows PE Cross-Targeting](#linux-to-windows-pe-cross-targeting)
   - [Windows to Linux ELF Cross-Targeting](#windows-to-linux-elf-cross-targeting)
6. [Containerized Hermetic Builds with Docker](#containerized-hermetic-builds-with-docker)
7. [Troubleshooting Common Build & Toolchain Issues](#troubleshooting-common-build--toolchain-issues)
8. [Chapter Summary](#chapter-summary)

---

## 1. System Prerequisites & Toolchain Dependencies

The JOCKY compiler (`jky`) is written in highly portable, standard ANSI C99/C11. It has been deliberately engineered to have **zero external third-party library dependencies** (e.g., no Boost, no LLVM link requirements in v0.1, no Python build scripts, no CMake overhead).

To build the `jky` compiler executable and compile JOCKY scripts into native agents, your host development workstation requires only a standard C compiler and standard build tools.

### Minimum Hardware Requirements
- **CPU:** x86_64 or ARM64 processor (1.0 GHz minimum, multi-core recommended for batch compilation)
- **RAM:** 512 MB minimum (2 GB recommended)
- **Disk Space:** 150 MB free disk space for the entire compiler, standard runtime headers, and documentation

### Supported Operating Systems & Backends

| Operating System | Recommended Host Compiler | Target Output Architecture |
| :--- | :--- | :--- |
| **Ubuntu Linux 20.04 / 22.04 / 24.04 LTS** | GCC 9.3+ or Clang 10+ | ELF 64-bit (`x86_64-linux-gnu`) |
| **Debian 11 / 12** | GCC 10+ or Clang 11+ | ELF 64-bit (`x86_64-linux-gnu`) |
| **RHEL / Rocky Linux / AlmaLinux 8 / 9** | GCC 8.5+ | ELF 64-bit (`x86_64-linux-gnu`) |
| **Windows 10 / 11 / Server 2016-2025** | MinGW-w64 GCC 10+ or LLVM Clang 12+ | PE 64-bit (`x86_64-pc-windows-msvc` / `gnu`) |
| **macOS (Darwin) 12+ (Analysis Host Only)** | Apple Clang (Xcode CLI Tools) | Mach-O 64-bit / Cross-PE / Cross-ELF |

---

## 2. Building the JOCKY Toolchain from Source

### Building on Linux (Ubuntu/Debian/RHEL)

#### Step 1: Install Build Tools
On Debian-based systems (Ubuntu, Debian, Kali Linux, Parrot OS):

```bash
sudo apt-get update
sudo apt-get install -y build-essential gcc clang make git
```

On RHEL-based systems (Fedora, Rocky Linux, AlmaLinux):

```bash
sudo dnf groupinstall -y "Development Tools"
sudo dnf install -y gcc clang make git
```

#### Step 2: Clone the JOCKY Repository
Clone the official NTRO JOCKY source tree to your local workspace:

```bash
git clone https://github.com/ntro-cyber/jocky.git
cd jocky
```

#### Step 3: Inspect Repository Structure
The canonical JOCKY source directory is organized as follows:

```
jocky/
├── Makefile                # Primary GNU Makefile
├── src/                    # Compiler frontend and CLI
│   ├── main.c              # CLI entry point (jky)
│   ├── lexer.c             # Lexical analyzer
│   ├── parser.c            # Recursive descent parser
│   ├── ast.c               # Arena AST allocator & nodes
│   ├── sema.c              # Two-pass semantic analyzer & symbol table
│   ├── codegen.c           # In-memory C transpiler & stealth mutator
│   ├── stealth.c           # 6-layer obfuscation & BLAKE2b crypto
│   └── util.c              # Memory arenas, string buffers, error diagnostics
├── runtime/                # Embedded runtime library
│   ├── jky_runtime.h       # Tagged union, runtime values, type headers
│   ├── jky_runtime.c       # GC, string allocator, list/map internals
│   ├── jky_host.c          # Platform host telemetry providers
│   ├── jky_process.c       # Process enumeration & memory inspection
│   ├── jky_network.c       # Network socket & interface enumerators
│   ├── jky_fs.c            # Forensic filesystem & hashing functions
│   ├── jky_evidence.c      # Evidence container & HMAC-SHA256 sealing
│   └── jky_crypto.c        # BLAKE2b, SHA-256, HMAC, and XOR engines
├── tests/                  # Unit and integration test suites
└── docs/                   # The JOCKY Language Book
```

#### Step 4: Compile the Toolchain
Run GNU `make` from the repository root:

```bash
make clean
make release
```

This compiles the `jky` binary using release optimization flags (`-O2`, `-s`, `-fstack-protector-strong`) and places the finished executable in `bin/jky`.

```bash
# Verify the build artifact
ls -lh bin/jky
# -rwxr-xr-x 1 analyst analyst 312K Aug 24 02:00 bin/jky
```

---

### Building on Windows (MSYS2/MinGW-w64 & Native Clang)

Building on Windows can be accomplished via **MSYS2 (MinGW-w64)** or **LLVM Clang for Windows**.

#### Method A: MSYS2 MinGW-w64 (Recommended)

1. Download and install MSYS2 from [https://www.msys2.org/](https://www.msys2.org/).
2. Open the **MSYS2 MINGW64** terminal.
3. Install the MinGW-w64 x86_64 toolchain and Make:

```bash
pacman -Syu
pacman -S --needed base-devel mingw-w64-x86_64-toolchain mingw-w64-x86_64-make git
```

4. Navigate to your JOCKY source directory:

```bash
cd /c/Users/LENOVO/Desktop/jocky
mingw32-make clean
mingw32-make release
```

5. The resulting executable is created at `bin/jky.exe`.

#### Method B: Native Windows PowerShell with Clang

If you have LLVM/Clang installed natively on Windows (via Visual Studio Installer or `winget install LLVM.LLVM`):

```powershell
# In PowerShell (Administrator or standard developer prompt)
cd C:\Users\LENOVO\Desktop\jocky

# Compile jky compiler executable directly
clang -O2 -std=c11 -Wall -Wextra -I./src -I./runtime `
    src/main.c src/lexer.c src/parser.c src/ast.c src/sema.c `
    src/codegen.c src/stealth.c src/util.c `
    -o bin/jky.exe

# Verify binary creation
Get-Item .\bin\jky.exe
```

---

## 3. Environment Configuration & PATH Setup

To invoke `jky` from any terminal or forensic workspace, add the `bin/` directory to your operating system's system PATH.

### Linux / Unix Shell Setup

Add the following line to your `~/.bashrc` or `~/.zshrc`:

```bash
export JOCKY_HOME="/opt/jocky"
export PATH="$JOCKY_HOME/bin:$PATH"
```

Apply the changes immediately:

```bash
source ~/.bashrc
```

### Windows System PATH Setup

Using PowerShell:

```powershell
# Set permanently for Current User
[Environment]::SetEnvironmentVariable(
    "Path",
    [Environment]::GetEnvironmentVariable("Path", "User") + ";C:\Users\LENOVO\Desktop\jocky\bin",
    "User"
)
```

Alternatively, configure via the Windows Graphical Interface:
1. Press `Win + R`, type `sysdm.cpl`, and press **Enter**.
2. Navigate to **Advanced** $\rightarrow$ **Environment Variables**.
3. Under **User variables**, select **Path** $\rightarrow$ **Edit** $\rightarrow$ **New**.
4. Paste the absolute path: `C:\Users\LENOVO\Desktop\jocky\bin`.
5. Click **OK** to save and restart your PowerShell or Command Prompt terminal.

---

## 4. Verifying the Installation

To verify that the JOCKY compiler and toolchain are correctly installed and accessible, execute the `version` command:

```bash
jky version
```

### Expected Output

```
JOCKY Toolchain v0.1.0-alpha (National Technical Research Organisation)
Target Architecture: x86_64-pc-windows-gnu (or x86_64-unknown-linux-gnu)
Compiler Backend: In-Memory Transpiler + Native Toolchain Integration
Stealth Engine: Enabled (BLAKE2b String XOR, CFG Flattening, Stack Randomization)
Build Date: 2026-08-24T02:00:00Z
Compiler Hash: 8f4a2b91c0e345f7
```

### Testing Compiler Diagnostics
Run `jky check` on a simple test script to ensure parser and semantic analysis passes succeed:

```bash
# Create a test script
echo 'import host; fn main() -> void { host.info(); }' > test_install.jky

# Perform semantic syntax verification
jky check test_install.jky
```

If the installation is healthy, `jky check` outputs:

```
[+] Syntax and semantic check passed: test_install.jky (0 errors, 0 warnings)
```

---

## 5. Cross-Compilation Setup

Forensic incident responders frequently author and compile collection agents on a hardened Linux analysis workstation, but must deploy the resulting executable to a compromised Windows domain controller (or vice versa).

JOCKY supports **cross-compilation targeting** via the `--target` flag.

```
+-------------------------------------------------------------------------+
|                       Cross-Compilation Architecture                    |
+-------------------------------------------------------------------------+
| [ Linux Analysis Host ]                                                 |
|   Source: triage.jky                                                    |
|     |                                                                   |
|   Command: jky compile triage.jky -o triage.exe                         |
|            --target x86_64-w64-mingw32                                  |
|     |                                                                   |
|     v                                                                   |
|   Transpiles in memory -> Pipes to x86_64-w64-mingw32-gcc via STDIN     |
|     |                                                                   |
|     v                                                                   |
|   Produces: triage.exe (Windows PE 64-bit Native Binary)                |
+-------------------------------------------------------------------------+
```

### Linux to Windows PE Cross-Targeting

To build Windows executables (`.exe`) from Ubuntu/Debian, install the MinGW-w64 cross-compiler:

```bash
sudo apt-get install -y gcc-mingw-w64-x86-64
```

Now compile your JOCKY script targeting Windows:

```bash
jky compile triage.jky -o triage.exe --target x86_64-w64-mingw32
```

### Windows to Linux ELF Cross-Targeting

To build Linux ELF binaries from a Windows host, install `clang` and configure standard sysroot targeting, or utilize the provided containerized workflow.

```powershell
jky compile triage.jky -o triage.elf --target x86_64-linux-gnu
```

---

## 6. Containerized Hermetic Builds with Docker

For sovereign agencies and enterprise security teams requiring mathematically reproducible compiler environments, JOCKY provides an official hermetic `Dockerfile`.

### The Hermetic `Dockerfile`

```dockerfile
# Multi-stage hermetic builder for JOCKY
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    clang \
    gcc-mingw-w64-x86-64 \
    make \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
COPY . .

RUN make clean && make release
RUN make test

# Final lightweight analysis image
FROM ubuntu:22.04 AS runner
RUN apt-get update && apt-get install -y \
    gcc \
    clang \
    gcc-mingw-w64-x86-64 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /build/bin/jky /usr/local/bin/jky
COPY --from=builder /build/runtime /usr/local/share/jky/runtime

WORKDIR /workspace
ENTRYPOINT ["jky"]
CMD ["help"]
```

### Building and Using the Container

```bash
# Build the hermetic JOCKY compiler image
docker build -t ntro/jky:v0.1 .

# Compile a local script to a Windows native agent via Docker
docker run --rm -v $(pwd):/workspace ntro/jky:v0.1 compile \
    triage.jky -o triage.exe --target x86_64-w64-mingw32
```

---

## 7. Troubleshooting Common Build & Toolchain Issues

### Issue 1: `fatal error: jky_runtime.h: No such file or directory`
- **Root Cause:** The `jky` compiler cannot locate the embedded runtime headers during the native C backend compilation step.
- **Solution:** Ensure the `JOCKY_HOME` environment variable is set to the root directory containing the `runtime/` folder, or place the `runtime/` directory alongside the `jky` binary.

### Issue 2: `gcc: error: unrecognized command-line option '-nostdlib'`
- **Root Cause:** An incompatible or non-standard backend compiler wrapper was selected.
- **Solution:** Verify your GCC/Clang version is GCC 9+ or Clang 10+. Run `gcc --version` and update if necessary.

### Issue 3: Windows Antivirus Flags `jky.exe` (The Compiler Itself)
- **Root Cause:** Because the `jky` compiler binary contains embedded AST mutators, string XOR decryptor stubs, and shellcode emission routines, aggressive local AV heuristics on the developer machine may misinterpret the compiler as a malware builder.
- **Solution:** Add the JOCKY development directory (`C:\Users\LENOVO\Desktop\jocky\bin`) to your local antivirus exclusion list on your analysis workstation.

> [!WARNING]
> Never compile forensic collection agents on an untrusted or suspect target machine. Always compile your `.jky` scripts on a secured, offline analyst workstation, and transfer only the final compiled native binary to the target.

---

## 8. Chapter Summary

- **Zero External Dependencies:** Building the JOCKY compiler requires only a standard C99 compiler (`gcc` or `clang`) and `make`.
- **Cross-Platform:** Native compilation is supported on Linux and Windows, with seamless cross-compilation support via `--target`.
- **Verification:** Always verify toolchain health with `jky version` and `jky check` before deploying collection agents.
- **Hermetic Docker Support:** Containerized compilation provides reproducible, supply-chain-hardened builds across security teams.

In the next chapter, **[Chapter 4: Quick Start: From Zero to Sealed Evidence](ch04_quick_start.md)**, we write and compile our very first JOCKY scripts, inspect evidence bundles, and observe compile-time polymorphism in action.
