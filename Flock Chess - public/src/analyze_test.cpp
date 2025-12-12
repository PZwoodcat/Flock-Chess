#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "movegen.h"
#include "parser.h"

inline void print_bitboard_json(uint64_t bb) {
    fputc('[', stdout);

    bool first = true;

    while (bb) {
        int sq = indexLSB(bb);
        bb &= bb - 1;

        if (!first) fputc(',', stdout);
        fprintf(stdout, "%d", sq);
        first = false;
    }

    fputc(']', stdout);
}

void print_moves_json(const std::array<uint64_t,64>& moves) {
    fputc('[', stdout);

    for (int i = 0; i < 64; ++i) {
        print_bitboard_json(moves[i]);
        if (i != 63) fputc(',', stdout);
    }

    fputc(']', stdout);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: listener <a> <b>\n";
        return 1;
    }

    // int a = std::atoi(argv[1]);
    std::string fen = argv[1];
    std::string gameMode = argv[2];

    // std::cout << multiply(a, b);
    std::unordered_map<std::string, Variant> variants = parse("../variants.ini");
    
    Variant v;
    try {
        v = variants.at(gameMode);
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: Variant '" << gameMode << "' not found.\n"
                << e.what() << "\n";
        return 1;
    }

    Bitboards out = parse_fen_bitboards(fen);
    // print_Bitboards(out);
    std::array<uint64_t, 64> moves = movegen(out, v.movesets);

    print_moves_json(moves);
    return 0;
}