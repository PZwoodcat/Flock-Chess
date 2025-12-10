// rook.h
#pragma once
#include <cstdint>
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "magic.h"

Bitboard rook_attacks(int sq, Bitboard occ);

// Initialization (implemented in rook.cpp)
void init_rook_magics();