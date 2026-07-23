#pragma once

#include <cstdint>

#include "lib.hpp"
#include "output.hpp"


// sunrise logo text with gradient
CellString getSunriseAnsi();

// 4-bit color catalogue
CellString getColorCatalogue_4bit();


// 8-bit color catalogue
CellString getColorCatalogue_8bit();

// 24-bit color catalogue
CellString getColorCatalogue_24bit();

// character catalogue
CellString getCharCatalogue();