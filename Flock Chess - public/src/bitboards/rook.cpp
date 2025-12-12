#pragma once
#include "rook.h"

// ================== Rook Directions ==================
const int rookDirs[4][2] = {
    { 1,  0},   // north
    {-1,  0},   // south
    { 0,  1},   // east
    { 0, -1}    // west
};

// ================== Square index helper ==================
#define SQ(r,f) ((r)*8+(f))

// ================== Rook mask ==================
// Relevant occupancy mask (excluding edges)
Bitboard rook_mask(int sq) {
    int r = sq / 8;
    int f = sq % 8;
    Bitboard mask = 0ULL;

    // North
    for (int rr = r + 1; rr < 7; rr++) mask |= 1ULL << SQ(rr, f);
    // South
    for (int rr = r - 1; rr > 0; rr--) mask |= 1ULL << SQ(rr, f);
    // East
    for (int ff = f + 1; ff < 7; ff++) mask |= 1ULL << SQ(r, ff);
    // West
    for (int ff = f - 1; ff > 0; ff--) mask |= 1ULL << SQ(r, ff);

    return mask;
}


// ================== Compute attacks on the fly ==================
Bitboard rook_attacks_on_the_fly(int sq, Bitboard blockers) {
    Bitboard attacks = 0ULL;
    int r = sq / 8;
    int f = sq % 8;

    for (auto &d : rookDirs) {
        int rr = r + d[0];
        int ff = f + d[1];

        while (rr >= 0 && rr < 8 && ff >= 0 && ff < 8) {
            attacks |= 1ULL << SQ(rr, ff);
            if (blockers & (1ULL << SQ(rr, ff)))
                break;
            rr += d[0];
            ff += d[1];
        }
    }

    return attacks;
}

// ================== Rook Magic structure ==================
struct RookMagic {
    Bitboard mask;
    Bitboard magic;
    int shift;
    std::vector<Bitboard> attacks;
};

RookMagic rookMagics[64];

void print_rook_magic_constants() {
    std::cout << "static const RookMagic rookMagics[64] = {\n";

    for (int sq = 0; sq < 64; ++sq) {
        const RookMagic &M = rookMagics[sq];

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

bool write_rook_magics_to_file(const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        return false;
    }
    for (int sq = 0; sq < 64; ++sq) {
        RookMagic &M = rookMagics[sq];

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

// ================== Initialize rook magics ==================
void init_rook_magics() {
    for (int sq = 0; sq < 64; sq++) {
        RookMagic &M = rookMagics[sq];

        M.mask = rook_mask(sq);

        // Build magic table
        M.magic = find_magic(
            sq,
            M.mask,
            rook_attacks_on_the_fly,
            M.shift,
            M.attacks
        );
    }
    std::string filename = "rookMagics.bin";
    if (!write_rook_magics_to_file(filename)) {
        std::cout << "Failed to write rook magics to file!\n";
    }
}

inline bool file_exists(const std::string& filename) {
    std::ifstream in_r(filename, std::ios::binary);
    bool b = in_r.good();
    in_r.close();
    return in_r.good();
}

bool read_rook_magics_from_file(const std::string& filename) {
    if (!file_exists(filename)) {
        return false;
    }
    std::ifstream in(filename, std::ios::binary);
    for (int sq = 0; sq < 64; ++sq) {
        RookMagic &M = rookMagics[sq];

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
Bitboard rook_attacks(int sq, Bitboard occ) {
    std::string filename = "rookMagics.bin";

    if (!file_exists(filename)) {
        // std::cout << "File does not exist, creating it...\n";
        init_rook_magics();
    }
    if (!read_rook_magics_from_file(filename)) {
        std::cout << "Failed to read rook magics from file!\n";
        return 0ULL;
    }
    const RookMagic &M = rookMagics[sq];
    Bitboard blockers = occ & M.mask;
    Bitboard index = (blockers * M.magic) >> M.shift;
    return M.attacks[index];
}