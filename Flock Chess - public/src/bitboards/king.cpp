#include "king.h"

Bitboard kingAttacks[64];
bool kingAttacksInitialized = false;

static Bitboard king_mask(int sq) {
    Bitboard bb = 0ULL;
    int r = sq / 8;
    int f = sq % 8;

    for (int dr = -1; dr <= 1; dr++) {
        for (int df = -1; df <= 1; df++) {
            if (dr == 0 && df == 0) continue;

            int rr = r + dr;
            int ff = f + df;

            if (rr >= 0 && rr < 8 && ff >= 0 && ff < 8)
                bb |= 1ULL << (rr * 8 + ff);
        }
    }

    return bb;
}

void init_king_attacks() {
    for (int sq = 0; sq < 64; sq++)
        kingAttacks[sq] = king_mask(sq);

    kingAttacksInitialized = true;
}

Bitboard king_attacks(int sq, Bitboard occ) {
    (void)occ; // king attacks also ignore occupancy

    if (!kingAttacksInitialized)
        init_king_attacks();

    return kingAttacks[sq];
}
