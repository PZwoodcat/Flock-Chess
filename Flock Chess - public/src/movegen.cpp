#include "movegen.h"

using AttackFunc = Bitboard(*)(int sq, Bitboard occ);

std::unordered_map<int, AttackFunc> attack_map = {
    {1,  rook_attacks},
    {2,  bishop_attacks},
    {3,  knight_attacks},
    {16, king_attacks},
    {17, white_pawn_attacks},
    {19, duck_attacks},
    {20, black_pawn_attacks},
};

Bitboard run_attack(int code, int sq, Bitboard occ) {
    auto it = attack_map.find(code);
    if (it == attack_map.end())
        throw std::runtime_error("Unknown attack code: " + std::to_string(code));
    return it->second(sq, occ);
}

Bitboard evaluate_expr(const std::string& s, int sq, Bitboard occ)
{
    if (s.find('+') == std::string::npos) {
        // simple number
        return run_attack(std::stoi(s), sq, occ);
    }

    // a + b + c form
    std::stringstream ss(s);
    std::string token;
    Bitboard result = 0;
    while (std::getline(ss, token, '+')) {
        result ^= run_attack(std::stoi(token), sq, occ);
    }
    return result;
}

std::array<Bitboard, 64> piece_movegen(const Bitboards& bb, std::unordered_map<char, std::string> piece_to_expr, Bitboard occ)
{
    std::array<Bitboard, 64> moves{};

    Bitboard remaining = occ;

    while (remaining) {
        int sq = indexLSB(remaining);
        remaining &= remaining - 1;

        Bitboard bit = 1ULL << sq;

        // find which piece is on this square
        char found = 0;
        for (auto& [piece, board] : bb.pieceBoards) {
            if (board & bit) {
                found = piece;
                break;
            }
        }
        if (!found) continue;   // impossible but safe

        // get attack expression, e.g. "1+2", "3", "17"
        auto it = piece_to_expr.find(found);
        if (it == piece_to_expr.end())
            continue;

        const std::string& expr = it->second;

        Bitboard result = evaluate_expr(expr, sq, occ);

        moves[sq] = result;
    }

    // remove white pieces from moves
    for (int sq = 0; sq < 64; ++sq) {
        moves[sq] &= ~occ;
    }

    return moves;
}
std::array<Bitboard, 64> neutral_piece_movegen(const Bitboards& bb, std::unordered_map<char, std::string> piece_to_expr, Bitboard neutral_occ ,Bitboard occ)
{
    std::array<Bitboard, 64> moves{};

    Bitboard remaining = neutral_occ;

    while (remaining) {
        int sq = indexLSB(remaining);
        remaining &= remaining - 1;

        Bitboard bit = 1ULL << sq;

        // find which piece is on this square
        char found = 0;
        for (auto& [piece, board] : bb.pieceBoards) {
            if (board & bit) {
                found = piece;
                break;
            }
        }
        if (!found) continue;   // impossible but safe

        // get attack expression, e.g. "1+2", "3", "17"
        auto it = piece_to_expr.find(found);
        if (it == piece_to_expr.end())
            continue;

        const std::string& expr = it->second;

        Bitboard result = evaluate_expr(expr, sq, occ);

        moves[sq] = result;
    }

    // remove white pieces from moves
    for (int sq = 0; sq < 64; ++sq) {
        moves[sq] &= ~occ;
    }

    return moves;
}
std::array<Bitboard, 64> movegen(const Bitboards& bb, const std::unordered_map<char, std::string>& piece_to_expr) 
{
    std::array<Bitboard, 64> moves = piece_movegen(bb, piece_to_expr, bb.w_occupancy);

    auto black_moves = piece_movegen(bb, piece_to_expr, bb.b_occupancy);
    auto neutral_moves = neutral_piece_movegen(
        bb,
        piece_to_expr,
        bb.occupancy & ~(bb.w_occupancy | bb.b_occupancy),
        bb.occupancy
    );

    for (int i = 0; i < 64; i += 4) {
        moves[i+0] |= black_moves[i+0] | neutral_moves[i+0];
        moves[i+1] |= black_moves[i+1] | neutral_moves[i+1];
        moves[i+2] |= black_moves[i+2] | neutral_moves[i+2];
        moves[i+3] |= black_moves[i+3] | neutral_moves[i+3];
    }

    return moves;
}


std::array<Bitboard, 64> test_movegen(const Bitboards& bb, std::unordered_map<char, std::string> piece_to_expr)
{
    std::array<Bitboard, 64> moves{};
    Bitboard occ = bb.occupancy;

    Bitboard remaining = occ;

    while (remaining) {
        int sq = indexLSB(remaining);
        remaining &= remaining - 1;

        Bitboard bit = 1ULL << sq;

        // find which piece is on this square
        char found = 0;
        for (auto& [piece, board] : bb.pieceBoards) {
            if (board & bit) {
                found = piece;
                break;
            }
        }
        if (!found) continue;   // impossible but safe

        // get attack expression, e.g. "1+2", "3", "17"
        auto it = piece_to_expr.find(found);
        if (it == piece_to_expr.end())
            continue;

        const std::string& expr = it->second;

        Bitboard result = evaluate_expr(expr, sq, occ);

        moves[sq] = result;
    }

    return moves;
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

uint64_t rand64(std::mt19937_64 &rng) {
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);
    return dist(rng);
}

void init_zobrist(Zobrist &z,
                  const std::vector<char>& piece_list,
                  size_t num_quantum_layers)
{
    // Copy the piece list to the struct
    z.piece_idx.clear();
    for (size_t i = 0; i < piece_list.size(); ++i) {
        z.piece_idx[piece_list[i]] = static_cast<int>(i);
    }

    // Seed the RNG with high-resolution clock
    std::mt19937_64 rng(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()
    );

    z.piece_square.clear();
    z.piece_square.resize(piece_list.size());

    for (size_t i = 0; i < piece_list.size(); ++i) {
        for (int sq = 0; sq < 64; ++sq) {
            z.piece_square[i][sq] = rand64(rng);
        }
    }
    z.quantum_square.clear();
    z.quantum_square.resize(num_quantum_layers);

    for (size_t layer = 0; layer < num_quantum_layers; ++layer) {
        for (int sq = 0; sq < 64; ++sq) {
            z.quantum_square[layer][sq] = rand64(rng);
        }
    }

    for (int i = 0; i < 4; ++i)
        z.castling_rights[i] = rand64(rng);
    for (int f = 0; f < 8; ++f)
        z.enpassant_file[f] = rand64(rng);

    z.side_to_move = rand64(rng);  // IMPORTANT: this must be a 64-bit number
}


uint64_t compute_zobrist(const Bitboards& bb, const Zobrist& table) {
    uint64_t hash = 0ULL;

    for (const auto& [piece, board] : bb.pieceBoards) {
        int idx = table.piece_idx.at(piece);
        Bitboard b = board;
        while (b) {
            int sq = indexLSB(b);
            b &= b-1;
            hash ^= table.piece_square[idx][sq];
        }
    }

    for (size_t qi = 0; qi < bb.quantum_state.size(); ++qi) {

        // skip if table doesn't have quantum hash for that index
        if (qi >= table.quantum_square.size()) {
            std::cout << "Warning: Zobrist table missing quantum index " << qi << "\n";
            continue;
        }

        Bitboard qb = bb.quantum_state[qi];

        while (qb) {
            int sq = indexLSB(qb);
            qb &= qb - 1;
            hash ^= table.quantum_square[qi][sq];
        }
    }


    // side to move
    if (!bb.w_to_move) hash ^= table.side_to_move;

    // castling rights
    if (bb.w_k_castle) hash ^= table.castling_rights[0];
    if (bb.w_q_castle) hash ^= table.castling_rights[1];
    if (bb.b_k_castle) hash ^= table.castling_rights[2];
    if (bb.b_q_castle) hash ^= table.castling_rights[3];

    // en passant file
    if (bb.enpassant_sq) {
        int file = indexLSB(bb.enpassant_sq) % 8;
        hash ^= table.enpassant_file[file];
    }

    return hash;
}

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
    bool neutral = false;

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

        if (c == '+') {
            neutral = true;
            continue;
        }

        int sq = sq_index(rank, file);
        uint64_t bit = 1ULL << sq;

        // Ensure entry exists for this piece type
        bb.pieceBoards[c] |= bit;

        // Add globally to occupancy
        bb.occupancy |= bit;
        if (!neutral) {
            // Neutral pieces go to both sides' occupancy
            bb.w_occupancy |= bit;
            bb.b_occupancy |= bit;
        }
        else {
            neutral = false;
        }

        file++;
    }

    return bb;
}

