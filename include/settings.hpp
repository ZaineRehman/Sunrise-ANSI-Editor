/* 
 * Settings for the program
**/

#pragma once


/*
 *  == TODO ==
 * [x] extend art bounds when inserting outside of range
 * [x] clean up main.cpp
 * [x] proper windows input support
 * [x] color catalogue picker
 * [x] character catalogue picker
 * [x] custom palettes
 * [x] export art to .ans file
 * [ ] session logging
 * [ ] custom session IDs
 * [ ] load palettes to/from file
 * [ ] proper linux input support
 * [ ] stop re-calculating some things every single frame
 * [ ] ANSI mode
 * [ ] ANSI editor mode
 * [ ] minimum screen size
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
 *      [ ] recognize the colors and appropriately downgrade?
 * [ ] finalize window sizes
 * [ ] make input checkers use separate thread (mutex?)
 * [ ] win terminal shift+arrow and ctrl+arrow
 * [ ] clean up files
 * [ ] add a support mode for only 4-bit color
 * [ ] tutorial
 * [ ] only re-render when needed
 * [ ] you dont need to loop through every cell on the screen
 * [ ] export file location picker
 * [ ] autosave
 * [ ] screen scrolling
 * [ ] more than just color codes?
 * [ ] popups
 * SIDE PANEL: 
 *   [x] sunrise text
 *   [x] char hotkeys
 *   [ ] settings button
 *   		[ ] music volume
 *   		[ ] input safe mode
 *   		[ ] fps?
 *   		[ ] cursor animation
 *   		[ ] border color
 *   		[ ] key color
 *   		[ ] side panel size
 *   		[ ] bottom panel size
 *   [ ] song name
 *   [ ] art size
 *   [ ] export button
 *   [ ] copy/paste button
 *   [ ] load from file button
 *   [ ] animation buttons
 *   [x] color mode
 *   [ ] direct key input mode
 *   [x] color catalogue
 *   		[ ] color mode changer
 *   		[x] 4-bit code table
 *   		[x] 8-bit code table
 *   		[ ] 8-bit code RGB explicit picker
 *   		[ ] 24-bit code table
 *   		[ ] 24-bit code RGB explicit picker
 *   
 *   [x] char catalogue
 *   		[x] 16x16 grid
 *   		[x] hotkey swapper
 *   		[ ] default background changer
 * 
 *  == CONSIDER ==
 * [ ] timeBeginPeriod() to change minimum sleep time
 * [ ] threaded inputs suck?
 * [ ] 8-bit color catalogue sucks
 * [ ] in color picker, render a character onto the currently chosen color
**/


inline std::string DEBUG_STR = "";

// 0 = nothing (bad idea)
// 1 = report crashes, errors
// 2 = also report major events (export, etc.)
// 3 = you wish for too much knowledge
inline int DEBUG_REPORT_LEVEL = 2;

/*
 * [date][minor]-[program condition][dev stage]
 * 
 * date: [month][year (2 digits)]
 * 
 * minor: for multiple versions in the same month with the same everything
 * 
 * program conditions: 
 * 		N = not working / critical bug
 * 		U = under development (not ready for release)
 * 		R = release
 * 		X = experimental test build
 * 
 * development stages: 
 * 		A = alpha (many key features not implemented)
 * 		B = beta (most/all features implemented, needs refining)
 * 		F = finished (all planned features implemented and working well)
**/
inline constexpr const char* VERSION = "v07260-UA";


// if true, uses buffered terminal inputs instead of raw keyboard keystates
inline bool INPUT_SAFE_MODE = false;

// Linux path to search for input devices
inline constexpr const char* INPUT_DEVICE_SEARCH_PATH = "/proc/bus/input/devices";

// default background char
inline const std::string DEFAULT_BACK = ".";


// fps
inline float FPS = 30.0f;

// inputs per second
// should not be lower than FPS
inline float IPS = 60.0f;

inline bool USE_THREADED_INPUT = false;

// animation duration for cursor (seconds)
inline float ANIM_CURSOR = 0.5f;

// how often (in frames) to do a more expensive analysis frame
inline int ANALYSIS_FREQUENCY = 10; 


// screen width
inline int SCREEN_WIDTH = 75;
// screen height
inline int SCREEN_HEIGHT = 20;

// size of side panel
inline constexpr int PANEL_SIZE = 35;

// size of bottom panel
inline constexpr int BOTTOM_PANEL_SIZE = 2;

// colors in palette
inline constexpr int PALETTE_SIZE = 16;

inline constexpr int COLOR_CATALOGUE_4B_X =  8;
inline constexpr int COLOR_CATALOGUE_4B_Y =  2;
inline constexpr int COLOR_CATALOGUE_8B_X = 18;
inline constexpr int COLOR_CATALOGUE_8B_Y =  6;
// these are all floats and thats dumb but it makes for less casts
inline constexpr float COLOR_CATALOGUE_24B_X = 32.0f;
inline constexpr float COLOR_CATALOGUE_24B_Y =  6.0f;

// largest color catalogue Y size
inline constexpr int COLOR_CATALOGUE_LARGEST_Y = 6;


// if true, using direct keyboard inputs
inline bool DIRECT_KEY_INPUTS = false;

// if true, use HJKL instead of UHJK for arrows, vim style
//inline bool VIM_ARROWS = false;

// type of color
// 0 = none,  1 = 4-bit,  2 = 8-bit,  3 = 24-bit
inline int COLOR_MODE = 0;


#include "output.hpp"


// border color
inline std::string BORDER_COLOR = ANSI::bold;
// key highlight color
inline std::string KEY_COLOR = ANSI::bold;
// cursor color
inline std::string CURSOR_COLOR = ANSI::Color_8bit::makeColor(230, true);
inline std::string CURSOR_COLOR_BACK = ANSI::Color_8bit::makeColor(227, false);

inline std::string DISPLAY_COLOR_4BIT = ANSI::green;
inline std::string DISPLAY_COLOR_8BIT = ANSI::Color_8bit::makeColor(154);
inline std::string DISPLAY_COLOR_24BIT = ANSI::Color_24bit::makeColor(45, 214, 183);

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