#include <iostream>
#include <cstdint>
#include <cassert>
#include <vector>
#include <string>

#include "output.hpp"

#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <sys/ioctl.h>
#endif


std::pair<int,int> getTerminalDimensions() {
	#ifdef _WIN32
		CONSOLE_SCREEN_BUFFER_INFO info;
		HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

		if (GetConsoleScreenBufferInfo(hstdout, &info)) {
			return std::make_pair<int,int>(info.srWindow.Right-info.srWindow.Left+1, info.srWindow.Bottom-info.srWindow.Top+1);
		} else {
			std::cerr << "ERR: could not find terminal dimensions" << std::endl;
			while (1);
		}
	#else
		struct winsize w;
		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
			return std::make_pair<int,int>(w.ws_col,w.ws_row);
		} else {
			std::cerr << "ERR: could not find terminal dimensions" << std::endl;
			while (1);
		}
	#endif
}


void Renderer::put(uint32_t x, uint32_t y, const Cell& cell) {
	assert(x < width && y < height);

	buffer[y*width + x] = cell;
}

void Renderer::edit(uint32_t x, uint32_t y, const std::string& str, char col) {
	assert(x < width && y < height);

	     if (col == 0) buffer[y*width + x].color_fore = str;
	else if (col == 1) buffer[y*width + x].color_back = str;
	else buffer[y*width + x].ch    = str;
}

void Renderer::putString(uint32_t x, uint32_t y, const CellString& cells) {
	assert(x+cells.size()-1);  // what the fuck is this

	for (int i = 0; i < static_cast<int>(cells.size()); ++i) {
		// (2, 34)
		buffer[y*width + x + i] = cells[i];
	}
}

Cell Renderer::get(uint32_t x, uint32_t y) const {
	assert(x < width && y < height);

	return buffer[y*width + x];
}

void Renderer::render() const {
	std::string frame = ANSI::cursor_home;

	for (uint32_t y = 0; y < height; ++y) {
		//std::string prevColor;
		for (uint32_t x = 0; x < width; ++x) {
			Cell c = buffer[y*width + x];
			frame += c.color_fore + c.color_back + c.ch;

			if (c.color_fore.size() || c.color_back.size() /*&& prevColor != c.color*/) frame += ANSI::reset;
			//prevColor = c.color;
		}
		frame += ANSI::reset + (y != height-1 ? '\n' : '\0');
	}

	#ifdef _WIN32
		DWORD written;
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), frame.c_str(), frame.size(), &written, NULL);
	#else
		write(STDOUT_FILENO, frame.data(), frame.size());
	#endif
}

void Renderer::fill(const Cell& replacement) {
	for (uint32_t y = 0; y < height; ++y) {
		for (uint32_t x = 0; x < width; ++x) {
			buffer[y*width + x] = replacement;
		}
	}
}

void Renderer::resize(int _width, int _height) {
	width = _width;
	height = _height;
	buffer.resize(width*height, Cell{DEFAULT_BACK,"", ""});
}


void clear() {
	#ifdef _WIN32
		system("cls");
	#else
		system("clear");
	#endif
}

int findHighestColorCode(const CellString& cells) {
	int highest = 0;
	for (size_t c = 0; c < cells.size(); ++c) {
		int type1 = ANSI::findColorType(cells[c].color_fore) + 1;
		int type2 = ANSI::findColorType(cells[c].color_back) + 1;
		if (type1 > highest) highest = type1;
		if (type2 > highest) highest = type2;

		if (highest == 2) break;
	}
	return highest;
}


#include "ANSI.hpp"
