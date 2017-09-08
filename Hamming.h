#ifndef HAMMING_H
#define HAMMING_H

#include <cstdint>

inline int CpuHammingDistance(const uint64_t &a, const uint64_t &b) {
    return __builtin_popcountll(a ^ b);
}

__device__ inline int GpuHammingDistance(const uint64_t &a, const uint64_t &b) {
    return __popcll(a ^ b);
}

#endif //HAMMING_H