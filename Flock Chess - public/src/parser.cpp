#include "parser.h"

std::string loadFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    std::stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
}

std::vector<char> parsePieceList(const std::string& s) {
    std::vector<char> pieces;
    for (char c : s)
        if (isspace(c) || c == '+')
            continue;
        else
            pieces.push_back(c);
    return pieces;
}
std::vector<std::string> parseMovesetList(const std::string& s) {
    std::vector<std::string> moves;

    size_t a = s.find('[');
    size_t b = s.find(']');
    if (a == std::string::npos || b == std::string::npos)
        return moves;

    std::string inner = s.substr(a + 1, b - a - 1);

    std::stringstream ss(inner);
    std::string item;

    while (std::getline(ss, item, ',')) {
        // trim
        item.erase(0, item.find_first_not_of(" \t"));
        item.erase(item.find_last_not_of(" \t") + 1);
        moves.push_back(item);
    }
    return moves;
}

std::unordered_map<std::string, Variant>
parseVariantsINI(const std::string& iniText)
{
    std::unordered_map<std::string, Variant> variants;

    std::stringstream ss(iniText);
    std::string line;

    Variant* cur = nullptr;
    std::vector<std::string> pendingMoveset;
    bool movesetPending = false;

    while (std::getline(ss, line)) {
        // Trim
        line.erase(0, line.find_first_not_of(" \t"));
        if (line.empty() || line[0] == ';' || line[0] == '#')
            continue;

        // Section header
        if (line.front() == '[' && line.back() == ']') {
            std::string name = line.substr(1, line.size() - 2);
            variants[name] = Variant{};
            variants[name].gameMode = name;
            cur = &variants[name];
            continue;
        }

        if (!cur) continue;

        // key=value
        size_t eq = line.find('=');
        if (eq == std::string::npos)
            continue;

        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);

        // trim both
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        val.erase(0, val.find_first_not_of(" \t"));
        val.erase(val.find_last_not_of(" \t") + 1);

        // Assign to strict fields
        if (key == "Pieces") {
            cur->pieces = parsePieceList(val);

            // If moveset was seen earlier, build the map now
            if (movesetPending && !pendingMoveset.empty()) {
                if (pendingMoveset.size() != cur->pieces.size()) {
                    std::cerr << "Error: Mismatched Pieces and Moveset count in variant "
                              << cur->gameMode << "\n";
                } else {
                    for (size_t i = 0; i < cur->pieces.size(); ++i) {
                        cur->movesets[cur->pieces[i]] = pendingMoveset[i];
                    }
                }
                movesetPending = false;
            }
        }
        else if (key == "Moveset") {
            pendingMoveset = parseMovesetList(val);

            // If pieces already exist, build map immediately
            if (!cur->pieces.empty()) {
                if (pendingMoveset.size() != cur->pieces.size()) {
                    std::cerr << "Error: Mismatched Pieces and Moveset count in variant "
                              << cur->gameMode << "\n";
                } else {
                    for (size_t i = 0; i < cur->pieces.size(); ++i) {
                        cur->movesets[cur->pieces[i]] = pendingMoveset[i];
                    }
                }
            } else {
                movesetPending = true;  // wait until pieces appear
            }
        }
        else if (key == "Effects") {
            cur->effects = val;
        }
        else if (key == "Board") {
            cur->board = val;
        }
        else if (key == "StdPos") {
            cur->stdPos = val;
        }
        else if (key == "Move_num") {
            cur->move_num = std::stoi(val);
        }
        else if (key == "Board_num") {
            cur->board_num = std::stoi(val);
        }
        else {
            std::cerr << "Warning: Unknown key '" << key
                      << "' in variant '" << cur->gameMode << "'\n";
        }
    }
    return variants;
}

std::unordered_map<std::string, Variant>
parse(const std::string& path) {
    try {
        // 1. Load file
        std::string iniText = loadFile(path);

        // 2. Parse
        auto variants = parseVariantsINI(iniText);
        return variants;

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
    }
    return {};
}
std::unordered_map<std::string, Variant>
test_parse(const std::string& path) {
    try {
        // 1. Load file
        std::string iniText = loadFile(path);

        // 2. Parse
        auto variants = parseVariantsINI(iniText);

        // DEBUG: Print all variants
        for (auto& [name, v] : variants) {
            std::cout << "=== Variant: " << name << " ===\n";
            std::cout << "Pieces: ";
            for (char c : v.pieces) std::cout << c << " ";
            std::cout << "\nMovesets: ";
            int i = 0;
            for (const auto& [piece, mv] : v.movesets) {
                std::cout << piece << " = " << mv << " ";
                if (i++ % 8 == 0) std::cout << "\n";
            }
            std::cout << "\nEffects: " << v.effects << "\n";
            std::cout << "Board: " << v.board << "\n";
            std::cout << "StdPos: " << v.stdPos << "\n";
            std::cout << "Move_num: " << v.move_num << "\n";
            std::cout << "Board_num: " << v.board_num << "\n\n";
        }

        // Example: access Flock Chess
        if (variants.find("Flock-Chess") != variants.end()) {
            auto& flock = variants["Flock-Chess"];

            std::cout << "Pieces: ";
            for (char c : flock.pieces) std::cout << c << " ";

            std::cout << "\nMovesets:\n";
            for (const auto& [piece, mv] : flock.movesets) {
                std::cout << piece << " = " << mv << "\n";
            }

            std::cout << "Effects = " << flock.effects << "\n";
            std::cout << "Board   = " << flock.board << "\n";
            std::cout << "StdPos  = " << flock.stdPos << "\n";
            std::cout << "move_num  = " << flock.move_num << "\n";
            std::cout << "board_num  = " << flock.board_num << "\n";

        }
        return variants;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
    }
    return {};
}