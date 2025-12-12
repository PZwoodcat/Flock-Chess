// movegen.h
#pragma once

#include "bitboards/bishops.h"
#include "bitboards/rook.h"
#include "bitboards/knight.h"
#include "bitboards/duck.h"
#include "bitboards/king.h"
#include "bitboards/pawn.h"
#include "bitboards/magic.h"
#include "bitutils.h"

#include <unordered_map>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdint>
#include <array>
#include <vector>
#include <functional>
#include <random>
#include <chrono>

struct Zobrist {
    std::vector<std::array<uint64_t,64>> piece_square;
    std::vector<std::array<uint64_t,64>> quantum_square;
    std::array<uint64_t, 4> castling_rights;   // K Q k q
    std::array<uint64_t,8> enpassant_file;     // a–h
    uint64_t side_to_move;

    std::unordered_map<char,int> piece_idx;
};

struct Bitboards {
    Bitboard occupancy = 0ULL;
    Bitboard w_occupancy = 0ULL;
    Bitboard b_occupancy = 0ULL;
    std::unordered_map<char, Bitboard> pieceBoards; // keyed by piece letter
    std::vector<Bitboard> quantum_state;
    bool w_to_move = true;
    bool b_q_castle = true;
    bool b_k_castle = true;
    bool w_q_castle = true;
    bool w_k_castle = true;
    Bitboard enpassant_sq = 0ULL;
    int halfmove_clock = 0;
    int fullmove_number = 1;
    uint64_t zobrist_hash = 0ULL;
    std::unordered_map<uint64_t, int> zobrist_table;
};

inline void print_bitboard(Bitboard b)
{
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            int sq = rank * 8 + file;
            std::cout << ((b >> sq) & 1ULL);
        }
        std::cout << "\n";
    }
}

inline void print_Bitboards(const Bitboards& bb)
{
    std::cout << "=== Occupancy ===\n";
    print_bitboard(bb.occupancy);
    std::cout << "\n";
    std::cout << "=== White_Occupancy ===\n";
    print_bitboard(bb.w_occupancy);
    std::cout << "\n";
    std::cout << "=== Black_Occupancy ===\n";
    print_bitboard(bb.b_occupancy);
    std::cout << "\n";

    for (const auto& [piece, board] : bb.pieceBoards) {
        std::cout << "=== Piece " << piece << " ===\n";
        print_bitboard(board);
        std::cout << "\n";
    }
    // Quantum state (print list of bitboards)
    std::cout << "=== Quantum State ===\n";
    if (bb.quantum_state.empty()) {
        std::cout << "(empty)\n\n";
    } else {
        for (size_t i = 0; i < bb.quantum_state.size(); ++i) {
            std::cout << "State " << i << ":\n";
            print_bitboard(bb.quantum_state[i]);
            std::cout << "\n";
        }
    }
    // Side to move
    std::cout << "Side to move: " << (bb.w_to_move ? "White" : "Black") << "\n\n";

    // Castling rights
    std::cout << "Castling rights:\n";
    std::cout << "  White King-side  (K): " << (bb.w_k_castle ? "yes" : "no") << "\n";
    std::cout << "  White Queen-side (Q): " << (bb.w_q_castle ? "yes" : "no") << "\n";
    std::cout << "  Black King-side  (k): " << (bb.b_k_castle ? "yes" : "no") << "\n";
    std::cout << "  Black Queen-side (q): " << (bb.b_q_castle ? "yes" : "no") << "\n\n";

    // En-passant
    std::cout << "En-passant square:\n";
    if (bb.enpassant_sq == 0) {
        std::cout << "(none)\n\n";
    } else {
        print_bitboard(bb.enpassant_sq);
        std::cout << "\n";
    }

    // Clocks
    std::cout << "Halfmove clock: " << bb.halfmove_clock << "\n";
    std::cout << "Fullmove number: " << bb.fullmove_number << "\n\n";

    // Zobrist hash
    std::cout << "Zobrist hash: " << bb.zobrist_hash << "\n";
    for (const auto& [hash, reps] : bb.zobrist_table) {
        std::cout << "=== zobrist_hash " << hash << " ===\n";
        std::cout << reps << "\n";
    }
}

inline std::ostream& operator<<(std::ostream& os, const Bitboards& bb) {
    os << "Occupancy: " << bb.occupancy << "\n";
    os << "White_Occupancy: " << bb.w_occupancy << "\n";
    os << "Black_Occupancy: " << bb.b_occupancy << "\n";
    for (const auto& [piece, board] : bb.pieceBoards) {
        os << piece << ": " << board << "\n"; // assumes Bitboard is uint64_t
    }
    // Quantum state (print list of bitboards)
    os << "=== Quantum State ===\n";
    if (bb.quantum_state.empty()) {
        os << "(empty)\n\n";
    } else {
        for (size_t i = 0; i < bb.quantum_state.size(); ++i) {
            os << "State " << i << ":\n";
            for (int rank = 7; rank >= 0; --rank) {
                for (int file = 0; file < 8; ++file) {
                    int sq = rank * 8 + file;
                    os << ((bb.quantum_state[i] >> sq) & 1ULL);
                }
                os << "\n";
            }
            os << "\n";
        }
    }
    // Side to move
    os << "Side to move: " << (bb.w_to_move ? "White" : "Black") << "\n\n";

    // Castling rights
    os << "Castling rights:\n";
    os << "  White King-side  (K): " << (bb.w_k_castle ? "yes" : "no") << "\n";
    os << "  White Queen-side (Q): " << (bb.w_q_castle ? "yes" : "no") << "\n";
    os << "  Black King-side  (k): " << (bb.b_k_castle ? "yes" : "no") << "\n";
    os << "  Black Queen-side (q): " << (bb.b_q_castle ? "yes" : "no") << "\n\n";

    // En-passant
    os << "En-passant square:\n";
    if (bb.enpassant_sq == 0) {
        os << "(none)\n\n";
    } else {
        for (int rank = 7; rank >= 0; --rank) {
            for (int file = 0; file < 8; ++file) {
                int sq = rank * 8 + file;
                std::cout << ((bb.enpassant_sq >> sq) & 1ULL);
            }
            std::cout << "\n";
        }
        os << "\n";
    }

    // Clocks
    os << "Halfmove clock: " << bb.halfmove_clock << "\n";
    os << "Fullmove number: " << bb.fullmove_number << "\n\n";

    // Zobrist hash
    os << "Zobrist hash: " << bb.zobrist_hash << "\n";
    for (const auto& [hash, reps] : bb.zobrist_table) {
        os << "=== zobrist_hash " << hash << " ===\n";
        os << reps << "\n";
    }
    return os;
}

Bitboards parse_fen_bitboards(const std::string& fen);
std::array<uint64_t,64> movegen(const Bitboards& bb, const std::unordered_map<char,std::string>& piece_to_expr);
// std::array<uint64_t, 64> generate_from_fen(const std::string& fen, const std::string& mode);
// Initialize all move generators (magics, lookup tables, etc.)
uint64_t init_moves();