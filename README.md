# Empirical Evaluation of Compiler-Induced Timing Variability in ML-KEM Modular Arithmetic

This repository contains the code, measurement traces, and statistical analysis scripts to reproduce the empirical study of compiler-induced timing variability in constant-time modular arithmetic for ML-KEM (as specified in NIST FIPS 203). 

## Overview

While post-quantum cryptographic primitives like ML-KEM are mathematically secure, they can be vulnerable to microarchitectural timing side-channel attacks. Although implementations are often written to be "constant-time" at the source-code level, compilers (GCC, Clang, MSVC) can introduce optimizations (e.g., branch insertion, conditional moves, instruction reordering) that break these guarantees at the binary level.

This project evaluates three modular reduction primitives:
1. **Naive Conditional Reduction** (Hardened branchless variant)
2. **Barrett Reduction**
3. **Montgomery Reduction**

We evaluate these across multiple compiler configurations (GCC, Clang, MSVC with varying optimization flags) to determine if statistical timing leakage exists between fixed and random inputs using Test Vector Leakage Assessment (TVLA).

## Repository Structure

```
mlkem-tvla/
├── src/
│   ├── common.h           # RDTSCP timing wrapper and shared definitions
│   ├── naive.c            # Naive conditional reduction
│   ├── barrett.c          # Barrett reduction
│   ├── montgomery.c       # Montgomery reduction
│   └── harness.c          # Main measurement driver (LFENCE -> RDTSCP -> PRIM -> RDTSCP)
├── analysis/
│   ├── run_all_tvla.py    # Welch's t-test and Cohen's d calculation
│   ├── tvla.py            # Core TVLA logic for a single pair
│   └── disasm_extract.sh  # Bash script to extract instruction counts from objdump
├── bin/
│   ├── windows/           # Compiled Windows executables (.exe)
│   └── linux/             # Compiled Linux executables
├── traces/
│   ├── windows/           # Cycle-count CSVs generated on Windows
│   └── linux/             # Cycle-count CSVs generated on Linux
├── build_msvc.bat         # Batch script to build MSVC binaries
├── windows_results.csv    # Final TVLA verdicts for Windows
└── results_linux.csv      # Final TVLA verdicts for Linux
```

## Prerequisites

### Windows
- **MSVC:** Visual Studio 2022 Build Tools (Use `x64 Native Tools Command Prompt`)
- **GCC/Clang:** MSYS2 (MinGW-w64) or LLVM for Windows
- **Python:** 3.8+ with `numpy` and `scipy`

### Linux (or WSL)
- **Compilers:** `gcc`, `clang`
- **Python:** 3.8+ with `numpy` and `scipy`
- **Utils:** `binutils` (for `objdump`)

Install Linux dependencies via:
```bash
sudo apt update
sudo apt install gcc clang build-essential binutils python3-numpy python3-scipy python3-pandas
```

## Reproduction Instructions

### 1. Build the Binaries

**For Windows (MSVC):**
Open the `x64 Native Tools Command Prompt for VS 2022`, navigate to the project, and run:
```cmd
build_msvc.bat
```

**For Windows (GCC & Clang):**
Open `MSYS2 MinGW64` and run:
```bash
mkdir -p bin/windows traces/windows
for PRIM in 0 1 2; do
  for MODE in 0 1; do
    for CC in gcc clang; do
      for OPT in -O0 -Os -O3 -Ofast; do
        OPT_NAME=$(echo $OPT | sed 's/[^a-zA-Z0-9]//g')
        OUT="bin/windows/${CC}_${OPT_NAME}_p${PRIM}_m${MODE}.exe"
        $CC $OPT -m64 -g0 -DPRIMITIVE=$PRIM -DMODE=$MODE src/harness.c -o "$OUT"
      done
    done
  done
done
```

**For Linux (GCC & Clang):**
```bash
mkdir -p bin/linux traces/linux
for PRIM in 0 1 2; do
  for MODE in 0 1; do
    for CC in gcc clang; do
      for OPT in -O0 -Os -O3 -Ofast; do
        OPT_NAME=$(echo $OPT | sed 's/[^a-zA-Z0-9]//g')
        OUT="bin/linux/${CC}_${OPT_NAME}_p${PRIM}_m${MODE}"
        $CC $OPT -m64 -g0 -DPRIMITIVE=$PRIM -DMODE=$MODE src/harness.c -o "$OUT"
      done
    done
  done
done
```

### 2. Generate Timing Traces

Run the compiled binaries to generate the cycle-accurate `.csv` trace files. 

**On Windows (MSYS2):**
```bash
for bin_file in bin/windows/*.exe; do
    base_name=$(basename "$bin_file" .exe)
    trace_file="traces/windows/${base_name}.csv"
    ./$bin_file "$trace_file"
done
```

**On Linux (with CPU pinning for reduced noise):**
```bash
for bin_file in bin/linux/*; do
    base_name=$(basename "$bin_file")
    trace_file="traces/linux/${base_name}.csv"
    sudo taskset -c 1 nice -n -20 ./$bin_file "$trace_file"
done

# Fix file ownership if created with sudo
sudo chown -R $USER:$USER traces/linux
```

### 3. Run Statistical Leakage Assessment (TVLA)

Run the Python script to execute Welch's t-test comparing fixed (`m1`) vs random (`m0`) inputs. Pass the specific trace directory you wish to analyze.

```bash
# For Windows traces
python3 analysis/run_all_tvla.py traces/windows

# For Linux traces
python3 analysis/run_all_tvla.py traces/linux
```
This will generate a `results_windows.csv` or `results_linux.csv` file containing the absolute t-statistic (`|t|`), Cohen's d, and the PASS/FAIL verdict based on the `|t| > 4.5` threshold.

## Empirical Findings from Reproduction

Based on the generated traces across Windows and Linux environments, the following trends were observed in the data:

1. **Primitive Vulnerability:** Barrett Reduction (p1) and Naive Reduction (p0) consistently showed the highest frequency of timing leakage. Montgomery Reduction (p2) exhibited better stability on Linux but was still prone to leakage on Windows.
2. **Compiler Impact:**
   - On Windows, all three compilers (GCC, Clang, MSVC) exhibited significant leakage, with MSVC failing 10 out of 12 configurations.
   - On Linux, GCC and Clang were substantially more stable than on Windows. Clang passed 8 out of 12 configurations, while GCC passed 5 out of 12.
3. **Optimization Level:** 
   - On Windows, `-O0` and `-Os` frequently produced extreme leakage (e.g., Clang `-Os` yielded `|t| > 100` for all primitives). 
   - On Linux, `-O0` was generally stable (mostly PASS), while `-Os` and `-Ofast` in GCC were more prone to leakage, particularly for Barrett Reduction.
4. **Environment Impact:** The Windows environment exhibited substantially higher timing variability and leakage frequency (32 out of 36 failures) compared to the Linux environment with CPU pinning (11 out of 24 failures). This highlights the critical role of OS-level noise mitigation (such as `taskset` and `nice`) in microarchitectural timing experiments.
