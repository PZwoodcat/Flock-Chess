#pragma once
#include <cstdint>

using Bitboard = uint64_t;

#ifdef _MSC_VER
    #include <intrin.h>
#endif

// -------- Popcount --------
inline int popcount(Bitboard b) {
#ifdef _MSC_VER
    return static_cast<int>(__popcnt64(b));
#else
    return __builtin_popcountll(b);
#endif
}

// -------- Least significant bit index --------
inline int indexLSB(Bitboard b) {
#ifdef _MSC_VER
    unsigned long idx;
    _BitScanForward64(&idx, b);
    return static_cast<int>(idx);
#else
    return __builtin_ctzll(b);
#endif
}

inline int lsb(uint64_t x) {
#ifdef _MSC_VER
    unsigned long idx;
    _BitScanForward64(&idx, x);
    return idx;
#else
    return __builtin_ctzll(x);
#endif
}