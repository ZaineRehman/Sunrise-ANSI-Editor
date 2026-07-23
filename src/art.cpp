/* 
 * Art class
**/

#include <atomic>
#include <csignal>
#include <string>
#include <vector>
#include <cassert>

#include "art.hpp"
#include "output.hpp"
#include "settings.hpp"


void Art::set(int x, int y, const Cell& cell) {
	assert(x < width && y < height);

	map[y*width + x] = cell;
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

			if (!col)     map[_y*width + _x].color_fore = str;
	else if (col == 1) map[_y*width + _x].color_back = str;
	else               map[_y*width + _x].ch = str;
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
}