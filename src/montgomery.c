#include "common.h"

// R = 2^16. For q=3329, -q^{-1} mod R is 62209
#define QINV 62209 

int16_t montgomery_reduce(int32_t a) {
    int32_t u = (a * QINV) & 0xFFFF; // (a * q') mod R
    int32_t t = (a - u * Q) >> 16;   // (a - u*q) / R
    // Correction
    int32_t m = -((int32_t)(t < 0));
    t = t + (Q & m);
    return (int16_t)t;
}