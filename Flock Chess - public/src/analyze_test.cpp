#include <iostream>
#include <cstdlib>
#include "movegen.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: listener <a>\n";
        return 1;
    }

    // int a = std::atoi(argv[1]);
    std::string& fen = argv[1];

    // std::cout << multiply(a, b);
    Bitboards out = parse_fen_bitboards(fen);
    std::cout << out;
    return 0;
}
