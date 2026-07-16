#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define Q 3329

// Cycle-accurate timing wrapper
#if defined(_MSC_VER)
#include <intrin.h>
#pragma intrinsic(__rdtscp)
static inline uint64_t rdtscp_serialized() {
    unsigned int ui;
    _mm_lfence();
    uint64_t t = __rdtscp(&ui);
    _mm_lfence();
    return t;
}
#else
static inline uint64_t rdtscp_serialized() {
    uint32_t lo, hi;
    asm volatile (
        "lfence\n\t"
        "rdtscp\n\t"
        "mov %%edx, %1\n\t"
        "mov %%eax, %0\n\t"
        "lfence"
        : "=r"(lo), "=r"(hi)
        :: "rax", "rbx", "rcx", "rdx", "memory"
    );
    return ((uint64_t)hi << 32) | lo;
}
#endif

#endif