#pragma once

#include <iostream>
#include <cstdint>
#include <vector>


// ANSI escape codes
namespace ANSI {
	extern const std::string reset;
	extern const std::string bold;
	extern const std::string dim;
	extern const std::string italic;
	extern const std::string underline;
	extern const std::string blink_slow;
	extern const std::string blink_fast;
	extern const std::string invert;
	extern const std::string hide;
	extern const std::string strike;
	extern const std::string primary;
	extern const std::string alternate1;
	extern const std::string alternate2;
	extern const std::string alternate3;
	extern const std::string alternate4;
	extern const std::string alternate5;
	extern const std::string alternate6;
	extern const std::string alternate7;
	extern const std::string alternate8;
	extern const std::string alternate9;
	extern const std::string fraktur;
	extern const std::string doubleUnder;
	extern const std::string noBold;
	extern const std::string noItalic;
	extern const std::string noUnder;
	extern const std::string noBlink;
	extern const std::string propSpacing;
	extern const std::string noReverse;
	extern const std::string reveal;
	extern const std::string noStrike;
	extern const std::string noPropSpacing;
	extern const std::string framed;
	extern const std::string encircled;
	extern const std::string overlined;

	extern const std::string ideogramUnderline;
	extern const std::string ideogramDoubleUnderline;
	extern const std::string ideogramOverline;
	extern const std::string ideogramDoubleOverline;
	extern const std::string ideogramStress;
	extern const std::string ideogramReset;

	extern const std::string superscript;
	extern const std::string subscript;
	extern const std::string normalscript;

	// tests general ANSI codes (most of 0-75, no colors)
	//void generalCodesTest();
	void generalCodesTest();

	// standard 4-bit color codes

	extern const std::string black;
	extern const std::string red;
	extern const std::string green;
	extern const std::string yellow;
	extern const std::string blue;
	extern const std::string magenta;
	extern const std::string cyan;
	extern const std::string white;
	
	extern const std::string black_back;
	extern const std::string red_back;
	extern const std::string green_back;
	extern const std::string yellow_back;
	extern const std::string blue_back;
	extern const std::string magenta_back;
	extern const std::string cyan_back;
	extern const std::string white_back;

	extern const std::string black_bright;
	extern const std::string red_bright;
	extern const std::string green_bright;
	extern const std::string yellow_bright;
	extern const std::string blue_bright;
	extern const std::string magenta_bright;
	extern const std::string cyan_bright;
	extern const std::string white_bright;

	extern const std::string black_back_bright;
	extern const std::string red_back_bright;
	extern const std::string green_back_bright;
	extern const std::string yellow_back_bright;
	extern const std::string blue_back_bright;
	extern const std::string magenta_back_bright;
	extern const std::string cyan_back_bright;
	extern const std::string white_back_bright;

	extern const std::string cursor_home;
	extern const std::string cursor_visible;
	extern const std::string cursor_invisible;
	
	extern const std::string clearToEnd;
	extern const std::string clearFromStart;
	extern const std::string clear;
	extern const std::string clearScrollback;
	extern const std::string clearLine;
	extern const std::string clearLineToEnd;
	extern const std::string clearLineFromStart;
	
	extern const std::string saveScreen;
	extern const std::string restoreScreen;
	
	extern const std::string enableAltBuffer;
	extern const std::string disableAltBuffer;
	
	extern const std::string enableLineWrapping;
	extern const std::string disableLineWrapping;

	extern const std::string enableMouseTracking;
	extern const std::string disableMouseTracking;
	
	extern const std::string resetUnderlineColor;

	// tests the standard ANSI 4-bit color codes
	void colorTest();

	// returns the type of color code given
	// 4-bit: 0=foreground, 1=background, 2=foreground (bright), 3=background (bright)
	// 8-bit: 4=foreground, 5=background
	// 24-bit: 6=foreground, 7=background
	int findColorType(const std::string& code);

	// inverts a color-code string from foreground to background and vice versa
	std::string invertColor(const std::string& code);

	// selection from 8-bit color codes
	namespace Color_8bit {
		// creates a color code using the 8-bit ANSI code formula: 16 + 36 × r + 6 × g + b (0 ≤ r, g, b ≤ 5)
		// values should be between 0-5 (inclusive)
		std::string makeColor(int r, int g, int b, bool background = false);

		// creates the color code from the actual code value
		// https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit:~:text=hide-,256%2Dcolor%20mode,-Foreground%3A%20ESC%5B38%3B5
		std::string makeColor(int n, bool background = false);

		// creates a gray color using the 8-bit ANSI grayscale color table
		// level value is between 1-24 (inclusive)
		// higher = lighter
		std::string makeGray(int level, bool background = false);

		// sets underline color using 8-bit color
		std::string underlineColor(int r, int g, int b);

		void underlineColorTest();

		void colorTest();
	}
	// 24-bit color codes
	namespace Color_24bit {
		// creates a color code with 24-bit color
		std::string makeColor(uint8_t r, uint8_t g, uint8_t b, bool background = false);

		// sets underline color using 24-bit color
		std::string underlineColor(int r, int g, int b);

		void underlineColorTest();

		void colorTest();
	}
}

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

int findHighestColorCode(const CellString& cells);

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
	void putString(uint32_t x, uint32_t y, const CellString& str);

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