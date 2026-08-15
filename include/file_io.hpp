/* 
 * File I/O
**/

#pragma once

#include <fstream>
#include <string>

#include "art.hpp"


// loads art from a .ans file
bool loadArtFromFile(const std::string& path, Art& art);

// loads art into a .ans file
bool loadArtIntoFile(const Art& art, const std::string& path);


// loads palette from file
// swap: change fore -> back and vise versa
bool loadPaletteFromFile(const std::string& path, std::string palette[PALETTE_SIZE], bool swap = false);

// loads palette into file
// swap: change fore -> back and vise versa
bool loadPaletteIntoFile(const std::string palette[PALETTE_SIZE], const std::string& path, bool swap = false);