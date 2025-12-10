// magic.h
#pragma once
#include <cstdint>
#include <vector>
#include <random>
#include <cstring>
#include <iostream>
#include "bitutils.h"

// =====================================================
// Shared magic function used by all sliding pieces
// =====================================================
Bitboard find_magic(
    int square,
    Bitboard mask,
    Bitboard (*attacks)(int, Bitboard),
    int &outShift,
    std::vector<Bitboard> &outAttackTable
);
