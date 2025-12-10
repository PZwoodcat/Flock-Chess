#include "movegen.h"
// ------------------------------------------------------------
// Helper: convert (rank,file) to bit index
// ------------------------------------------------------------
inline int sq_index(int rank, int file) {
    return rank * 8 + file;
}

// ------------------------------------------------------------
// Parse FEN into bitboards
// ------------------------------------------------------------

Bitboards parse_fen_bitboards(const std::string& fen)
{
    Bitboards bb;

    int rank = 7;
    int file = 0;

    for (char c : fen) {
        if (c == ' ') break;

        if (c == '/') {
            rank--;
            file = 0;
            continue;
        }

        if (isdigit(c)) {
            file += c - '0';
            continue;
        }

        int sq = sq_index(rank, file);
        uint64_t bit = 1ULL << sq;

        // Ensure entry exists for this piece type
        bb.pieceBoards[c] |= bit;

        // Add globally to occupancy
        bb.occupancy |= bit;

        file++;
    }

    return bb;
}


// ------------------------------------------------------------
// Your init_moves() function
// ------------------------------------------------------------
uint64_t init_moves() {
    // init_bishop_magics();
    // init_rook_magics();
    // init_duck_magics();

    uint64_t a = bishop_attacks(0, 0ULL);
    uint64_t b = rook_attacks(0, 0ULL);
    uint64_t c = duck_attacks(0, 0ULL);
    uint64_t d = bishop_attacks(32, 0ULL);
    uint64_t e = rook_attacks(32, 0ULL);
    uint64_t f = duck_attacks(32, 0ULL);
    uint64_t g = bishop_attacks(53, 0ULL);
    uint64_t h = rook_attacks(53, 0ULL);

    std::cout << a << " " << b << " " << c << " "
              << d << " " << e << " " << f << " "
              << g << " " << h << "\n";

    return a;
}

// ------------------------------------------------------------
// FUNCTION YOU REQUESTED:
// - Takes FEN + mode
// - If mode = "Flock", generate bitboards and call init_moves()
// ------------------------------------------------------------
// std::array<uint64_t, 64> generate_from_fen(const std::string& fen, const std::string& mode) {
//     std::array<uint64_t, 64> moves{}; // zero-initialized

//     if (mode == "Flock") {
//         Bitboards bb = parse_fen_bitboards(fen);

//         uint64_t occ = bb.occupancy;
//         uint64_t occ_copy = occ;

//         while (occ_copy) {
//             int sq = indexLSB(occ_copy);
//             uint64_t bit = 1ULL << sq;
//             occ_copy &= occ_copy - 1;

//             if (bb.bishops & bit)
//                 moves[sq] = bishop_attacks(sq, occ);
//             else if (bb.rooks & bit)
//                 moves[sq] = rook_attacks(sq, occ);
//             else if (bb.queens & bit)
//                 moves[sq] = rook_attacks(sq, occ) | bishop_attacks(sq, occ);
//             else if (bb.knights & bit)
//                 moves[sq] = knight_attacks(sq);
//             else if (bb.kings & bit)
//                 moves[sq] = king_attacks(sq);
//             else if (bb.pawns & bit) {
//                 bool isWhite = (sq / 8) < 4;
//                 moves[sq] = isWhite ? pawn_attacks_white(sq)
//                                     : pawn_attacks_black(sq);
//             }
//             else if (bb.ducks & bit)
//                 moves[sq] = duck_attacks(sq, occ);
//         }
//     }

//     return moves;
// }

