/* 
 * Pre-calculated things used in main.cpp
**/

#include <cstdint>

#include "art.hpp"
#include "output.hpp"
#include "lib.hpp"


CellString getSunriseAnsi() {
	CellString sunriseAnsi{concat("Sunrise ANSI Editor   ", VERSION)};

	constexpr float r_start = 255.0f, g_start = 100.0f, b_start = 10.0f;
	constexpr float r_end   = 135.0f, g_end   = 205.0f, b_end   = 255.0f;

	float r = r_start, g = g_start, b = b_start;
	for (size_t i = 0; i < sunriseAnsi.size(); ++i) {
		sunriseAnsi.internal[i].color_fore = ANSI::bold + ANSI::Color_24bit::makeColor(
			static_cast<uint8_t>(r), 
			static_cast<uint8_t>(g), 
			static_cast<uint8_t>(b), 
			false
		);
		r -= (r_start - r_end) / static_cast<float>(sunriseAnsi.size());
		g -= (g_start - g_end) / static_cast<float>(sunriseAnsi.size());
		b -= (b_start - b_end) / static_cast<float>(sunriseAnsi.size());
	}

	return sunriseAnsi;
}

CellString getColorCatalogue_4bit() {
	std::vector<Cell> color4bitInits {
		Cell{" ", "", ANSI::black_back}, 
		Cell{" ", "", ANSI::red_back}, 
		Cell{" ", "", ANSI::green_back}, 
		Cell{" ", "", ANSI::yellow_back}, 
		Cell{" ", "", ANSI::blue_back}, 
		Cell{" ", "", ANSI::magenta_back}, 
		Cell{" ", "", ANSI::cyan_back}, 
		Cell{" ", "", ANSI::white_back}, 
		Cell{" ", "", ANSI::black_back_bright}, 
		Cell{" ", "", ANSI::red_back_bright}, 
		Cell{" ", "", ANSI::green_back_bright}, 
		Cell{" ", "", ANSI::yellow_back_bright}, 
		Cell{" ", "", ANSI::blue_back_bright}, 
		Cell{" ", "", ANSI::magenta_back_bright}, 
		Cell{" ", "", ANSI::cyan_back_bright}, 
		Cell{" ", "", ANSI::white_back_bright}, 
	};

	return CellString{color4bitInits};
}

CellString getColorCatalogue_8bit() {
	CellString colorCatalogue_8bit;
	for (int q = 16; q <= 231; q+=2) {
		colorCatalogue_8bit += Cell{" ", "", ANSI::Color_8bit::makeColor(q, true)};
	}

	return colorCatalogue_8bit;
}

CellString getColorCatalogue_24bit() {
	CellString colorCatalogue_24bit;

	for (float q = 0.0f; q < COLOR_CATALOGUE_24B_Y-1; ++q) {
		float specialNumber = (q+1)*(255.0f/(COLOR_CATALOGUE_24B_Y-1));
		//std::cout << specialNumber << '\n';

		if (q == 0.0f) specialNumber += 10.0f;

		float r = specialNumber;
		float g = 0.0f, b = 0.0f;

		for (float i = 0.0f; i < COLOR_CATALOGUE_24B_X; ++i) {
			colorCatalogue_24bit += Cell{" ", "", ANSI::Color_24bit::makeColor(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), true)};

			if (specialNumber == 0.0f) continue;

			if (i < (COLOR_CATALOGUE_24B_X/4.0f)) {
				g += (specialNumber)/(COLOR_CATALOGUE_24B_X/4.0f);
			}
			else if (i < 2.0f*(COLOR_CATALOGUE_24B_X/4.0f)) {
				r -= (specialNumber)/(COLOR_CATALOGUE_24B_X/4.0f);
				b += (specialNumber)/(COLOR_CATALOGUE_24B_X/4.0f);
			}
			else if (i < 3.0f*(COLOR_CATALOGUE_24B_X/4.0f)) {
				g -= (specialNumber)/(COLOR_CATALOGUE_24B_X/4.0f);
			}
			else {
				r += (specialNumber)/(COLOR_CATALOGUE_24B_X/4.0f);
			}
		}
	}

	for (int f = 0; f < COLOR_CATALOGUE_24B_X; ++f) {
		colorCatalogue_24bit += Cell{" ", "", ANSI::Color_24bit::makeColor(f*(255/COLOR_CATALOGUE_24B_X), f*(255/COLOR_CATALOGUE_24B_X), f*(255/COLOR_CATALOGUE_24B_X), true)};
	}

	return colorCatalogue_24bit;
}

CellString getCharCatalogue() {
	CellString charCatalogue {};

	for (int i = 0; i < 256; ++i) {
		charCatalogue += " ";
		charCatalogue += Cell{charLookupTable[i], "", ""};
	}

	return charCatalogue;
}