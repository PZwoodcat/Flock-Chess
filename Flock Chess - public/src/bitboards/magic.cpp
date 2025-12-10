#include "magic.h"

// Enumerate all subsets of a mask
inline Bitboard next_subset(Bitboard subset, Bitboard mask) {
    // Caller convention: pass current subset (start with mask), returns next subset.
    return (subset - 1) & mask;
}

// ======================================================
// Random magic number generator (high entropy)
// ======================================================

Bitboard random_magic() {
    static std::mt19937_64 rng(0xC0FFEE123456789ULL);

    // Good magic numbers must have few low bits — but this generator is
    // already good enough for both rooks and bishops
    Bitboard candidate = 0;
    candidate = rng();
    candidate &= rng();
    candidate &= rng();
    return candidate;
}

// ======================================================
// Core: find_magic()
// mask = relevant occupancy mask
// attacks(subset) = function that computes attacks for a given blocker subset
// ======================================================

Bitboard find_magic(
    int square,
    Bitboard mask,
    Bitboard (*attacks)(int, Bitboard),   // user-supplied attack generator
    int &outShift,
    std::vector<Bitboard> &outAttackTable
) {
    int relevantBits = popcount(mask);
    int tableSize = 1 << relevantBits;

    // Temporary buffer for testing collisions
    std::vector<Bitboard> used(tableSize);

    // Precompute subsets
    std::vector<Bitboard> subsets;
    std::vector<Bitboard> moves;
    subsets.reserve(tableSize);
    moves.reserve(tableSize);

    Bitboard subset = mask;
    do {
        subsets.push_back(subset);
        moves.push_back(attacks(square, subset));
        subset = (subset - 1) & mask;
    } while (subset != mask); // stops after subset==0 has been processed

    // We want shift = 64 - relevantBits
    int shift = 64 - relevantBits;

    // Try random magics until one works
    while (true) {
        Bitboard magic = random_magic();

        // Magic numbers must satisfy this heuristic (used by Glaurung/Stockfish):
        if (popcount((magic * mask) & 0xFF00000000000000ULL) < 6)
            continue;

        std::fill(used.begin(), used.end(), 0ULL);
        bool failed = false;

        for (int i = 0; i < tableSize; i++) {
            Bitboard occ = subsets[i];
            int idx = (int)((occ * magic) >> shift);

            if (used[idx] == 0ULL)
                used[idx] = moves[i];
            else if (used[idx] != moves[i]) {
                failed = true;
                break;
            }
        }

        if (!failed) {
            // Found a good magic!
            outShift = shift;
            outAttackTable = used;
            return magic;
        }
    }
}
