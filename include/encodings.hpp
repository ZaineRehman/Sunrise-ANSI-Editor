/*
 * File encodings (UTF-8 and CP437)
 * Converting between the two
**/

#pragma once

#include <iostream>
#include <cstdint>
#include <vector>


// converts a CP437 character to UTF-8
std::string convert_cp437_utf8(const std::string& ch);

// converts a UTF-8 character to CP437
std::string convert_utf8_cp437(const std::string& ch);