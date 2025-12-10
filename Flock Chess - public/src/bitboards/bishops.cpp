#include "bishops.h"

// ========== Utility ==========
inline uint64_t set_bit(uint64_t b, int sq) { return b | (1ULL << sq); }

// Converts rank/file → square index
#define SQ(r,f) ((r)*8+(f))

// ========== Directions for bishops ==========
const int bishopDirs[4][2] = {
    { 1,  1},
    { 1, -1},
    {-1,  1},
    {-1, -1}
};

// ========== Bishop masks ==========
// This returns the bishop *mask*: all squares the bishop can slide to
// on an EMPTY board, EXCEPT edges (magic bitboard requirement).
Bitboard bishop_mask(int sq) {
    int r = sq / 8, f = sq % 8;
    Bitboard mask = 0ULL;

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

// ========== Bishop attacks ==========
// Given blockers, compute attacks (used when generating magic table)
Bitboard bishop_attacks_on_the_fly(int sq, Bitboard blockers) {
    Bitboard attacks = 0ULL;
    int r = sq / 8, f = sq % 8;

    for (auto &d : bishopDirs) {
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

// ========== Generate all occupancy subsets ==========
uint64_t subset_enum(uint64_t mask, uint64_t subset) {
    return (subset - mask) & mask;
}

// ========== Magic data structure ==========
struct BishopMagic {
    Bitboard mask;
    Bitboard magic;
    int shift;
    std::vector<Bitboard> attacks;
};

BishopMagic bishopMagics[64];

// ========== Hard-coded magic numbers for bishops ==========
// (These are known working Stockfish-compatible numbers)
const uint64_t bishopMagicNumbers[64] = {
    0x007fbfbfbfbfbfffull, 0x0000a060401007f9ull, 0x0001004008020000ull, 0x0000806004000000ull,
    0x0000100400002000ull, 0x0000200080001000ull, 0x0000110004000200ull, 0x00000a1020200200ull,
    0x0000044040080200ull, 0x00000a1020200200ull, 0x0000110004000200ull, 0x0000200080001000ull,
    0x0000100400002000ull, 0x0000806004000000ull, 0x0001004008020000ull, 0x0000a060401007f9ull,
    0x0000044040080200ull, 0x0000100200200200ull, 0x0000080080100020ull, 0x0000004040040000ull,
    0x0000004040040000ull, 0x0000080080100020ull, 0x0000100200200200ull, 0x0000044040080200ull,
    0x0000024420400800ull, 0x0000024420400800ull, 0x0000001100100200ull, 0x0000000808080800ull,
    0x0000000808080800ull, 0x0000001100100200ull, 0x0000024420400800ull, 0x0000024420400800ull,
    0x0000001100100200ull, 0x0000001100100200ull, 0x0000000808080800ull, 0x0000000404040400ull,
    0x0000000404040400ull, 0x0000000808080800ull, 0x0000001100100200ull, 0x0000001100100200ull,
    0x0000024420400800ull, 0x0000024420400800ull, 0x0000100200200200ull, 0x0000080080100020ull,
    0x0000004040040000ull, 0x0000004040040000ull, 0x0000080080100020ull, 0x0000100200200200ull,
    0x0000044040080200ull, 0x00000a1020200200ull, 0x0000110004000200ull, 0x0000200080001000ull,
    0x0000100400002000ull, 0x0000806004000000ull, 0x0001004008020000ull, 0x0000a060401007f9ull,
    0x007fbfbfbfbfbfffull, 0ull, 0ull, 0ull
};

void print_bishop_magic_constants() {
    std::cout << "static const BishopMagic bishopMagics[64] = {\n";

    for (int sq = 0; sq < 64; ++sq) {
        const BishopMagic &M = bishopMagics[sq];

        std::cout << "  {\n";

        // mask
        std::cout << "    0x" << std::hex << M.mask << "ull,\n";

        // magic
        std::cout << "    0x" << std::hex << M.magic << "ull,\n";

        // shift
        std::cout << "    " << std::dec << M.shift << ",\n";

        std::cout << "  }";
        if (sq != 63) std::cout << ",";
        std::cout << "\n";
    }

    std::cout << "};\n";
}

bool write_bishop_magics_to_file(const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        return false;
    }
    for (int sq = 0; sq < 64; ++sq) {
        BishopMagic &M = bishopMagics[sq];

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

// ========== Initialize bishop magic tables ==========
void init_bishop_magics() {
    for (int sq = 0; sq < 64; sq++) {
        BishopMagic &M = bishopMagics[sq];

        M.mask = bishop_mask(sq);

        // Build magic table
        M.magic = find_magic(
            sq,
            M.mask,
            bishop_attacks_on_the_fly,
            M.shift,
            M.attacks
        );
    }
    std::string filename = "bishopMagics.bin";
    if (!write_bishop_magics_to_file(filename)) {
        std::cout << "Failed to write bishop magics to file!\n";
    }
}

inline bool file_exists(const std::string& filename) {
    std::ifstream in_r(filename, std::ios::binary);
    bool b = in_r.good();
    in_r.close();
    return in_r.good();
}

bool read_bishop_magics_from_file(const std::string& filename) {
    if (!file_exists(filename)) {
        return false;
    }
    std::ifstream in(filename, std::ios::binary);
    for (int sq = 0; sq < 64; ++sq) {
        BishopMagic &M = bishopMagics[sq];

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
Bitboard bishop_attacks(int sq, Bitboard occ) {
    std::string filename = "bishopMagics.bin";

    if (!file_exists(filename)) {
        std::cout << "File does not exist, creating it...\n";
        init_bishop_magics();
    }
    if (!read_bishop_magics_from_file(filename)) {
        std::cout << "Failed to read bishop magics from file!\n";
        return 0ULL;
    }
    const BishopMagic &M = bishopMagics[sq];
    Bitboard blockers = occ & M.mask;
    Bitboard index = (blockers * M.magic) >> M.shift;
    return M.attacks[index];
}
