#pragma once

#include "output.hpp"


// if true, uses buffered terminal inputs instead of raw keyboard keystates
inline bool INPUT_SAFE_MODE = true;

// path to search for input devices
inline const char* INPUT_DEVICE_SEARCH_PATH = "/proc/bus/input/devices";

// fps
inline float FPS = 30.0f;


// animation duration for cursor (seconds)
inline float ANIM_CURSOR = 0.5f;


// screen width
inline int SCREEN_WIDTH = 20;
// screen height
inline int SCREEN_HEIGHT = 8;

// width of art
inline int ART_WIDTH = 10;
// height of art
inline int ART_HEIGHT = 5;

// if true, ASCII art instead of ANSI art
inline bool ART_IS_ASCII = false;


// size of side panel
constexpr int PANEL_SIZE = 30;

// size of bottom panel
constexpr int BOTTOM_PANEL_SIZE = 2;


// mode of operation
// 0 = standard, 1 = simple
inline int MODE = 0;

inline std::string HOT_KEY_1 = "█";
inline std::string HOT_KEY_2 = "▓";
inline std::string HOT_KEY_3 = "▒";
inline std::string HOT_KEY_4 = "░";
inline std::string HOT_KEY_5 = "▀";
inline std::string HOT_KEY_6 = "▄";
inline std::string HOT_KEY_7 = "▌";
inline std::string HOT_KEY_8 = "▐";
inline std::string HOT_KEY_9 = "■";
inline std::string HOT_KEY_0 = "☻";

inline std::string HOT_COLOR_Q = ANSI::red;
inline std::string HOT_COLOR_A = ANSI::red_back;
inline std::string HOT_COLOR_W = ANSI::blue;
inline std::string HOT_COLOR_S = ANSI::blue_back;
inline std::string HOT_COLOR_E = ANSI::green;
inline std::string HOT_COLOR_D = ANSI::green_back;
inline std::string HOT_COLOR_R = ANSI::yellow;
inline std::string HOT_COLOR_F = ANSI::yellow_back;
inline std::string HOT_COLOR_T = ANSI::white;
inline std::string HOT_COLOR_G = ANSI::white_back;
inline std::string HOT_COLOR_Y = ANSI::black;
inline std::string HOT_COLOR_H = ANSI::black_back;