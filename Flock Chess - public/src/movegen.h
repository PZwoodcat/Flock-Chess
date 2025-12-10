// movegen.h
#pragma once

#include "bitboards/bishops.h"
#include "bitboards/rook.h"
#include "bitboards/knight.h"
#include "bitboards/duck.h"
#include "bitboards/magic.h"
#include "bitutils.h"

#include <iostream>
#include <string>
#include <iostream>
#include <cstdint>
#include <vector>

struct Bitboards {
    Bitboard occupancy = 0ULL;
    std::unordered_map<char, Bitboard> pieceBoards; // keyed by piece letter
};

Bitboards parse_fen_bitboards(const std::string& fen);

// Initialize all move generators (magics, lookup tables, etc.)
int init_moves();