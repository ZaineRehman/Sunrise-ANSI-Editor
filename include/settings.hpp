#pragma once

#include "output.hpp"


/*
 *  == TODO ==
 * [x] extend art bounds when inserting outside of range
 * [x] clean up main.cpp
 * [ ] proper windows support
 * [ ] proper linux support
 * [ ] color catalogue picker
 * [ ] character catalogue picker
 * [ ] stop re-calculating some things every single frame
 * [ ] export art to .ans file
 * [ ] ANSI mode
 * [ ] minimum screen size
 * [ ] custom palettes
 * [ ] music
 * [ ] mouse input
 * [ ] highlighting sections for copy/paste
 * [ ] loading art from file
 * [ ] saving preferences
 * [ ] changing to/from input safe mode
 * [ ] creating animations
 * [ ] animation time delay
 * [ ] animations to gif
 * [ ] CHECK ALL TODO COMMENTS
 * [ ] gradient maker
 * [ ] trim empty spaces from art
 * [ ] downgrading color mode -> remove those codes from art
 * [ ] finalize window sizes
**/


inline std::string DEBUG_STR = "";


/*
 * [date][minor]-[program condition][dev stage]
 * 
 * date: [month][year (2 digits)]
 * 
 * minor: for multiple versions in the same month
 * 
 * program conditions: 
 * 		N = not working (critical bug)
 * 		U = under development (not ready for release)
 * 		W = working (needs further testing before release)
 * 		R = release
 * 		X = experimental test build
 * 
 * development stages: 
 * 		A = alpha (many key features not implemented)
 * 		B = beta (most/all features implemented, needs refining)
 * 		W = working (all planned features implemented and working well)
**/
constexpr const char* VERSION = "v07261-UA";


// if true, uses buffered terminal inputs instead of raw keyboard keystates
inline bool INPUT_SAFE_MODE = true;

// path to search for input devices
inline constexpr const char* INPUT_DEVICE_SEARCH_PATH = "/proc/bus/input/devices";

// default background char
inline const std::string DEFAULT_BACK = ".";


// fps
inline float FPS = 30.0f;

// animation duration for cursor (seconds)
inline float ANIM_CURSOR = 0.5f;

// how often to do a more expensive analysis frame
inline int ANALYSIS_FREQUENCY = 10; 


// border color
inline std::string BORDER_COLOR = ANSI::bold;
// key highlight color
inline std::string KEY_COLOR = ANSI::bold;

// screen width
inline int SCREEN_WIDTH = 75;
// screen height
inline int SCREEN_HEIGHT = 20;

// size of side panel
constexpr int PANEL_SIZE = 40;

// size of bottom panel
constexpr int BOTTOM_PANEL_SIZE = 2;

// colors in palette
constexpr int PALETTE_SIZE = 16;

// color catalogue x
constexpr float COLOR_CATALOGUE_X = 32.0f;

// color catalogue y
constexpr float COLOR_CATALOGUE_Y = 6.0f;


// if true, using direct keyboard inputs
inline bool DIRECT_KEY_INPUTS = false;

// if true, use HJKL instead of UHJK for arrows, vim style
inline bool VIM_ARROWS = false;

// type of color
// 0 = none,  1 = 4-bit,  2 = 8-bit,  3 = 24-bit
inline int COLOR_MODE = 0;

inline std::string HOTKEY_CHAR_1 = "█";
inline std::string HOTKEY_CHAR_2 = "▓";
inline std::string HOTKEY_CHAR_3 = "▒";
inline std::string HOTKEY_CHAR_4 = "░";
inline std::string HOTKEY_CHAR_5 = " ";
inline std::string HOTKEY_CHAR_6 = "▌";
inline std::string HOTKEY_CHAR_7 = "▐";
inline std::string HOTKEY_CHAR_8 = "▀";
inline std::string HOTKEY_CHAR_9 = "▄";
inline std::string HOTKEY_CHAR_0 = "■";

inline std::string HOTKEY_COLOR_Q = ANSI::red;
inline std::string HOTKEY_COLOR_A = ANSI::red_back;
inline std::string HOTKEY_COLOR_W = ANSI::blue;
inline std::string HOTKEY_COLOR_S = ANSI::blue_back;
inline std::string HOTKEY_COLOR_E = ANSI::green;
inline std::string HOTKEY_COLOR_D = ANSI::green_back;
inline std::string HOTKEY_COLOR_R = ANSI::yellow;
inline std::string HOTKEY_COLOR_F = ANSI::yellow_back;
inline std::string HOTKEY_COLOR_T = ANSI::white;
inline std::string HOTKEY_COLOR_G = ANSI::white_back;
inline std::string HOTKEY_COLOR_Y = ANSI::black;
inline std::string HOTKEY_COLOR_H = ANSI::black_back;