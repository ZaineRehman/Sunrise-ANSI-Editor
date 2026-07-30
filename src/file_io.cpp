/* 
 * File I/O
**/
#include "file_io.hpp"

#include <fstream>
#include <string>
#include <vector>

#include "output.hpp"
#include "lib.hpp"
#include "log.hpp"


bool loadArtFromFile(const std::string& path, Art& art) {
	std::vector<Cell> newMap {};
	std::vector<std::vector<Cell>> tempMap {};
	
	std::ifstream file(path);
	if (!file.is_open()) {
		if (DEBUG_REPORT_LEVEL >= 1) reportLog("!!! Failure to import file: could not open \"" + path + "\"");
		return false;
	}
	
	if (DEBUG_REPORT_LEVEL >= 2) reportLog("Importing file...");

	/*
	 *  v-----------CELL-----------v
	 * [color_fore][color_back][char][reset]
	 * 
	 * all optional except char, 
	 * ignore all reset codes
	**/

	std::string line;
	int newY = 0;
	while (std::getline(file, line)) {
		newY++;
		// ANSI code index
		int codeStart = -1;

		Cell build {};
		tempMap.push_back({});

		for (size_t i = 0; i < line.size(); ++i) {
			if (line[i] == '\n') continue;  // break? is this needed? whatever
			if (line[i] == '\033') {
				// start of ANSI code
				codeStart = i;
			} else if (line[i] == 'm' && codeStart != -1) {  // TODO this only works for color codes
				// end of ANSI (color) code
				std::string code = line.substr(codeStart, i-codeStart+1);

				if (code == ANSI::reset) {
					build.color_fore = "";
					build.color_back = "";
				} else if (ANSI::findColorType(code) % 2) {  // findColorType always returns odd numbers on backgrounds
					build.color_back = code;
				} else {
					build.color_fore = code;
				}

				codeStart = -1;
			} else if (codeStart == -1) {
				// normal character
				// but its prolly UTF-8 so hah

				std::string utf8_char = "";
				unsigned char lead = static_cast<unsigned char>(line[i]);

				if (lead < 0x80) {
					// 1-byte ASCII (0xxxxxxx)
					utf8_char += line[i];
				} else if ((lead & 0xE0) == 0xC0) {
					// 2-byte UTF-8 (110xxxxx)
					if (i + 1 < line.size()) {
						utf8_char = line.substr(i, 2);
						i += 1; // Advance the outer loop counter
					}
				} else if ((lead & 0xF0) == 0xE0) {
					// 3-byte UTF-8 (1110xxxx) - Common for symbols, box-drawing, and CJK
					if (i + 2 < line.size()) {
						utf8_char = line.substr(i, 3);
						i += 2;
					}
				} else if ((lead & 0xF8) == 0xF0) {
					// 4-byte UTF-8 (11110xxx) - Emojis
					if (i + 3 < line.size()) {
						utf8_char = line.substr(i, 4);
						i += 3;
					}
				}

				// If we failed to extract a valid sequence, fallback safely to the raw byte
				if (utf8_char.empty()) {
					utf8_char += line[i];
				}

				build.ch = utf8_char;
				tempMap[tempMap.size() - 1].push_back(build);
				build = Cell{};
			}
		}
	}

	// turn temp map into a real map
	// pinnochio

	// find new width
	int largestFoundWidth = 0;
	for (size_t y = 0; y < tempMap.size(); ++y) {
		if (DEBUG_REPORT_LEVEL >= 3) {
			std::string linee = "";
			for (const Cell& i : tempMap[y]) {
				linee += "{" + i.color_fore + i.color_back + i.ch + "}";
			}

			reportLog("Line " + std::to_string(y) + ": " + linee);
		}
		
		largestFoundWidth = max(static_cast<size_t>(largestFoundWidth), tempMap[y].size());
	}

	if (DEBUG_REPORT_LEVEL >= 3) reportLog("\tnew width: " + std::to_string(largestFoundWidth));

	// put it all in
	for (size_t i = 0; i < tempMap.size(); ++i) {
		// fill in any blank spaces
		LOOP(largestFoundWidth - tempMap[i].size()) {
			tempMap[i].push_back(Cell{" ", "", ""});
		}

		for (size_t n = 0; n < tempMap[i].size(); ++n) {
			newMap.push_back(tempMap[i][n]);
		}
	}

	art.map = newMap;
	art.width = largestFoundWidth;
	art.height = newY;

	if (DEBUG_REPORT_LEVEL >= 3) reportLog("\tnew size: " + std::to_string(art.width) + "x" + std::to_string(art.height));

	return true;
}

bool loadArtIntoFile(const Art& art, const std::string& path) {
	std::string built = "";
	
	std::ofstream file(path);
	if (!file.is_open()) return false;

	for (size_t i = 0; i < static_cast<size_t>(art.height); ++i) {
		for (size_t x = 0; x < static_cast<size_t>(art.width); ++x) {
			Cell thisCell = art.map[i*art.width + x];
			built += thisCell.color_fore + thisCell.color_back + thisCell.ch;
			if (thisCell.color_fore.size() || thisCell.color_back.size()) built += ANSI::reset;
		}
		if (i != static_cast<size_t>(art.height-1)) built += "\n";
	}

	file << built;
	file.close();

	return true;
}