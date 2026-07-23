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
