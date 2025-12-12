// duck.cpp
// Move generation for the "duck" piece:
// - moves like a bishop if the immediately adjacent diagonal square is empty
// - if the adjacent diagonal square is occupied, the duck may move to the first
//   empty square further along that same diagonal (if any).
//
// This file provides an on-the-fly attack generator (no magic tables).
// Style and helpers follow your bishop.cpp.

#include "duck.h"

// ========== Utility ==========
inline uint64_t set_bit(uint64_t b, int sq) { return b | (1ULL << sq); }

// Converts rank/file → square index
#define SQ(r,f) ((r)*8+(f))

// ========== Directions for bishops/ducks ==========
const int bishopDirs[4][2] = {
    { 1,  1},
    { 1, -1},
    {-1,  1},
    {-1, -1}
};

// ========== Duck mask ==========
/*
  We provide a duck_mask function similar to bishop_mask. This mask is useful
  if you want to reason about "relevant squares" — but note: the duck's
  attack set *may* depend on occupancy outside this mask (so classic magic
  bitboards built from the bishop_mask might be insufficient).
*/
Bitboard duck_mask(int sq) {
    int r = sq / 8, f = sq % 8;
    Bitboard mask = 0ULL;

    // We'll use the same "interior-only" mask convention as bishop_mask:
    // add diagonal squares but stop before edges.
    for (auto &d : bishopDirs) {
        int rr = r + d[0];
        int ff = f + d[1];
        while (rr > 0 && rr < 7 && ff > 0 && ff < 7) {
            mask |= 1ULL << SQ(rr, ff);
            rr += d[0];
            ff += d[1];
        }
    }
    return mask;
}

// ========== Duck attacks (on the fly) ==========
/*
  For each diagonal direction:
    - if the adjacent diagonal square is empty:
        behave like a sliding bishop in that direction (add every square up to
        a blocker or edge; stop when a blocker is reached and include that
        blocker square if you want captures -- here we include the blocker square).
    - else (adjacent diagonal square is occupied):
        scan outward from that blocker until you find the first empty square;
        add that single empty square (if it exists). Do NOT add squares beyond
        that first empty spot. If all squares after the adjacent blocker are
        occupied or off-board, that direction yields no moves.
*/
Bitboard duck_attacks_on_the_fly(int sq, Bitboard occ) {
    Bitboard attacks = 0ULL;
    int r = sq / 8, f = sq % 8;

    for (auto &d : bishopDirs) {
        int rr = r + d[0];
        int ff = f + d[1];

        // If adjacent square is off-board, skip this direction
        if (rr < 0 || rr >= 8 || ff < 0 || ff >= 8) continue;

        int adj_sq = SQ(rr, ff);
        Bitboard adj_bb = (1ULL << adj_sq);

        if ((occ & adj_bb) == 0ULL) {
            // Adjacent is empty => slide like a bishop
            while (rr >= 0 && rr < 8 && ff >= 0 && ff < 8) {
                int cur_sq = SQ(rr, ff);
                attacks |= (1ULL << cur_sq);
                if (occ & (1ULL << cur_sq)) {
                    // encountered a blocker - include it (capture possible) and stop
                    break;
                }
                rr += d[0];
                ff += d[1];
            }
        } else {
            // Adjacent is occupied => find first empty square further along
            rr += d[0];
            ff += d[1];
            while (rr >= 0 && rr < 8 && ff >= 0 && ff < 8) {
                int cur_sq = SQ(rr, ff);
                if ((occ & (1ULL << cur_sq)) == 0ULL) {
                    // first empty beyond the adjacent blocker — that's a legal landing square
                    attacks |= (1ULL << cur_sq);
                    break; // only the first empty square is allowed
                }
                // square occupied, keep scanning
                rr += d[0];
                ff += d[1];
            }
            // if loop ends without finding an empty square => no moves in this direction
        }
    }

    return attacks;
}

struct DuckMagic {
    Bitboard mask;
    Bitboard magic;
    int shift;
    std::vector<Bitboard> attacks;
};

DuckMagic duckMagics[64];

void print_duck_magic_constants() {
    std::cout << "static const DuckMagic duckMagics[64] = {\n";

    for (int sq = 0; sq < 64; ++sq) {
        const DuckMagic &M = duckMagics[sq];

        std::cout << "  {\n";

        // mask
        std::cout << "    0x" << std::hex << M.mask << "ull,\n";

        // magic
        std::cout << "    0x" << std::hex << M.magic << "ull,\n";

        // shift
        std::cout << "    " << std::dec << M.shift << ",\n";

        // attacks vector
        std::cout << "    {";
        for (size_t i = 0; i < M.attacks.size(); ++i) {
            std::cout << " 0x" << std::hex << M.attacks[i] << "ull";
            if (i + 1 != M.attacks.size()) std::cout << ",";
        }
        std::cout << " }\n";

        std::cout << "  }";
        if (sq != 63) std::cout << ",";
        std::cout << "\n";
    }

    std::cout << "};\n";
}

bool write_duck_magics_to_file(const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        return false;
    }
    for (int sq = 0; sq < 64; ++sq) {
        DuckMagic &M = duckMagics[sq];

        out.write((char*)&M.mask, sizeof(M.mask));
        out.write((char*)&M.magic, sizeof(M.magic));
        out.write((char*)&M.shift, sizeof(M.shift));

        uint64_t size = M.attacks.size();
        out.write((char*)&size, sizeof(size));
        out.write((char*)M.attacks.data(), size * sizeof(Bitboard));
    }
    out.close();
    return true;
}

void init_duck_magics() {
    for (int sq = 0; sq < 64; sq++) {
        DuckMagic &M = duckMagics[sq];

        M.mask = duck_mask(sq);

        // Build magic table
        M.magic = find_magic(
            sq,
            M.mask,
            duck_attacks_on_the_fly,
            M.shift,
            M.attacks
        );
    }
    std::string filename = "duckMagics.bin";
    if (!write_duck_magics_to_file(filename)) {
        std::cout << "Failed to write duck magics to file!\n";
    }
}

inline bool file_exists(const std::string& filename) {
    std::ifstream in_r(filename, std::ios::binary);
    bool b = in_r.good();
    in_r.close();
    return in_r.good();
}

bool read_duck_magics_from_file(const std::string& filename) {
    if (!file_exists(filename)) {
        return false;
    }
    std::ifstream in(filename, std::ios::binary);
    for (int sq = 0; sq < 64; ++sq) {
        DuckMagic &M = duckMagics[sq];

        in.read((char*)&M.mask, sizeof(M.mask));
        in.read((char*)&M.magic, sizeof(M.magic));
        in.read((char*)&M.shift, sizeof(M.shift));

        uint64_t size;
        in.read((char*)&size, sizeof(size));
        M.attacks.resize(size);
        in.read((char*)M.attacks.data(), size * sizeof(Bitboard));
    }
    in.close();
    return true;
}

// ================== Query function ==================
Bitboard duck_attacks(int sq, Bitboard occ) {
    std::string filename = "duckMagics.bin";

    if (!file_exists(filename)) {
        // std::cout << "File does not exist, creating it...\n";
        init_duck_magics();
    }
    if (!read_duck_magics_from_file(filename)) {
        std::cout << "Failed to read duck magics from file!\n";
        return 0ULL;
    }
    const DuckMagic &M = duckMagics[sq];
    Bitboard blockers = occ & M.mask;
    Bitboard index = (blockers * M.magic) >> M.shift;
    return M.attacks[index];
}

