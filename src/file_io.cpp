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

	// find file encoding

	std::string smallLine;
	std::string wholeFile;
	while (std::getline(file, smallLine)) wholeFile += smallLine;
	// reset file for parsing again
	file.clear();
	file.seekg(0, std::ios::beg);

	// 0 = UTF-8, 1 = CP437
	int encodingFound = is_utf8(wholeFile) ? 0 : 1;


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
			} else if ((line[i] == 'm' || line[i] == 'h' || line[i] == 'l') && codeStart != -1) {  // TODO this only works for some codes
				// end of ANSI (color) code
				std::string code = line.substr(codeStart, i-codeStart+1);

				// check for multiple codes
				for (const std::string& codePart : ANSI::breakupColorCode(code)) {
					//if (DEBUG_REPORT_LEVEL >= 4) reportLog("Split code: " + codePart);

					/*
					* [7h[0;1;40;30m[?33h-
					* 
					* Should be split like this: 
					* 		Fore: \033[30m
					* 		Back: \033[40m
					* 		Extra: \033[7h + \033[0m + \033[1m + \033[?33h
					**/

					// find type of code
					int type = ANSI::findCodeType(codePart);

					if (0 <= type && type <= 7) {
						//if (DEBUG_REPORT_LEVEL >= 4) reportLog("\tfound COLOR (" + std::string(type % 2 ? "back" : "fore") + ")");
						// color
						// overrides prior codes
						if (type % 2) build.color_back = codePart;
						else          build.color_fore = codePart;
					} else {
						//if (DEBUG_REPORT_LEVEL >= 4) reportLog("\tfound OTHER " + std::string(codePart == ANSI::reset ? "(reset)" : ""));
						// other ANSI code
						build.extra_codes += codePart;
						if (codePart == ANSI::reset) {
							// reset colors
							build.color_fore = "";
							build.color_back = "";
						}
					}
				}

				codeStart = -1;
			} else if (codeStart == -1) {
				// normal character
				std::string utf8_char = "";
				unsigned char lead = static_cast<unsigned char>(line[i]);

				if (encodingFound == 0) {
					if (lead < 0x80) {
						// 1-byte ASCII (0xxxxxxx)
						utf8_char += line[i];
					} else if (i + 1 < line.size() && (lead & 0xE0) == 0xC0) {
						// 2-byte UTF-8 (110xxxxx)
						utf8_char = line.substr(i, 2);
						i += 1;
					} else if (i + 2 < line.size() && (lead & 0xF0) == 0xE0) {
						// 3-byte UTF-8 (1110xxxx) - symbols, box-drawing, CJK
						utf8_char = line.substr(i, 3);
						i += 2;
					} else if (i + 3 < line.size() && (lead & 0xF8) == 0xF0) {
						// 4-byte UTF-8 (11110xxx) - emojis
						utf8_char = line.substr(i, 4);
						i += 3;
						// TODO why the fuck are you using emojis
					}
				} else {
					utf8_char += line[i];
				}

				// fallback safety check
				if (utf8_char.empty()) {
					utf8_char += line[i];
				}

				build.ch = utf8_char;
				tempMap[tempMap.size() - 1].push_back(build);
				build = Cell{};
			}
		}
	}

	if (DEBUG_REPORT_LEVEL >= 3) reportLog("\tencoding found: " + std::string(encodingFound ? "CP437" : "UTF-8"));

	// turn temp map into a real map
	// pinnochio

	// sets new x and y of cursor
	auto moveCellCursor = [&tempMap](size_t& x, size_t& y, int up, int down, int left, int right) {
		// left
		for (int i = 0; i < left && x > 0; ++i, --x);
		
		// up
		for (int i = 0; i < up && y > 0; ++i, --y);

		// right                                                                                  TODO check this  v
		if (x + right >= tempMap[y].size()) tempMap[y].insert(tempMap[y].end(), (size_t)(x+right-tempMap[y].size()+1), Cell{" "});
		x += right;
		
		// down
		if (y + down >= tempMap.size()) tempMap.insert(tempMap.end(), (size_t)(y+down-tempMap.size()+1), std::vector<Cell>(x, Cell{" "}));
		x += right;
	};

	// handle cursor codes
	for (size_t y = 0; y < tempMap.size(); ++y) {
		for (size_t x = 0; x < tempMap[y].size(); ++x) {
			std::string allOfem = tempMap[y][x].extra_codes;
			for (const std::string& c : ANSI::splitCodes(allOfem)) {
				if (ANSI::findCodeType(c) == -4) {
					std::pair<int,int> info = ANSI::getCursorCodeInfo(c);

					if (DEBUG_REPORT_LEVEL >= 4) reportLog("\tcursor code found: " + c + "  ->  " + std::to_string(info.first) + "," + std::to_string(info.second));

					switch (info.first) {
						case 0: moveCellCursor(x, y, info.second, 0, 0, 0); break;  // up
						case 1: moveCellCursor(x, y, 0, info.second, 0, 0); break;  // down
						case 2: moveCellCursor(x, y, 0, 0, info.second, 0); break;  // left
						case 3: moveCellCursor(x, y, 0, 0, 0, info.second); break;  // right
						case 4: moveCellCursor(x, y, 0, 1, 0, 0); break;  // next line
						case 5: moveCellCursor(x, y, 1, 0, 0, 0); break;  // prev line
						case 6: {  // set column
							int amount = info.second - x;
							moveCellCursor(x, y, 0, 0, (amount<0?-amount:0), (amount>0?amount:0));
							break;
						}
						case 7: break;  // TODO
					}
				}
			}
		}
	}

	// find new width
	size_t largestFoundWidth = 0;
	for (size_t y = 0; y < tempMap.size(); ++y) {
		if (DEBUG_REPORT_LEVEL >= 3) {
			std::string linee = "";
			for (const Cell& i : tempMap[y]) {
				linee += "{"+i.color_fore+", ";
				linee +=     i.color_back+", ";
				linee +=     i.extra_codes+"}";
				linee += "("+(encodingFound == 1 ? convert_cp437_utf8(i.ch) : i.ch) + ") ";
			}
			if (DEBUG_REPORT_LEVEL >= 4) reportLog("\tline " + std::to_string(y) + ": " + linee);
		}

		largestFoundWidth = max(largestFoundWidth, tempMap[y].size());
	}

	if (DEBUG_REPORT_LEVEL >= 3) reportLog("\tnew width: " + std::to_string(largestFoundWidth));

	// put it all in
	Cell priorCell {};
	for (size_t i = 0; i < tempMap.size(); ++i) {
		// fill in any blank spaces
		// TODO do this later? blank spaces get bled into
		while(tempMap[i].size() < largestFoundWidth) {
			tempMap[i].push_back(Cell{" "});
		}

		//reportLog("New size: " + std::to_string(tempMap[i].size()));

		//std::string bleed = "Bleed: ";

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
				if (!toAdd.color_fore.size()) toAdd.color_fore = priorCell.color_fore; else priorCell.color_fore = toAdd.color_fore;
				if (!toAdd.color_back.size()) toAdd.color_back = priorCell.color_back; else priorCell.color_back = toAdd.color_back;

				// also bleed extras
				// dont add duplicates
				for (const std::string& thiscode : ANSI::splitCodes(priorCell.extra_codes)) {
					if (ANSI::findCodeType(thiscode) == -2 && toAdd.extra_codes.find(thiscode) == std::string::npos) {
						// ok its not in there (and applicable code) lets add
						toAdd.extra_codes += thiscode;
					}
				}
			%> else <%
				// reset found, so assure colors do not bleed
				priorCell.color_fore = toAdd.color_fore;
				priorCell.color_back = toAdd.color_back;
				
				// reset extra codes if applicable
				std::string newExtras = "";
				for (const std::string& thiscode : ANSI::splitCodes(toAdd.extra_codes)) {
					if (ANSI::findCodeType(thiscode) != -2) {
						// applicable
						reportLog("\tKeeping extra code after reset: " + thiscode);
						newExtras += thiscode;
					}
				}
				
				priorCell.extra_codes = newExtras;
			%> // lil digraphs

			// add extra codes if applicable
			for (const std::string& thiscode : ANSI::splitCodes(toAdd.extra_codes)) {
				reportLog("\tChecking for bleed: " + thiscode + " (is " + std::to_string(ANSI::findCodeType(thiscode)) + ")");
				if (ANSI::findCodeType(thiscode) == -2) {
					// applicable
					reportLog("\tBleeding extra code: " + thiscode);
					priorCell.extra_codes += thiscode;
				}
			}

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

			built += thisCell.extra_codes + thisCell.color_fore + thisCell.color_back + thisCell.ch;

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