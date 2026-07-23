/* 
 * ANSI codes and functions
**/

#include <iostream>
#include <cstdint>
#include <cassert>
#include <cmath>

#include "ANSI.hpp"

namespace ANSI {

	const std::string reset         = "\033[0m";
	const std::string bold          = "\033[1m";
	const std::string dim           = "\033[2m";
	const std::string italic        = "\033[3m";
	const std::string underline     = "\033[4m";
	const std::string blink_slow    = "\033[5m";
	const std::string blink_fast    = "\033[6m";
	const std::string invert        = "\033[7m";
	const std::string hide          = "\033[8m";
	const std::string strike        = "\033[9m";
	const std::string primary       = "\033[10m";
	const std::string alternate1    = "\033[11m";
	const std::string alternate2    = "\033[12m";
	const std::string alternate3    = "\033[13m";
	const std::string alternate4    = "\033[14m";
	const std::string alternate5    = "\033[15m";
	const std::string alternate6    = "\033[16m";
	const std::string alternate7    = "\033[17m";
	const std::string alternate8    = "\033[18m";
	const std::string alternate9    = "\033[19m";
	const std::string fraktur       = "\033[20m";
	const std::string doubleUnder   = "\033[21m";
	const std::string noBold        = "\033[22m";
	const std::string noItalic      = "\033[23m";
	const std::string noUnder       = "\033[24m";
	const std::string noBlink       = "\033[25m";
	const std::string propSpacing   = "\033[26m";
	const std::string noReverse     = "\033[27m";
	const std::string reveal        = "\033[28m";
	const std::string noStrike      = "\033[29m";
	const std::string noPropSpacing = "\033[50m";
	const std::string framed        = "\033[51m";
	const std::string encircled     = "\033[52m";
	const std::string overlined     = "\033[53m";

	const std::string ideogramUnderline       = "\033[60m";
	const std::string ideogramDoubleUnderline = "\033[61m";
	const std::string ideogramOverline        = "\033[62m";
	const std::string ideogramDoubleOverline  = "\033[63m";
	const std::string ideogramStress          = "\033[64m";
	const std::string ideogramReset           = "\033[65m";

	const std::string superscript  = "\033[73m";
	const std::string subscript    = "\033[74m";
	const std::string normalscript = "\033[75m";

	// standard 4-bit color codes

	const std::string black   = "\033[30m";
	const std::string red     = "\033[31m";
	const std::string green   = "\033[32m";
	const std::string yellow  = "\033[33m";
	const std::string blue    = "\033[34m";
	const std::string magenta = "\033[35m";
	const std::string cyan    = "\033[36m";
	const std::string white   = "\033[37m";
	
	const std::string black_back   = "\033[40m";
	const std::string red_back     = "\033[41m";
	const std::string green_back   = "\033[42m";
	const std::string yellow_back  = "\033[43m";
	const std::string blue_back    = "\033[44m";
	const std::string magenta_back = "\033[45m";
	const std::string cyan_back    = "\033[46m";
	const std::string white_back   = "\033[47m";

	const std::string black_bright   = "\033[90m";
	const std::string red_bright     = "\033[91m";
	const std::string green_bright   = "\033[92m";
	const std::string yellow_bright  = "\033[93m";
	const std::string blue_bright    = "\033[94m";
	const std::string magenta_bright = "\033[95m";
	const std::string cyan_bright    = "\033[96m";
	const std::string white_bright   = "\033[97m";

	const std::string black_back_bright   = "\033[100m";
	const std::string red_back_bright     = "\033[101m";
	const std::string green_back_bright   = "\033[102m";
	const std::string yellow_back_bright  = "\033[103m";
	const std::string blue_back_bright    = "\033[104m";
	const std::string magenta_back_bright = "\033[105m";
	const std::string cyan_back_bright    = "\033[106m";
	const std::string white_back_bright   = "\033[107m";

	const std::string cursor_home      = "\033[H";
	const std::string cursor_visible   = "\033[?25h";
	const std::string cursor_invisible = "\033[?25l";
	
	const std::string clearToEnd      = "\033[0J";
	const std::string clearFromStart  = "\033[1J";
	const std::string clear           = "\033[2J";
	const std::string clearScrollback = "\033[3J";
	const std::string clearLine       = "\033[2K";
	const std::string clearLineToEnd     = "\033[0K";
	const std::string clearLineFromStart = "\033[1K";
	
	const std::string saveScreen    = "\033[?47h";
	const std::string restoreScreen = "\033[?47l";
	
	const std::string enableAltBuffer  = "\033[?1049h";
	const std::string disableAltBuffer = "\033[?1049l";
	
	const std::string enableLineWrapping  = "\033[=7h";
	const std::string disableLineWrapping = "\033[=7l";

	const std::string enableMouseTracking  = "\033[?1000h";
	const std::string disableMouseTracking = "\033[?1000hl";
	
	const std::string resetUnderlineColor = "\033[59m";


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
		// TODO: idk
		return ANSI::strike;
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