// things i dont want cluttering main
// (mostly Art class)

#include <atomic>
#include <csignal>
#include <string>
#include <vector>
#include <cassert>

#include "lib.hpp"
#include "output.hpp"
#include "settings.hpp"


void signalHandler(int signal) {
	switch (signal) {
		case SIGINT: 
		case SIGTERM: 
		case SIGABRT: 
			RUNNING = false;
			break;
	}
}

std::string concat(const std::string& str1, const char* str2) {
	return str1 + str2;
}


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

        //DEBUG_STR += std::to_string(left);
        //DEBUG_STR += ",";
        //DEBUG_STR += std::to_string(right);
        //DEBUG_STR += ",";
        //DEBUG_STR += std::to_string(up);
        //DEBUG_STR += ",";
        //DEBUG_STR += std::to_string(down);

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
        for (int e = 0; e < width; ++e) map.insert(map.begin(), defCell);
    }
    height += hUp; y -= hUp;

    for (int i = 0; i < hDown; ++i) {
        for (int e = 0; e < width; ++e) map.push_back(defCell);
    }
    height += hDown;

    for (int i = 0; i < wLeft; ++i) {
        for (int e = 0; e < height; ++e) {
            map.insert(map.begin()+e*width + e, defCell);
        }
        width++; x--;
    }

    for (int i = 0; i < wRight; ++i) {
        for (int e = 0; e < height; ++e) {
            map.insert(map.begin()+e*width + width + e, defCell);
        }
        width++;
    }
}