/* 
 * Output related things
**/

#pragma once

#include <iostream>
#include <cstdint>
#include <vector>

#include "ANSI.hpp"


inline const std::string charLookupTable[] = {
	" ", "☺", "☻", "♥", "♦", "♣", "♠", "•", "◘", "○", "◙", "♂", "♀", "♪", "♫", "☼", 
	"►", "◄", "↕", "‼", "¶", "§", "▬", "↨", "↑", "↓", "→", "←", "∟", "↔", "▲", "▼", 
	" ", "!", "\"", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/", 
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", ":", ";", "<", "=", ">", "?", 
	"@", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", 
	"P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "[", "\\", "]", "^", "_", 
	"`", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", 
	"p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "{", "|", "}", "~", "⌂", 
	"Ç", "ü", "é", "â", "ä", "à", "å", "ç", "ê", "ë", "è", "ï", "î", "ì", "Ä", "Å", 
	"É", "æ", "Æ", "ô", "ö", "ò", "û", "ù", "ÿ", "Ö", "Ü", "¢", "£", "¥", "₧", "ƒ", 
	"á", "í", "ó", "ú", "ñ", "Ñ", "ª", "º", "¿", "⌐", "¬", "½", "¼", "¡", "«", "»", 
	"░", "▒", "▓", "│", "┤", "╡", "╢", "╖", "╕", "╣", "║", "╗", "╝", "╜", "╛", "┐", 
	"└", "┴", "┬", "├", "─", "┼", "╞", "╟", "╚", "╔", "╩", "╦", "╠", "═", "╬", "╧", 
	"╨", "╤", "╥", "╙", "╘", "╒", "╓", "╫", "╪", "┘", "┌", "█", "▄", "▌", "▐", "▀", 
	"α", "β", "Γ", "π", "Σ", "σ", "µ", "τ", "Φ", "Θ", "Ω", "δ", "∞", "φ", "ε", "∩", 
	"≡", "±", "≥", "≤", "⌠", "⌡", "÷", "≈", "°", "·", "∙", "√", "ⁿ", "²", "■", " "
};


#include "settings.hpp"


// returns the width and height of the terminal (width, height)
std::pair<int,int> getTerminalDimensions();


// contains strings for a character along with 2 color strings (fore/back)
// do NOT put more than 1 character in 'ch' (you WILL be found)
struct Cell {
	std::string ch;
	std::string color_fore;
	std::string color_back;
};

struct CellString {
	std::vector<Cell> internal;

	CellString() = default;
	CellString(const CellString& other) : internal(other.internal) {}
	CellString(const std::string& str) {
		append(str);
	}
	CellString(const std::string& str, const std::string& color) {
		append(str, color);
	}
	CellString(const std::string& str, const std::string& colorF, const std::string& colorB) {
		append(str, colorF, colorB);
	}
	CellString(const std::vector<Cell>& vec) : internal(vec) {}
	CellString(const Cell& cell) {
		append(cell);
	}

	inline const Cell& operator[](std::size_t i) const noexcept {
		return internal[i];
	}
	inline size_t size() const noexcept {
		return internal.size();
	}

	inline CellString& operator+=(const std::string& str) {
		append(str);
		return *this;
	}
	inline CellString& operator+=(const Cell& str) {
		append(str);
		return *this;
	}
	inline CellString& operator+=(const CellString& cellstr) {
		for (const Cell& c : cellstr.internal) {
			append(c);
		}
		return *this;
	}

	inline CellString operator+(const CellString& other) const {
		std::vector<Cell> built = this->internal;
		built.reserve(built.size() + other.internal.size());
		built.insert(built.end(), other.internal.begin(), other.internal.end());
		return CellString{built};
	}
	inline CellString operator+(const std::string& str) const {
		return CellString(str) + *this;
	}
	inline CellString operator+(const char* str) const {
		std::string realstr = str;
		return CellString(realstr) + *this;
	}

	inline CellString& operator=(const CellString& other) {
		internal = other.internal;
		return *this;
	}

	inline void append(const std::string& str, const std::string& colorF = "", const std::string& colorB = "") {
		for (size_t i = 0; i < str.size(); ++i) {
			internal.push_back(Cell{std::string(1, str[i]), colorF, colorB});
		}
	}
	inline void append(const Cell& cell) {
		internal.push_back(cell);
	}
};

// returns the highest level color code found
// 0 = none,  1 = 4-bit,  2 = 8-bit,  3 = 24-bit
int findHighestColorCode(const CellString& cells);

// clamps a cell to a certain color code or lower, 
// replacing the color with nothing if it must be removed
// TODO  ^ this is temp ^
Cell clampColor(const Cell& cell, int maxColor);

// stores a screen buffer of Cells and renders them
class Renderer {
private:
	uint32_t width, height;

public: 
	std::vector<Cell> buffer {};

	Renderer(uint32_t width, uint32_t height) : width(width), height(height), buffer(width*height, Cell{DEFAULT_BACK, "", ""}) {}

	// puts a cell onto the buffer
	void put(uint32_t x, uint32_t y, const Cell& cell);
	// edits a cell in the buffer
	// col = 0 (default): edit foreground color,  col = 1: edit background color,  col = 2: edit character
	void edit(uint32_t x, uint32_t y, const std::string& str, char col = 0);
	// puts a string of cells onto the buffer
	void put(uint32_t x, uint32_t y, const CellString& str);

	// get
	Cell get(uint32_t x, uint32_t y) const;

	// renders all cells
	void render() const;

	// sets all cells to value of 'replacement'
	void fill(const Cell& replacement);

	// sets all cells to spaces
	inline void clear() {
		fill(Cell{" ","", ""});
	}

	// resizes the cell bounds
	void resize(int width, int height);
};


void clear();