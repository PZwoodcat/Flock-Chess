#include <iostream>
#include <cstdlib>
#include "multiply.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: listener <a> <b>\n";
        return 1;
    }

    int a = std::atoi(argv[1]);
    int b = std::atoi(argv[2]);

    std::cout << multiply(a, b);
    return 0;
}
