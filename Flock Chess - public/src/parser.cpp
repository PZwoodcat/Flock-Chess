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
        if (!isspace(c))
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
        }
        else if (key == "Moveset") {
            cur->movesets = parseMovesetList(val);
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

parse(const std::string& path) {
    try {
        // 1. Load file
        std::string iniText = loadFile(path);

        // 2. Parse
        auto variants = parseVariantsINI(iniText);

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
    }
}
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
            for (auto& m : v.movesets) std::cout << m << " ";
            std::cout << "\nEffects: " << v.effects << "\n";
            std::cout << "Board: " << v.board << "\n";
            std::cout << "StdPos: " << v.stdPos << "\n";
            std::cout << "Move_num: " << v.move_num << "\n";
            std::cout << "Board_num: " << v.board_num << "\n\n";
        }

        // Example: access Flock Chess
        if (variants.contains("Flock-Chess")) {
            auto& flock = variants["Flock-Chess"];

            std::cout << "Pieces: ";
            for (char c : flock.pieces) std::cout << c << " ";

            std::cout << "\nMovesets:\n";
            for (auto& m : flock.movesets) std::cout << "  " << m << "\n";

            std::cout << "Effects = " << flock["Effects"] << "\n";
            std::cout << "Board   = " << flock["Board"] << "\n";
            std::cout << "StdPos  = " << flock["StdPos"] << "\n";
            std::cout << "move_num  = " << flock["move_num"] << "\n";
            std::cout << "board_num  = " << flock["board_num"] << "\n";

        }

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
    }
}