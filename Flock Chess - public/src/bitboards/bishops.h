// bishop.h
#pragma once
#include <cstdint>
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "magic.h"

// Lookup bishop attacks (runtime)
Bitboard bishop_attacks(int sq, Bitboard occ);

// Initialization (implemented in bishop.cpp)
void init_bishop_magics();
