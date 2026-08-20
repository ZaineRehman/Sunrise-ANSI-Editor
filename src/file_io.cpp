/* 
 * File I/O
**/

#include "file_io.hpp"

#include <fstream>
#include <string>
#include <vector>

#include "settings.hpp"
#include "output.hpp"
#include "lib.hpp"
#include "log.hpp"
#include "encodings.hpp"


bool loadArtFromFile(const std::string& path, Art& art) {
	std::vector<Cell> newMap {};
	std::vector<std::vector<Cell>> tempMap {};
	
	std::ifstream file(path);
	if (!file.is_open()) {
		reportLog("!!! Failure to import art file: could not open \"" + path + "\"");
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
	// 0 = UTF-8, 1 = CP437
	int encodingFound = 1;
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
			} else if ((line[i] == 'm' || line[i] == 'h' || line[i] == 'l') && codeStart != -1) {  // TODO this only works for some codes
				// end of ANSI (color) code
				std::string code = line.substr(codeStart, i-codeStart+1);

				// check for multiple codes
				for (const std::string& codePart : ANSI::splitCode(code)) {
					// find type of code
					int type = ANSI::findCodeType(codePart);

					if (0 <= type && type <= 7) {
						// color
						// overrides prior codes
						if (type % 2) build.color_back = code;
						else          build.color_fore = code;
					} else {
						// other ANSI code
						build.extra_codes += code;
						if (code == ANSI::reset) {
							// reset colors
							build.color_fore = "";
							build.color_back = "";
						}
					}
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
						i += 1; // advance outer loop counter
						encodingFound = 0;
					}
				} else if ((lead & 0xF0) == 0xE0) {
					// 3-byte UTF-8 (1110xxxx) - symbols, box-drawing, CJK
					if (i + 2 < line.size()) {
						utf8_char = line.substr(i, 3);
						i += 2;
						encodingFound = 0;
					}
				} else if ((lead & 0xF8) == 0xF0) {
					// 4-byte UTF-8 (11110xxx) - emojis
					if (i + 3 < line.size()) {
						utf8_char = line.substr(i, 4);
						i += 3;
						encodingFound = 0;
						// TODO why the fuck are you using emojis
					}
				}

				// fallback
				if (utf8_char.empty()) {
					utf8_char += line[i];
				}

				build.ch = utf8_char;
				tempMap[tempMap.size() - 1].push_back(build);
				build = Cell{};
			}
		}
	}

	if (DEBUG_REPORT_LEVEL >= 3) reportLog("\tencoding found: " + std::string(encodingFound ? "UTF-8" : "CP437"));

	// turn temp map into a real map
	// pinnochio

	// find new width
	int largestFoundWidth = 0;
	for (size_t y = 0; y < tempMap.size(); ++y) {
		if (DEBUG_REPORT_LEVEL >= 3) {
			std::string linee = "";
			for (const Cell& i : tempMap[y]) {
				linee += "{"+i.color_fore+", ";
				linee +=     i.color_back+", ";
				linee +=     i.extra_codes+"}";
				linee += "("+(encodingFound == 1 ? convert_cp437_utf8(i.ch) : i.ch) + ") ";
			}
			reportLog("\tline " + std::to_string(y) + ": " + linee);
		}

		largestFoundWidth = max(static_cast<size_t>(largestFoundWidth), tempMap[y].size());
	}

	if (DEBUG_REPORT_LEVEL >= 3) reportLog("\tnew width: " + std::to_string(largestFoundWidth));

	// put it all in
	Cell priorColor {};
	for (size_t i = 0; i < tempMap.size(); ++i) {
		// fill in any blank spaces
		while(tempMap[i].size() < largestFoundWidth) {
			tempMap[i].push_back(Cell{" "});
		}

		//reportLog("New size: " + std::to_string(tempMap[i].size()));

		std::string bleed = "Bleed: ";

		for (size_t n = 0; n < tempMap[i].size(); ++n) {
			Cell toAdd = tempMap[i][n];

			// make sure the encoding is proper (everything internally should be UTF-8)
			// TODO redundantly runs this when no extended characters are found
			if (encodingFound == 1) toAdd.ch = convert_cp437_utf8(toAdd.ch);

			// ! bleed colors !
			// check if the new color slot is empty, 
			// if it is (AND if there is no reset code) then fill it
			if (toAdd.extra_codes.find(ANSI::reset) == std::string::npos) <%
				// reset not found!
				if (!toAdd.color_fore.size()) toAdd.color_fore = priorColor.color_fore; else priorColor.color_fore = toAdd.color_fore;
				if (!toAdd.color_back.size()) toAdd.color_back = priorColor.color_back; else priorColor.color_back = toAdd.color_back;
			%> else <%
				// reset found, so reset
				priorColor = Cell{};
			%>
			// lil digraphs

			newMap.push_back(toAdd);
		}

		//reportLog(bleed);
	}

	//if (DEBUG_REPORT_LEVEL >= 3) {
	//	std::string linee = "";
	//	for (size_t i = 0; i < newMap.size(); ++i) {
	//		if (i % largestFoundWidth == 0) {
	//			linee += "\nFIN: ";
	//		}
	//		linee += "{"+newMap[i].color_fore+", ";
	//		linee +=     newMap[i].color_back+", ";
	//		linee +=     newMap[i].extra_codes+"}";
	//		linee += "("+newMap[i].ch + ") ";
	//	}
	//	reportLog(linee);
	//}

	art.map = newMap;
	art.width = largestFoundWidth;
	art.height = newY;

	if (DEBUG_REPORT_LEVEL >= 3) reportLog("\tnew size: " + std::to_string(art.width) + "x" + std::to_string(art.height));
	if (DEBUG_REPORT_LEVEL >= 2) reportLog("Loaded art from file: \"" + path + "\"");

	return true;
}

bool loadArtIntoFile(const Art& art, const std::string& path) {
	std::string built = "";
	
	std::ofstream file(path);
	if (!file.is_open()) {
		reportLog("!!! Failure to export file: could not open \"" + path + "\"");
		return false;
	}

	for (size_t i = 0; i < static_cast<size_t>(art.height); ++i) {
		for (size_t x = 0; x < static_cast<size_t>(art.width); ++x) {
			Cell thisCell = art.map[i*art.width + x];

			// make sure encoding is right
			if (ART_ENCODING == 1) {
				thisCell.ch = convert_utf8_cp437(thisCell.ch);
			}

			built += thisCell.color_fore + thisCell.color_back + thisCell.ch;

			if (thisCell.color_fore.size() || thisCell.color_back.size()) built += ANSI::reset;
		}
		if (i != static_cast<size_t>(art.height-1)) built += "\n";
	}

	file << built;
	file.close();

	if (DEBUG_REPORT_LEVEL >= 2) reportLog("Loaded art into file: \"" + path + "\"");

	return true;
}



bool loadPaletteFromFile(const std::string& path, std::string palette[PALETTE_SIZE], bool swap) {
	// .plt file
	// each line is an ANSI code for a color

	// TODO safeguard against: non .plt files, improper file syntax

	std::ifstream file(path);
	if (!file.is_open()) {
		reportLog("!!! Failure to import palette file: could not open \"" + path + "\"");
		return false;
	}

	//std::string built[PALETTE_SIZE];

	std::string line;
	int i = 0;
	while (std::getline(file, line)) {
		if (i == PALETTE_SIZE) break;
		if (DEBUG_REPORT_LEVEL >= 3) reportLog("\timporting color: " + line);
		palette[i] = (swap ? ANSI::invertColor(line) : line);  // grab all except \n?
		if (DEBUG_REPORT_LEVEL >= 3) reportLog("\timporED: " + palette[i]);
		i++;
	}


	if (DEBUG_REPORT_LEVEL >= 2) reportLog("Loaded palette from file: \"" + path + "\"");
	return true;
}

bool loadPaletteIntoFile(const std::string palette[PALETTE_SIZE], const std::string& path, bool swap) {
	std::ofstream file(path);
	if (!file.is_open()) {
		reportLog("!!! Failure to export palette: could not open \"" + path + "\"");
		return false;
	}

	for (size_t i = 0; i < PALETTE_SIZE; ++i) {
		if (DEBUG_REPORT_LEVEL >= 3) reportLog("\texporting color: " + palette[i]);
		file << (swap ? ANSI::invertColor(palette[i]) : palette[i]) << '\n';
	}

	if (DEBUG_REPORT_LEVEL >= 2) reportLog("Loaded palette into file: \"" + path + "\"");
	return true;
}