#include "common.h"

int16_t naive_reduce(int32_t a) {
    int32_t r = a;
    // Hardened constant-time variant (branchless)
    int32_t m = -((int32_t)(r >= Q));
    r = r - (Q & m);
    return (int16_t)r;
}