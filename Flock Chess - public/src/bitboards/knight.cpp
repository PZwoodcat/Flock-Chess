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

void init_knight_attacks() {
    for (int sq = 0; sq < 64; sq++)
        knightAttacks[sq] = knight_mask(sq);
}

Bitboard get_knight_attacks(int sq) {
    return knightAttacks[sq];
}
