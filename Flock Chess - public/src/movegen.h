// movegen.h
#pragma once

#include "bitboards/bishops.h"
#include "bitboards/rook.h"
#include "bitboards/knight.h"
#include "bitboards/duck.h"
#include "bitboards/magic.h"
#include "bitutils.h"

#include <unordered_map>
#include <iostream>
#include <string>
#include <iostream>
#include <cstdint>
#include <vector>

struct Bitboards {
    Bitboard occupancy = 0ULL;
    std::unordered_map<char, Bitboard> pieceBoards; // keyed by piece letter
};

inline std::ostream& operator<<(std::ostream& os, const Bitboards& bb) {
    os << "Occupancy: " << bb.occupancy << "\n";
    for (const auto& [piece, board] : bb.pieceBoards) {
        os << piece << ": " << board << "\n"; // assumes Bitboard is uint64_t
    }
    return os;
}

Bitboards parse_fen_bitboards(const std::string& fen);
// std::array<uint64_t, 64> generate_from_fen(const std::string& fen, const std::string& mode);
// Initialize all move generators (magics, lookup tables, etc.)
uint64_t init_moves();