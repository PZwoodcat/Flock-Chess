#include "pawn.h"        // your header

Bitboard whitePawnAttacks[64];
Bitboard blackPawnAttacks[64];
bool pawnAttacksInitialized = false;

// A helper that generates a single pawn’s attacks given a square
static Bitboard white_pawn_mask(int sq) {
    Bitboard bb = 0ULL;
    int r = sq / 8;
    int f = sq % 8;

    if (r == 7) return 0ULL; // white pawns cannot exist on rank 8 theoretically

    if (f > 0)     bb |= 1ULL << (sq + 7); // up-left
    if (f < 7)     bb |= 1ULL << (sq + 9); // up-right

    return bb;
}

static Bitboard black_pawn_mask(int sq) {
    Bitboard bb = 0ULL;
    int r = sq / 8;
    int f = sq % 8;

    if (r == 0) return 0ULL; // black pawns cannot exist on rank 1 theoretically

    if (f > 0)     bb |= 1ULL << (sq - 9); // down-left
    if (f < 7)     bb |= 1ULL << (sq - 7); // down-right

    return bb;
}

// Initializes all 64 entries
void init_pawn_attacks() {
    for (int sq = 0; sq < 64; sq++) {
        whitePawnAttacks[sq] = white_pawn_mask(sq);
        blackPawnAttacks[sq] = black_pawn_mask(sq);
    }
    pawnAttacksInitialized = true;
}

// Query functions
Bitboard white_pawn_attacks(int sq, Bitboard occ) {
    (void)occ;  // ignored — pawn attacks do NOT depend on occupancy
    if (!pawnAttacksInitialized)
        init_pawn_attacks();
    return whitePawnAttacks[sq];
}

Bitboard black_pawn_attacks(int sq, Bitboard occ) {
    (void)occ;  
    if (!pawnAttacksInitialized)
        init_pawn_attacks();
    return blackPawnAttacks[sq];
}