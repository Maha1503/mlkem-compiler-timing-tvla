#include <stdio.h>
#include <stdlib.h>
#include "common.h"

// Include primitives directly to prevent cross-module inlining
#include "naive.c"
#include "barrett.c"
#include "montgomery.c"

#ifndef PRIMITIVE
#error "PRIMITIVE must be defined (0=naive, 1=barrett, 2=montgomery)"
#endif

#ifndef MODE
#error "MODE must be defined (0=random, 1=fixed)"
#endif

#define NUM_SAMPLES 100000
#define WARMUP 10000

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <output_file>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "w");
    if (!f) {
        perror("fopen");
        return 1;
    }

    volatile int16_t sink;
    int32_t inputs[NUM_SAMPLES];

    // Prepare inputs based on MODE
    srand(42);
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        if (MODE == 1) {
            inputs[i] = 123456; // Fixed input
        } else {
            inputs[i] = rand() % 1000000; // Random input
        }
    }

    // Warmup loops
    for (int i = 0; i < WARMUP; ++i) {
        sink = naive_reduce(inputs[i % NUM_SAMPLES]);
        sink = barrett_reduce(inputs[i % NUM_SAMPLES]);
        sink = montgomery_reduce(inputs[i % NUM_SAMPLES]);
    }

    // Measurement loop
    for (int i = 0; i < NUM_SAMPLES; ++i) {
        uint64_t t0 = rdtscp_serialized();
        
        #if PRIMITIVE == 0
        sink = naive_reduce(inputs[i]);
        #elif PRIMITIVE == 1
        sink = barrett_reduce(inputs[i]);
        #elif PRIMITIVE == 2
        sink = montgomery_reduce(inputs[i]);
        #endif
        
        uint64_t t1 = rdtscp_serialized();
        // Cast to unsigned long long for MSVC compatibility
        fprintf(f, "%llu\n", (unsigned long long)(t1 - t0));
    }

    fclose(f);
    return 0;
}