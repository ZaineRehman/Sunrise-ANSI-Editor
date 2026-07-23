/* 
 * File I/O
**/

#include <fstream>
#include <string>
#include <vector>

#include "file_io.hpp"
#include "output.hpp"
#include "lib.hpp"


bool loadArtFromFile(const std::string& path, Art& art) {
	std::vector<Cell> newMap {};
	std::vector<std::vector<Cell>> tempMap {};

	std::ifstream file(path);
	if (!file.is_open()) return false;

	/*
	 *  v-----------CELL-----------v
	 * [color_fore][color_back][char][reset]
	 * 
	 * all optional except char, 
	 * ignore all reset codes
	**/

	std::string line;
	while (std::getline(file, line)) {
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
					// we do not care
				} else if (ANSI::findColorType(code) % 2) {  // findColorType always returns odd numbers on backgrounds
					build.color_back = code;
				} else {
					build.color_fore = code;
				}

				codeStart = -1;
			} else if (codeStart == -1) {
				// normal character
				// doin normal stuff
				build.ch = line[i];
				tempMap[tempMap.size()-1].push_back(build);
				build = Cell{};
			}
		}

		// turn temp map into a real map
		// pinnochio

		// find new width
		int largestFoundWidth = 0;
		for (size_t y = 0; y < tempMap.size(); ++y) {
			largestFoundWidth = max(static_cast<size_t>(largestFoundWidth), tempMap[y].size());
		}

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
		art.height = newMap.size();
	}

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
		if (i != art.height-1) built += "\n";
	}

	file << built;
	file.close();

	return true;
}