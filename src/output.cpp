#include <iostream>
#include <cstdint>
#include <cassert>
#include <cmath>
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
		// TODO
		//std::string prevColor;
		for (uint32_t x = 0; x < width; ++x) {
			Cell c = buffer[y*width + x];
			frame += c.color_fore + c.color_back + c.ch;

			if (c.color_fore.size() || c.color_back.size() /*&& prevColor != c.color*/) frame += ANSI::reset;
			//prevColor = c.color;
		}
		frame += ANSI::reset + '\n';
	}

	#ifdef _WIN32
		DWORD written;
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), frame.c_str(), frame.size(), &written, NULL);
	#else
		write(STDOUT_FILENO, frame.data(), frame.size());
	#endif
}

void Renderer::clear(const Cell& replacement) {
	for (uint32_t y = 0; y < height; ++y) {
		for (uint32_t x = 0; x < width; ++x) {
			buffer[y*width + x] = replacement;
		}
	}
}

void Renderer::resize(int _width, int _height) {
	width = _width;
	height = _height;
	buffer.resize(width*height, Cell{".","", ""});
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


namespace ANSI {

	void generalCodesTest() {
		std::cout
			<< ANSI::reset         << "Hello (regular)"                       << '\n'
			<< ANSI::bold          << "Hello (bold)"           << ANSI::reset << '\n'
			<< ANSI::dim           << "Hello (dim)"            << ANSI::reset << '\n'
			<< ANSI::italic        << "Hello (italic)"         << ANSI::reset << '\n'
			<< ANSI::underline     << "Hello (underline)"      << ANSI::reset << '\n'
			<< ANSI::blink_slow    << "Hello (blink_slow)"     << ANSI::reset << '\n'
			<< ANSI::blink_fast    << "Hello (blink_fast)"     << ANSI::reset << '\n'
			<< ANSI::invert        << "Hello (invert)"         << ANSI::reset << '\n'
			<< ANSI::hide          << "Hello (hide)"           << ANSI::reset << '\n'
			<< ANSI::strike        << "Hello (strike)"         << ANSI::reset << '\n'
			<< ANSI::primary       << "Hello (primary)"        << ANSI::reset << '\n'
			<< ANSI::alternate1    << "Hello (alternate1)"     << ANSI::reset << '\n'
			<< ANSI::alternate2    << "Hello (alternate2)"     << ANSI::reset << '\n'
			<< ANSI::alternate3    << "Hello (alternate3)"     << ANSI::reset << '\n'
			<< ANSI::alternate4    << "Hello (alternate4)"     << ANSI::reset << '\n'
			<< ANSI::alternate5    << "Hello (alternate5)"     << ANSI::reset << '\n'
			<< ANSI::alternate6    << "Hello (alternate6)"     << ANSI::reset << '\n'
			<< ANSI::alternate7    << "Hello (alternate7)"     << ANSI::reset << '\n'
			<< ANSI::alternate8    << "Hello (alternate8)"     << ANSI::reset << '\n'
			<< ANSI::alternate9    << "Hello (alternate9)"     << ANSI::reset << '\n'
			<< ANSI::fraktur       << "Hello (fraktur)"        << ANSI::reset << '\n'
			<< ANSI::doubleUnder   << "Hello (doubleUnder)"    << ANSI::reset << '\n'
			<< ANSI::noBold        << "Hello (noBold)"         << ANSI::reset << '\n'
			<< ANSI::noItalic      << "Hello (noItalic)"       << ANSI::reset << '\n'
			<< ANSI::noUnder       << "Hello (noUnder)"        << ANSI::reset << '\n'
			<< ANSI::noBlink       << "Hello (noBlink)"        << ANSI::reset << '\n'
			<< ANSI::propSpacing   << "Hello (propSpacing)"    << ANSI::reset << '\n'
			<< ANSI::noReverse     << "Hello (noReverse)"      << ANSI::reset << '\n'
			<< ANSI::reveal        << "Hello (reveal)"         << ANSI::reset << '\n'
			<< ANSI::noStrike      << "Hello (noStrike)"       << ANSI::reset << '\n'
			<< ANSI::noPropSpacing << "Hello (noPropSpacing)"  << ANSI::reset << '\n'
			<< ANSI::framed        << "Hello (framed)"         << ANSI::reset << '\n'
			<< ANSI::encircled     << "Hello (encircled)"      << ANSI::reset << '\n'
			<< ANSI::overlined     << "Hello (overlined)"      << ANSI::reset << '\n'
			<< '\n'
			<< ANSI::ideogramUnderline       << "Hello (ideogramUnderline)"       << ANSI::reset << '\n'
			<< ANSI::ideogramDoubleUnderline << "Hello (ideogramDoubleUnderline)" << ANSI::reset << '\n'
			<< ANSI::ideogramOverline        << "Hello (ideogramOverline)"        << ANSI::reset << '\n'
			<< ANSI::ideogramDoubleOverline  << "Hello (ideogramDoubleOverline)"  << ANSI::reset << '\n'
			<< ANSI::ideogramStress          << "Hello (ideogramStress)"          << ANSI::reset << '\n'
			<< ANSI::ideogramReset           << "Hello (ideogramReset)"           << ANSI::reset << '\n'
			<< '\n'
			<< ANSI::superscript  << "Hello (superscript)"  << ANSI::reset << '\n'
			<< ANSI::subscript    << "Hello (subscript)"    << ANSI::reset << '\n'
			<< ANSI::normalscript << "Hello (normalscript)" << ANSI::reset << '\n'
			<< std::endl;
	}

	void colorTest() {
		std::cout
			<< ANSI::black   << "###" << ANSI::reset << ' '
			<< ANSI::red     << "###" << ANSI::reset << ' '
			<< ANSI::green   << "###" << ANSI::reset << ' '
			<< ANSI::yellow  << "###" << ANSI::reset << ' '
			<< ANSI::blue    << "###" << ANSI::reset << ' '
			<< ANSI::magenta << "###" << ANSI::reset << ' '
			<< ANSI::cyan    << "###" << ANSI::reset << ' '
			<< ANSI::white   << "###" << ANSI::reset << '\n'
			<< ANSI::black_back   << "###" << ANSI::reset << ' '
			<< ANSI::red_back     << "###" << ANSI::reset << ' '
			<< ANSI::green_back   << "###" << ANSI::reset << ' '
			<< ANSI::yellow_back  << "###" << ANSI::reset << ' '
			<< ANSI::blue_back    << "###" << ANSI::reset << ' '
			<< ANSI::magenta_back << "###" << ANSI::reset << ' '
			<< ANSI::cyan_back    << "###" << ANSI::reset << ' '
			<< ANSI::white_back   << "###" << ANSI::reset << '\n'
			<< ANSI::black_bright   << "###" << ANSI::reset << ' '
			<< ANSI::red_bright     << "###" << ANSI::reset << ' '
			<< ANSI::green_bright   << "###" << ANSI::reset << ' '
			<< ANSI::yellow_bright  << "###" << ANSI::reset << ' '
			<< ANSI::blue_bright    << "###" << ANSI::reset << ' '
			<< ANSI::magenta_bright << "###" << ANSI::reset << ' '
			<< ANSI::cyan_bright    << "###" << ANSI::reset << ' '
			<< ANSI::white_bright   << "###" << ANSI::reset << '\n'
			<< ANSI::black_back_bright   << "###" << ANSI::reset << ' '
			<< ANSI::red_back_bright     << "###" << ANSI::reset << ' '
			<< ANSI::green_back_bright   << "###" << ANSI::reset << ' '
			<< ANSI::yellow_back_bright  << "###" << ANSI::reset << ' '
			<< ANSI::blue_back_bright    << "###" << ANSI::reset << ' '
			<< ANSI::magenta_back_bright << "###" << ANSI::reset << ' '
			<< ANSI::cyan_back_bright    << "###" << ANSI::reset << ' '
			<< ANSI::white_back_bright   << "###" << ANSI::reset << std::endl;
	}

	int findColorType(const std::string& code) {
		if (!code.size() || code[0] != '\033') return -1;
		int section = std::stoi(code.substr(2,2));
		// in case of 100-107
		if (section == 10) section = std::stoi(code.substr(2,3));

		// 4-bit foreground
		if (30 <= section && section <= 37) return 0;
		// 4-bit background
		if (40 <= section && section <= 47) return 1;
		// 4-bit foreground (bright)
		if (90 <= section && section <= 97) return 2;
		// 4-bit background (bright)
		if (100 <= section && section <= 107) return 3;

		// 8-bit foreground
		if (section == 38 && code.substr(5,1) == "5") return 4;
		// 8-bit background
		if (section == 48 && code.substr(5,1) == "5") return 5;

		// 24-bit foreground
		if (section == 38 && code.substr(5,1) == "2") return 6;
		// 24-bit background
		if (section == 48 && code.substr(5,1) == "2") return 7;

		return -1;
	}

	std::string invertColor(const std::string& code) {
		int section = std::stoi(code.substr(2, 2));
		// in case of 100-107
		if (section == 10) section = std::stoi(code.substr(2,3));

		// 4-bit foreground
		if (30 <= section && section <= 37) {
			std::string temp = code;
			return temp.replace(2, 2, std::to_string(section+10));
		}
		// 4-bit background
		if (40 <= section && section <= 47) {
			std::string temp = code;
			return temp.replace(2, 2, std::to_string(section-10));
		}
		// 4-bit foreground (bright)
		if (90 <= section && section <= 97) {
			std::string temp = code;
			return temp.replace(2, 2, std::to_string(section+10));
		}
		// 4-bit background (bright)
		if (100 <= section && section <= 107) {
			std::string temp = code;
			return temp.replace(2, 3, std::to_string(section-10));
		}

		// 8-bit, 24-bit foreground
		if (section == 38) {
			std::string temp = code;
			return temp.replace(2, 2, std::to_string(section+10));
		}
		// 8-bit, 24-bit background
		if (section == 48) {
			std::string temp = code;
			return temp.replace(2, 2, std::to_string(section-10));
		}

		// fallthrough
		return ANSI::red_back_bright;
	}

	std::string Color_8bit::makeColor(int r, int g, int b, bool background) {
		assert(
			0 <= r && r <= 5 && 
			0 <= g && g <= 5 && 
			0 <= b && b <= 5
		);
		return (background ? "\033[48;5;" : "\033[38;5;") + std::to_string(16 + 36*r + 6*g + b) + "m";
	}

	std::string Color_8bit::makeColor(int n, bool background) {
		return (background ? "\033[48;5;" : "\033[38;5;") + std::to_string(n) + "m";
	}

	std::string Color_8bit::makeGray(int level, bool background) {
		assert(1 <= level && level <= 24);
		return (background ? "\033[48;5;" : "\033[38;5;") + std::to_string(231+level) + "m";
	}

	std::string Color_8bit::underlineColor(int r, int g, int b) {
		return "\033[58;5;" + std::to_string(16 + 36*r + 6*g + b) + 'm';
	}

	void Color_8bit::underlineColorTest() {
		for (int r = 0; r < 5; r++) {
			for (int g = 0; g < 5; g++) {
				std::cout << ANSI::underline;
				for (int b = 0; b < 5; b++) {
					std::cout 
						<< ANSI::Color_8bit::underlineColor(r*50,g*50,b*50) 
						<< "Hi";
				}
			}
			std::cout << ANSI::reset << std::endl;
		}
	}

	void Color_8bit::colorTest() {
		for (int r = 0; r < 6; r++) {
			for (int g = 0; g < 6; g++) {
				for (int b = 0; b < 6; b++) {
					std::cout 
						<< ANSI::Color_8bit::makeColor(r,g,b, true) 
						<< ' ' << ((16 + 36*r + 6*g + b) > 99 ? '\0' : ' ')
						<< std::to_string(16 + 36*r + 6*g + b) << ' ';
				}
			}
			std::cout << ANSI::reset << std::endl;
		}
		for (int a = 1; a < 25; ++a) {
			std::cout
				<< ANSI::Color_8bit::makeGray(a, true) 
				<< "  " << std::to_string(231+a) << "  ";
		}
		std::cout << ANSI::reset << std::endl;
	}


	std::string Color_24bit::makeColor(uint8_t r, uint8_t g, uint8_t b, bool background) {
		return (background ? "\033[48;2;" : "\033[38;2;") + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
	}

	std::string Color_24bit::underlineColor(int r, int g, int b) {
		return "\033[58;2;" + std::to_string(r) + ';' + std::to_string(g) + ';' + std::to_string(b) + ';' + 'm';
	}

	void Color_24bit::underlineColorTest() {
		for (int r = 0; r < 5; r++) {
			for (int g = 0; g < 5; g++) {
				std::cout << ANSI::underline;
				for (int b = 0; b < 5; b++) {
					std::cout 
						<< ANSI::Color_24bit::underlineColor(r*50,g*50,b*50) 
						<< "Hi";
				}
			}
			std::cout << ANSI::reset << std::endl;
		}
	}

	void Color_24bit::colorTest() {
		for (int r = 0; r < 10; r++) {
			for (int g = 0; g < 10; g++) {
				for (int b = 0; b < 10; b++) {
					std::cout 
						<< ANSI::Color_24bit::makeColor(r*25,g*25,b*25, true) 
						<< "  ";
				}
			}
			std::cout << ANSI::reset << std::endl;
		}
		for (int n = 55; n < 255; ++n) {
			std::cout << ANSI::Color_24bit::makeColor(n,n,n, true) << " "; 
		} std::cout << ANSI::reset << std::endl;
		for (int n = 55; n < 255; ++n) {
			std::cout << ANSI::Color_24bit::makeColor(n,100,200, true) << " "; 
		} std::cout << ANSI::reset << std::endl;
		for (int n = 55; n < 255; ++n) {
			std::cout << ANSI::Color_24bit::makeColor(80,n,n/2, true) << " "; 
		} std::cout << ANSI::reset << std::endl;
		for (int s = 100; s > 0; s -= 8) {
			for (int n = 0; n < 200; ++n) {
				std::cout << ANSI::Color_24bit::makeColor(255-n, 255*sin(static_cast<float>(n)/s), (n/3)+s, true) << " "; 
			} std::cout << ANSI::reset << std::endl;
		}
	}

}