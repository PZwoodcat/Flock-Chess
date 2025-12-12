#include "knight.h"

const int knightMoves[8][2] = {
    {  2,  1 }, {  2, -1 },
    { -2,  1 }, { -2, -1 },
    {  1,  2 }, {  1, -2 },
    { -1,  2 }, { -1, -2 }
};

Bitboard knight_mask(int sq) {
    int r = sq / 8;
    int f = sq % 8;

    Bitboard attacks = 0ULL;

    for (int i = 0; i < 8; i++) {
        int rr = r + knightMoves[i][0];
        int ff = f + knightMoves[i][1];

        if (rr >= 0 && rr < 8 && ff >= 0 && ff < 8) {
            attacks |= 1ULL << (rr * 8 + ff);
        }
    }

    return attacks;
}

Bitboard knightAttacks[64];
bool knightAttacksInitialized = false;

void init_knight_attacks() {
    for (int sq = 0; sq < 64; sq++)
        knightAttacks[sq] = knight_mask(sq);

    knightAttacksInitialized = true;
}

Bitboard knight_attacks(int sq, Bitboard occ)
{
    (void)occ; // knights jump; ignore occupancy

    if (!knightAttacksInitialized)
        init_knight_attacks();

    return knightAttacks[sq];
}