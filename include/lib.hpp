#pragma once

#include <atomic>
#include <csignal>
#include <string>
#include <vector>
#include <cassert>

#include "lib.hpp"
#include "output.hpp"
#include "settings.hpp"


inline std::atomic<bool> RUNNING(true);

void signalHandler(int signal);

// inclusive
template <typename T>
inline void clamp(T& num, const T& lower, const T& upper) {
	     if (num < lower) num = lower;
	else if (num > upper) num = upper;
}

std::string concat(const std::string& str1, const char* str2);

class Art {
public: 
	std::vector<Cell> map;
	int width, height;
	Cell defCell;
	int x, y;

	Art() = default;
	Art(int w, int h, const Cell& def) : width(w), height(h), defCell(def) {
		map.resize(w*h, def);
	}

	void set(int x, int y, const Cell& cell);

	// col = 0: edit foreground color,  col = 1: edit background color,  col = 2: edit character
	void edit(int _x, int _y, const std::string& str, char col);

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

	void resize(int wLeft, int wRight, int hUp, int hDown);
};