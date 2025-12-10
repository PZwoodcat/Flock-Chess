#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <unordered_map>

extern struct Variant {
    std::string gameMode;               // [Section]

    std::vector<char> pieces;           // Pieces=KQRBNPD
    std::vector<std::string> movesets;  // Moveset=[16, 1+2+3, 1, 2, 3, 17]

    std::string effects;                // Effects=Flock, Quantum, Powerup (optional)
    std::string board;                  // Board=8x8
    std::string stdPos;                 // StdPos=...

    int move_num = 1;                   // Move_num=2  (optional, default = 1)
    int board_num = 1;                  // Board_num=2 (optional, default = 1)
};

parse(const std::string& path="./variants.ini")
test_parse(const std::string& path="./variants.ini")