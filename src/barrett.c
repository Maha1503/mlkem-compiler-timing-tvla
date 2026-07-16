#include "common.h"

#define K 32
// Use 1ULL to prevent MSVC C4293 warning
static const uint32_t MU = (1ULL << K) / Q; 

int16_t barrett_reduce(int32_t a) {
    int32_t t = ((int64_t)a * MU) >> K;
    int32_t r = a - t * Q;
    // Correction
    int32_t m = -((int32_t)(r >= Q));
    r = r - (Q & m);
    m = -((int32_t)(r < 0));
    r = r + (Q & m);
    return (int16_t)r;
}