/* 
 * Art class
**/

#include <atomic>
#include <csignal>
#include <string>
#include <vector>
#include <cassert>
#include <algorithm>

#include "art.hpp"
#include "output.hpp"
#include "settings.hpp"
#include "lib.hpp"
#include "log.hpp"


void Art::set(int x, int y, const Cell& cell) {
	assert(x < width && y < height);

	map[y*width + x] = cell;

	changeFlag = true;
}

// col = 0: edit foreground color,  col = 1: edit background color,  col = 2: edit character
void Art::edit(int _x, int _y, const std::string& str, char col) {
	//assert(x < width && y < height);

	if (_x < 0 || _y < 0 || _x >= width || _y >= height) {
		// out of bounds, resize art

		int left = _x < 0 ? -_x : 0;
		int right = _x >= width ? _x-width+1 : 0;
		int up = _y < 0 ? -_y : 0;
		int down = _y >= height ? _y-height+1 : 0;

		resize(left, right, up, down);

		// change coordinates to match
		_x += left;
		_y += up;
	}

	if (!col)          map[_y*width + _x].color_fore = str;
	else if (col == 1) map[_y*width + _x].color_back = str;
	else               map[_y*width + _x].ch = str;

	changeFlag = true;
}

void Art::resize(int wLeft, int wRight, int hUp, int hDown) {
	for (int i = 0; i < hUp; ++i) {
		for (int e = 0; e < width; ++e) map.insert(map.begin(), defaultCell);
	}
	height += hUp; y -= hUp;

	for (int i = 0; i < hDown; ++i) {
		for (int e = 0; e < width; ++e) map.push_back(defaultCell);
	}
	height += hDown;

	for (int i = 0; i < wLeft; ++i) {
		for (int e = 0; e < height; ++e) {
			map.insert(map.begin()+e*width + e, defaultCell);
		}
		width++; x--;
	}

	for (int i = 0; i < wRight; ++i) {
		for (int e = 0; e < height; ++e) {
			map.insert(map.begin()+e*width + width + e, defaultCell);
		}
		width++;
	}

	changeFlag = true;
}

void Art::trim() {
	if (!map.size()) return;

	auto cellIsEmpty = [](const Cell& cell) {
		// no char AND no background: empty
		return cell.ch == " " && cell.color_back == "";
	};

	// check for empty rows at beginning
	int emptyTop = 0;
	for (size_t r = 0; r < static_cast<size_t>(height); ++r) {
		bool rowEmpty = true;
		for (size_t c = 0; c < static_cast<size_t>(width); ++c) {
			if (!cellIsEmpty(map[r*width + c])) {
				rowEmpty = false;
				break;
			}
		}

		if (!rowEmpty) break;
		emptyTop++;
	}
	if (DEBUG_REPORT_LEVEL >= 4) reportLog("\tempty TOP: " + std::to_string(emptyTop));

	// check for empty rows at end
	// probably there is some smart way to not have to repeat all this code i dunno
	int emptyBottom = 0;
	for (size_t r = static_cast<size_t>(height)-1; r != 0; --r) {
		bool rowEmpty = true;
		for (size_t c = static_cast<size_t>(width)-1; c != 0; --c) {
			if (!cellIsEmpty(map[r*width + c])) {
				rowEmpty = false;
				break;
			}
		}

		if (!rowEmpty) break;
		emptyBottom++;
	}
	if (DEBUG_REPORT_LEVEL >= 4) reportLog("\tempty BOTTOM: " + std::to_string(emptyBottom));

	// quick check if its completely empty
	//if (emptyTop + emptyBottom == height) {
	//	map.clear();
	//	width = 0; height = 0;
	//	changeFlag = true;
	//	return;
	//}

	// check for emtpy rows on left
	int emptyLeft = 0;
	for (size_t c = 0; c < static_cast<size_t>(width); ++c) {
		bool columnEmpty = true;
		for (size_t r = 0; r < static_cast<size_t>(height); ++r) {
			if (!cellIsEmpty(map[r*width + c])) {
				columnEmpty = false;
				break;
			}
		}

		if (!columnEmpty) break;
		emptyLeft++;
	}
	if (DEBUG_REPORT_LEVEL >= 4) reportLog("\tempty LEFT: " + std::to_string(emptyLeft));

	// check for emtpy rows on right
	int emptyRight = 0;
	for (size_t c = static_cast<size_t>(width)-1; c != 0; --c) {
		bool columnEmpty = true;
		for (size_t r = static_cast<size_t>(height)-1; r != 0; --r) {
			if (!cellIsEmpty(map[r*width + c])) {
				columnEmpty = false;
				break;
			}
		}

		if (!columnEmpty) break;
		emptyRight++;
	}
	if (DEBUG_REPORT_LEVEL >= 4) reportLog("\tempty RIGHT: " + std::to_string(emptyRight));

	if (emptyTop + emptyBottom + emptyLeft + emptyRight == 0) {
		if (DEBUG_REPORT_LEVEL >= 4) reportLog("Not trimming - same dimensions");
		return;
	}

	
	// build new vector
	std::vector<Cell> newvec {};

	for (size_t r = emptyTop; r < static_cast<size_t>(height) - emptyBottom; ++r) {
		for (size_t c = emptyLeft; c < static_cast<size_t>(width) - emptyRight; ++c) {
			newvec.push_back(map[r*width + c]);
		}
	}

	if (DEBUG_REPORT_LEVEL >= 2) reportLog(
		"Trimming art from " + 
		std::to_string(width)+"x"+std::to_string(height) + 
		" to " + 
		std::to_string(width-emptyLeft-emptyRight)+"x"+std::to_string(height-emptyTop-emptyBottom) + 
		"(x: " + std::to_string(x) + "->" + std::to_string(x + emptyLeft) + 
		", y: " + std::to_string(y) + "->" + std::to_string(y + emptyTop) + ")"
	);

	width -= emptyLeft + emptyRight;
	height -= emptyTop + emptyBottom;
	x += emptyLeft;
	y += emptyTop;
	map = newvec;

	// just in case
	if (emptyLeft + emptyRight >= width) width = 0;
	if (emptyTop + emptyBottom >= height) height = 0;

	changeFlag = true;
}