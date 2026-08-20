/* 
 * Settings for the program
**/

#pragma once


/*
 * DIRECTORIES
 * 
 * /Export/Art/      --  exported .ans files
 * /Export/Palette/  --  exported .plt files
 * /Sessions/        --  session directories
 * 
 * 
 * SPECIFICATIONS
 * 
 * - .ans files can be CP437 or UTF-8, but everything internally is handled as UTF-8
 * - .plt files always store foreground colors
**/

/*
 *  == TODO ==
 * [x] extend art bounds when inserting outside of range
 * [x] clean up main.cpp
 * [x] proper windows input support
 * [x] color catalogue picker
 * [x] character catalogue picker
 * [x] custom palettes
 * [x] export art to .ans file
 * [x] session logging
 * [x] you dont need to loop through every cell on the screen
 * [x] screen scrolling
 * [x] importing art from file
 * [x] UTF-8 / CP-437 modes
 * [x] render borders last
 * [x] ASCII mode
 * [x] GPL -> AGPL
 * [x] load palettes to/from file
 * [x] trim empty spaces from art
 * [ ] optimize export codes
 * [ ] import art with cursor in the middle
 * [ ] assure proper filetypes for importing
 * [ ] proper linux input support
 * [ ] autosaves for rollbacks
 * 		[ ] automatic autosave deletion on termination
 * [ ] undo/redo
 * [ ] highlighting sections for copy/paste
 * [ ] export file location picker
 * [ ] mouse input
 * [ ] saving preferences
 * [ ] custom session IDs
 * [ ] finalize window sizes
 * [ ] better "screen too small" screen
 * [ ] support mode for only 4-bit color
 * [ ] changing to/from input safe mode
 * [ ] editor ASCII mode
 * [ ] gradient maker
 * [ ] only re-render when needed
 * [ ] music
 * [ ] creating animations
 * 		[ ] animation time delay
 * 		[ ] animations to gif
 * [ ] CHECK ALL TODO COMMENTS
 * [ ] downgrading color mode -> remove those codes from art
 * 		[ ] recognize the colors and appropriately downgrade?
 * [ ] make input checkers use separate thread (mutex?)
 * [ ] disable win terminal shift+arrow and ctrl+arrow
 * [ ] clean up files
 * 		[ ] put CellString builds into separate file
 * 		[ ] organize inputs
 * 		[ ] header comments
 * 		[ ] make separate folder for I/O files
 * [ ] tutorial
 * [ ] autosave
 * [ ] paste into popup text box
 * [ ] make popup text box cursor better
 * [ ] assure terminal scrolling is disabled
 * [ ] assure proper palette formatting
 * SIDE PANEL: 
 *   [x] sunrise text
 *   [x] char hotkeys
 *   [x] settings
 *          [x] color mode
 *          [x] encoding
 *   		[x] input safe mode
 *          [ ] pages of options
 *   		[ ] music volume
 *   		[ ] fps?
 *   		[ ] cursor animation
 *   		[ ] border color
 *   		[ ] key color
 *   [ ] song name
 *   [x] art size
 *   [x] export
 *   [x] import
 *   [ ] copy/paste
 *   [ ] animation
 *   [x] color mode
 *   [x] ASCII mode
 *   [x] color catalogue
 *   		[x] color mode changer
 *   		[x] 4-bit code table
 *   		[x] 8-bit code table
 *   		[x] 8-bit code RGB explicit picker
 *   		[x] 24-bit code table
 *   		[x] 24-bit code RGB explicit picker
 *   
 *   [x] char catalogue
 *   		[x] 16x16 grid
 *   		[x] hotkey swapper
 *   		[ ] default background changer
 * 
 *  == CONSIDER ==
 * [ ] timeBeginPeriod() to change minimum sleep time
 * [ ] threaded inputs suck?
 * [ ] change 8-bit color catalogue layout
 * [ ] in color picker, render a character onto the currently chosen color
 * [ ] more than just color codes
 * [ ] halt program when not the focused window
 * 
 *  == BUGS ==
 * [x] 'pick to palette' crashes
 * [x] Art::trim() does some weird shit
 * [ ] importing is completely broken
 * 		[ ] color codes do not cross over properly
 * 		[ ] multiple codes not recognized properly (ex. \033[1;30m)
 * [ ] alt+HJKL for moving cursor is not affected by keystate delay
**/


/*
 * CELL SYSTEM
 * 
 * Cell {
 *      string: character
 *      string: foreground color
 *      string: background color
 *      string: extra codes
 * }
 * 
 * Cells are rendered in this order: 
 *      add. + fore. + back. + char.
 * 
 * Importing yields obvious issues as codes are not always cleanly formatted in .ans. 
 * Things that must be taken into account when converting to cells: 
 *      1. Colors must bleed into succeeding cells
 *      2. Redundant calls must be accounted for
 * 
 * Example (from PabloDraw README.ans)
 *      
 *      \033[7h\033[0;1;40;30m\033[?33h-- ---------
 *      [  this section is all codes  ][ all chars ]
 *      
 *      this would be formatted into: 
 *          foreground color: \033[30m
 *          background color: \033[40m
 *          extra codes: \033[7h\033[0m\033[1m\033[?33h
 * 
 *      the formatting then bleeds into the rest of the cells. they would look like: 
 *          foreground color: \033[30m
 *          background color: \033[40m
 *          extra codes: 
 * 
 * Example
 *      
 *      \033[30m\033[0m#
 *      
 *      this would be formatted as a cell with no colors or codes. 
 *      every color code before the reset is to be ignored
 * 
 * Example
 * 
 *      \033[34m\033[35m#
 *      
 *      this would be formatted as a cell with just the \033[35m code in the foreground. 
 * 		color codes override prior color codes. 
 * 
**/



// version
inline constexpr const char* VERSION = "0.1.0";


inline std::string DEBUG_STR = "";

// 1 = report crashes, errors
// 2 = also report major events (export, etc.)
// 3 = good amount of stuff reported (very useful!)
// 4 = you wish for too much knowledge (EXTRA VERBOSE)
inline int DEBUG_REPORT_LEVEL = 4;


// if true, uses buffered terminal inputs instead of raw keyboard keystates
inline bool INPUT_SAFE_MODE = false;

// Linux path to search for input devices
inline constexpr const char* INPUT_DEVICE_SEARCH_PATH = "/proc/bus/input/devices";

// default art background char
inline std::string DEFAULT_BACK = " ";


// fps
inline float FPS = 60.0f;

// inputs per second
// should not be lower than FPS
// only used when threading input
inline float IPS = 60.0f;

inline bool USE_THREADED_INPUT = false;

// animation duration for cursor (seconds)
inline float ANIM_CURSOR = 0.5f;

// how often (in frames) to do a more expensive analysis frame
inline int ANALYSIS_FREQUENCY = 10; 

// frames in between fast keystate updates, just for arrow keys
// (these frames will hold arrow keystates at false)
inline int FAST_KEYSTATE_ARROW_DELAY = 1;


// screen width
inline int SCREEN_WIDTH = 75;
// screen height
inline int SCREEN_HEIGHT = 20;

// size of side panel
inline constexpr int PANEL_SIZE = 35;

// size of bottom panel
inline constexpr int BOTTOM_PANEL_SIZE = 2;

// colors in palette
inline constexpr size_t PALETTE_SIZE = 16;

inline constexpr int COLOR_CATALOGUE_4B_X =  8;
inline constexpr int COLOR_CATALOGUE_4B_Y =  2;
inline constexpr int COLOR_CATALOGUE_8B_X = 18;
inline constexpr int COLOR_CATALOGUE_8B_Y =  6;
// these are floats and thats dumb but it makes for less casts
inline constexpr float COLOR_CATALOGUE_24B_X = 32.0f;
inline constexpr float COLOR_CATALOGUE_24B_Y =  6.0f;

// largest color catalogue Y size
inline constexpr int COLOR_CATALOGUE_LARGEST_Y = 6;


// amount of frames to ignore input for when popup is opened
inline constexpr int POPUP_INPUT_DELAY = 10;

// width of popup (in % of screen width)
inline constexpr float POPUP_WIDTH_SCALE = .5f;
// height of popup (in % of screen height)
inline constexpr float POPUP_HEIGHT_SCALE = .333333f;

// amount of settings
inline constexpr int SETTINGS_AMOUNT = 3;


// 0 = UTF-8, 1 = CP437
inline int ART_ENCODING = false;

// if true, using direct keyboard inputs (ASCII mode)
inline bool ASCII_MODE = false;

// if true, use HJKL instead of UHJK for arrows, vim style
//inline bool VIM_ARROWS = false;

// type of color
// 0 = none,  1 = 4-bit,  2 = 8-bit,  3 = 24-bit
inline int ART_COLOR_MODE = 0;

// type of color for the UI
// 0 = none,  1 = 4-bit,  2 = 8-bit,  3 = 24-bit
inline int UI_COLOR_MODE = 3;

#include "output.hpp"


// border color
inline std::string BORDER_COLOR = ANSI::bold;
// key highlight color
inline std::string KEY_COLOR = ANSI::bold;
// cursor color
inline std::string CURSOR_COLOR = ANSI::Color_8bit::makeColor(230, true);
inline std::string CURSOR_COLOR_BACK = ANSI::Color_8bit::makeColor(227, false);
// panel header color
inline std::string PANEL_HEADER_COLOR = ANSI::bold;
// error color
inline std::string ERROR_COLOR = ANSI::red_back_bright;

// settings option color
inline std::string SETTINGS_OPTION_COLOR = ANSI::bold;
// settings on color
inline std::string SETTINGS_COLOR_ON = ANSI::green;
// settings off color
inline std::string SETTINGS_COLOR_OFF = ANSI::red;

inline std::string DISPLAY_COLOR_4BIT = ANSI::green;
inline std::string DISPLAY_COLOR_8BIT = ANSI::Color_8bit::makeColor(154);
inline std::string DISPLAY_COLOR_24BIT = ANSI::Color_24bit::makeColor(45, 214, 183);

inline std::string DISPLAY_ENCODING_UTF8 = ANSI::Color_24bit::makeColor(155, 155, 0);
inline std::string DISPLAY_ENCODING_CP437 = ANSI::Color_24bit::makeColor(0, 155, 155);


inline std::string HOTKEY_CHAR_1 = "█";
inline std::string HOTKEY_CHAR_2 = "▀";
inline std::string HOTKEY_CHAR_3 = "▄";
inline std::string HOTKEY_CHAR_4 = "▌";
inline std::string HOTKEY_CHAR_5 = "▐";
inline std::string HOTKEY_CHAR_6 = "▓";
inline std::string HOTKEY_CHAR_7 = "▒";
inline std::string HOTKEY_CHAR_8 = "░";
inline std::string HOTKEY_CHAR_9 = "■";
inline std::string HOTKEY_CHAR_0 = " ";