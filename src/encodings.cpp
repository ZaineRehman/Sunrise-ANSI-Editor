/*
 * File encodings (UTF-8 and CP437)
 * Converting between the two
 * 
 * used AI on parts of this file
**/

#include "encodings.hpp"

#include <iostream>
#include <cstdint>
#include <vector>


inline constexpr uint32_t cp437_to_utf8[128] = {
	0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7, // 128-135
	0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5, // 136-143
	0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9, // 144-151
	0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x20A7, 0x0192, // 152-159
	0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA, // 160-167
	0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB, // 168-175
	0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556, // 176-183 (shade / box)
	0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510, // 184-191
	0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F, // 192-199
	0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567, // 200-207
	0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B, // 208-215
	0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580, // 216-223 (blocks)
	0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x00B5, 0x03C4, // 224-231 (math / greek)
	0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229, // 232-239
	0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248, // 240-247
	0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0  // 248-255
};


// converts a CP437 character to UTF-8
std::string convert_cp437_utf8(const std::string& ch) {
	if (ch.empty()) return "";

	uint8_t b = static_cast<uint8_t>(ch[0]);

	// if standard ascii, our job just got easier
	if (b < 128) {
		return std::string(1, static_cast<char>(b));
	}

	// find value in table
	uint32_t cp = cp437_to_utf8[b - 128];
	std::string utf8_result;

	// encode into 2 or 3 UTF-8 bytes
	if (cp <= 0x7FF) {
		utf8_result += static_cast<char>(0xC0 | ((cp >> 6) & 0x1F));
		utf8_result += static_cast<char>(0x80 | (cp & 0x3F));
	} else if (cp <= 0xFFFF) {
		utf8_result += static_cast<char>(0xE0 | ((cp >> 12) & 0x0F));
		utf8_result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
		utf8_result += static_cast<char>(0x80 | (cp & 0x3F));
	}

	return utf8_result;
}

// converts a UTF-8 character to CP437
std::string convert_utf8_cp437(const std::string& ch) {
	if (ch.empty()) return "";

	uint32_t cp = 0;
	size_t len = ch.length();
	
	// decode multi-byte UTF-8 string into single unicode code point
	uint8_t b0 = static_cast<uint8_t>(ch[0]);
	if (b0 <= 0x7F) {
		cp = b0;
	}else if ((b0 & 0xE0) == 0xC0 && len >= 2) {
		cp = ((b0 & 0x1F) << 6) |
			(static_cast<uint8_t>(ch[1]) & 0x3F);
	} else if ((b0 & 0xF0) == 0xE0 && len >= 3) {
		cp = ((b0 & 0x0F) << 12) | 
			((static_cast<uint8_t>(ch[1]) & 0x3F) << 6) | 
			 (static_cast<uint8_t>(ch[2]) & 0x3F);
	} /*else if ((b0 & 0xF8) == 0xF0 && len >= 4) {   // doesnt matter for CP437
		cp = ((b0 & 0x07) << 18) |
			((static_cast<uint8_t>(ch[1]) & 0x3F) << 12) |
			((static_cast<uint8_t>(ch[2]) & 0x3F) << 6) |
			 (static_cast<uint8_t>(ch[3]) & 0x3F);
	}*/

	// map code point back to CP437 byte
	if (cp < 128) {
		return std::string(1, static_cast<char>(cp));
	}

	// find in table
	for (int i = 0; i < 128; ++i) {
		if (cp437_to_utf8[i] == cp) {
			return std::string(1, static_cast<char>(128 + i));
		}
	}

	// didnt find? whoopsies!
	return "?"; 
}