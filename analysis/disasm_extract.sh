#!/bin/bash
BINARY=$1
FUNC=$2

# Disassemble (objdump for GCC/Clang)
objdump -d "$BINARY" > temp.asm

# Extract function body via symbol filtering
awk "/<${FUNC}>:/,/^$/" temp.asm > func.asm

# Count metrics
TOTAL=$(grep -cE '^\s+[0-9a-f]+:' func.asm)
BRANCH=$(grep -ciE '\bjmp|jne|je|jl|jg|jle|jge|jb|ja|call|ret' func.asm)
CMOV=$(grep -ciE 'cmov' func.asm)
IMUL=$(grep -ciE '\bimul\b' func.asm)
SHIFT=$(grep -ciE 'sar|shr|sal|shl' func.asm)
MEM=$(grep -ciE 'mov.*\[' func.asm)

echo "$BINARY,$TOTAL,$BRANCH,$CMOV,$IMUL,$SHIFT,$MEM"