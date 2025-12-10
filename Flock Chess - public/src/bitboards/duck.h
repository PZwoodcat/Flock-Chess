// duck.h
#pragma once
#include <cstdint>
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "magic.h"

// Lookup duck attacks using magic table
Bitboard duck_attacks(int sq, Bitboard occ);

// Initialization (implemented in duck.cpp)
void init_duck_magics();
