/* 
 * Art class
**/

#pragma once


#include <atomic>
#include <csignal>
#include <string>
#include <vector>
#include <cassert>

#include "output.hpp"
#include "settings.hpp"


class Art {
public: 
	std::vector<Cell> map;
	int width, height;
	Cell defaultCell;
	int x, y;

	Art() = default;
	Art(int w, int h, const Cell& def) : width(w), height(h), defaultCell(def) {
		map.resize(w*h, def);
	}

	void set(int x, int y, const Cell& cell);

	// col = 0: edit foreground color,  col = 1: edit background color,  col = 2: edit character
	void edit(int _x, int _y, const std::string& str, char col);
	
	void resize(int wLeft, int wRight, int hUp, int hDown);
	

	inline constexpr bool inBounds(int _x, int _y) {
		return (
			_x >= x && _x < x+width &&
			_y >= y && _y < y+height
		);
	}

	// global space -> art space
	inline constexpr std::pair<int,int> toArtSpace(int _x, int _y) {
		//assert(inBounds(x, y));
		return std::make_pair<int,int>(_x - x, _y - y);
	}

	inline void reset(int newX, int newY) {
		map.clear();
		width = 0;
		height = 0;
		x = newX;
		y = newY;
	}
};