#pragma once

#include <iostream>
#include <cstdint>
#include <vector>


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
		for (size_t i = 0; i < str.size(); ++i) {
			internal.push_back(Cell{std::string(1, str[i]), "", ""});
		}
	}
	CellString(const std::string& str, const std::string& color) {
		for (size_t i = 0; i < str.size(); ++i) {
			internal.push_back(Cell{std::string(1, str[i]), color, ""});
		}
	}

	inline const Cell& operator[](std::size_t i) const noexcept {
		return internal[i];
	}
	inline size_t size() const noexcept {
		return internal.size();
	}
};

// stores a screen buffer of Cells and renders them
class Renderer {
private:
	uint32_t width, height;

public: 
	std::vector<Cell> buffer {};

	Renderer(uint32_t width, uint32_t height) : width(width), height(height), buffer(width*height, Cell{".", "", ""}) {}

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
	void clear(const Cell& replacement = Cell{" ","", ""});

	// resizes the cell bounds
	void resize(int width, int height);
};


void clear();


// ANSI escape codes
namespace ANSI {
	const inline std::string reset         = "\033[0m";
	const inline std::string bold          = "\033[1m";
	const inline std::string dim           = "\033[2m";
	const inline std::string italic        = "\033[3m";
	const inline std::string underline     = "\033[4m";
	const inline std::string blink_slow    = "\033[5m";
	const inline std::string blink_fast    = "\033[6m";
	const inline std::string invert        = "\033[7m";
	const inline std::string hide          = "\033[8m";
	const inline std::string strike        = "\033[9m";
	const inline std::string primary       = "\033[10m";
	const inline std::string alternate1    = "\033[11m";
	const inline std::string alternate2    = "\033[12m";
	const inline std::string alternate3    = "\033[13m";
	const inline std::string alternate4    = "\033[14m";
	const inline std::string alternate5    = "\033[15m";
	const inline std::string alternate6    = "\033[16m";
	const inline std::string alternate7    = "\033[17m";
	const inline std::string alternate8    = "\033[18m";
	const inline std::string alternate9    = "\033[19m";
	const inline std::string fraktur       = "\033[20m";
	const inline std::string doubleUnder   = "\033[21m";
	const inline std::string noBold        = "\033[22m";
	const inline std::string noItalic      = "\033[23m";
	const inline std::string noUnder       = "\033[24m";
	const inline std::string noBlink       = "\033[25m";
	const inline std::string propSpacing   = "\033[26m";
	const inline std::string noReverse     = "\033[27m";
	const inline std::string reveal        = "\033[28m";
	const inline std::string noStrike      = "\033[29m";
	const inline std::string noPropSpacing = "\033[50m";
	const inline std::string framed        = "\033[51m";
	const inline std::string encircled     = "\033[52m";
	const inline std::string overlined     = "\033[53m";

	const inline std::string ideogramUnderline       = "\033[60m";
	const inline std::string ideogramDoubleUnderline = "\033[61m";
	const inline std::string ideogramOverline        = "\033[62m";
	const inline std::string ideogramDoubleOverline  = "\033[63m";
	const inline std::string ideogramStress          = "\033[64m";
	const inline std::string ideogramReset           = "\033[65m";

	const inline std::string superscript  = "\033[73m";
	const inline std::string subscript    = "\033[74m";
	const inline std::string normalscript = "\033[75m";

	// tests general ANSI codes (most of 0-75, no colors)
	//void generalCodesTest();
	void generalCodesTest();

	// standard 4-bit color codes

	const inline std::string black   = "\033[30m";
	const inline std::string red     = "\033[31m";
	const inline std::string green   = "\033[32m";
	const inline std::string yellow  = "\033[33m";
	const inline std::string blue    = "\033[34m";
	const inline std::string magenta = "\033[35m";
	const inline std::string cyan    = "\033[36m";
	const inline std::string white   = "\033[37m";
	
	const inline std::string black_back   = "\033[40m";
	const inline std::string red_back     = "\033[41m";
	const inline std::string green_back   = "\033[42m";
	const inline std::string yellow_back  = "\033[43m";
	const inline std::string blue_back    = "\033[44m";
	const inline std::string magenta_back = "\033[45m";
	const inline std::string cyan_back    = "\033[46m";
	const inline std::string white_back   = "\033[47m";

	const inline std::string black_bright   = "\033[90m";
	const inline std::string red_bright     = "\033[91m";
	const inline std::string green_bright   = "\033[92m";
	const inline std::string yellow_bright  = "\033[93m";
	const inline std::string blue_bright    = "\033[94m";
	const inline std::string magenta_bright = "\033[95m";
	const inline std::string cyan_bright    = "\033[96m";
	const inline std::string white_bright   = "\033[97m";

	const inline std::string black_back_bright   = "\033[100m";
	const inline std::string red_back_bright     = "\033[101m";
	const inline std::string green_back_bright   = "\033[102m";
	const inline std::string yellow_back_bright  = "\033[103m";
	const inline std::string blue_back_bright    = "\033[104m";
	const inline std::string magenta_back_bright = "\033[105m";
	const inline std::string cyan_back_bright    = "\033[106m";
	const inline std::string white_back_bright   = "\033[107m";

	const inline std::string cursor_home      = "\033[H";
	const inline std::string cursor_visible   = "\033[?25H";
	const inline std::string cursor_invisible = "\033[?25l";
	
	const inline std::string clearToEnd      = "\033[0J";
	const inline std::string clearFromStart  = "\033[1J";
	const inline std::string clear           = "\033[2J";
	const inline std::string clearScrollback = "\033[3J";
	const inline std::string clearLine       = "\033[2K";
	const inline std::string clearLineToEnd     = "\033[0K";
	const inline std::string clearLineFromStart = "\033[1K";
	
	const inline std::string saveScreen    = "\033[?47h";
	const inline std::string restoreScreen = "\033[?47l";
	
	const inline std::string enableAltBuffer  = "\033[?1049h";
	const inline std::string disableAltBuffer = "\033[?1049l";
	
	const inline std::string enableLineWrapping  = "\033[=7h";
	const inline std::string disableLineWrapping = "\033[=7l";

	const inline std::string enableMouseTracking  = "\033[?1000h";
	const inline std::string disableMouseTracking = "\033[?1000hl";

	// tests the standard ANSI 4-bit color codes
	void colorTest();

	const inline std::string resetUnderlineColor = "\033[59m";

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
		std::string makeColor(uint8_t r, uint8_t g, uint8_t b, bool background = true);

		// sets underline color using 24-bit color
		std::string underlineColor(int r, int g, int b);

		void underlineColorTest();

		void colorTest();
	}
}