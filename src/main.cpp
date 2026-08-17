/* 
 * Zaine Rehman, 6/19/2026
**/

#include <iostream>
#include <csignal>
#include <thread>
#include <cstring>
#include <chrono>
#include <atomic>
#include <array>
#include <filesystem>

#include "input.hpp"
#include "output.hpp"
#include "settings.hpp"
#include "art.hpp"
#include "precalculate.hpp"
#include "lib.hpp"
#include "file_io.hpp"
#include "log.hpp"

#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <linux/input.h>
	#include <fcntl.h>
	#include <termios.h>
#endif



int main() {
	
	// -- SETUP -- 

	// make sure all directories exist
	if (!std::filesystem::is_directory("Export")) {
		std::filesystem::create_directory("Export");
		std::filesystem::create_directory("Export/Art");
		std::filesystem::create_directory("Export/Palette");
	} else {
		if (!std::filesystem::is_directory("Export/Art")) {
			std::filesystem::create_directory("Export/Art");
		}
		if (!std::filesystem::is_directory("Export/Palette")) {
			std::filesystem::create_directory("Export/Palette");
		}
	}
	if (!std::filesystem::is_directory("Sessions")) {
		std::filesystem::create_directory("Sessions");
	}

	// create session ID and session folder
	createSessionID();
	startSession();

	srand(time(0));
	std::ios_base::sync_with_stdio(false);

	std::signal(SIGINT,  signalHandler);
	std::signal(SIGTERM, signalHandler);
	std::signal(SIGABRT, signalHandler);

	KeyStates keyStates {};
	KeyStates keyStates_slow {};
	int keyChecker;

	bool SCREEN_TOO_SMALL = false;

	Art ART {1, 1, Cell{DEFAULT_BACK, "", ""}};
	Renderer render {static_cast<uint32_t>(SCREEN_WIDTH), static_cast<uint32_t>(SCREEN_HEIGHT)};
	int cursorX = SCREEN_WIDTH/2, cursorY = SCREEN_HEIGHT/2;

	// color catalogue picker
	int catalogue4bIndexX = 0, catalogue4bIndexY = 0;
	int catalogue8bIndexX = 0, catalogue8bIndexY = 0;
	int catalogue24bIndexX = 0, catalogue24bIndexY = 0;

	// char catalogue picker
	int charCatalogueIndexX = 0, charCatalogueIndexY = 0;

	// which catalogue to show
	// 0 = 4-bit, 1 = 8-bit, 2 = 24-bit
	// 3 = 8-bit picker, 4 = 24-bit picker
	int colorCatalogueType = 1;

	// catalogue color pickers
	int colorPicker8bit_r  = 0, colorPicker8bit_g  = 0, colorPicker8bit_b  = 0;
	int colorPicker24bit_r = 0, colorPicker24bit_g = 0, colorPicker24bit_b = 0;

	// showing input popup?
	// 0 = no, 1 = yes (text input), 2 = yes (confirmation)
	int popupShowing = 0;
	// input popup text (input)
	std::string inputPopupText = "";
	// input popup text (displayed)
	std::string inputPopupTextDisplayed = "";
	// frame that popup was requested
	// for a slight input delay
	int popupFrameStarted = 0;
	// what are we doing in the popup?
	// 0 = nothin/idc, 1 = changing R, 2 = changing G, 3 = changing B
	int popupFlag = 0;

	// program frame number
	int frame = 0;
	// ignore input for this many frames
	int ignoreInputFrames = 6;
	// ignore JUST fast keystate updates
	int ignoreArrowFrames = 0;

	// animation stage of the cursor
	int cursorAnim = 0;
	// 0 = no, 1 = colors, 2 = chars, 3 = exporting, 4 = importing, 5 = settings
	char sidePanelMode = 0;

	// double check to make sure art isnt accidentally reset
	bool killingArt = false;
	// double check for program exitg
	bool exitingProgram = false;

	// 0 = no fail,  1 = exporting art fail,  2 = exporting palette fail
	int showExportFail = 0;
	// 0 = exporting art,  1 = exporting foreground palette,  2 = exporting foreground palette
	int exportMode = 0;

	// path for import
	std::string importPathString = "";
	// 0 = no fail,  1 = importing art fail,  2 = importing palette fail
	int showImportFail = 0;
	// 0 = importing art,  1 = importing foreground palette,  2 = importing foreground palette
	int importMode = 0;
	
	// settings index
	// 0 = color mode
	// 1 = encoding
	int settingsIndex = 0;
	// if editing settings
	// 0 = no, 1 = left, 2 = right
	int settingsEdit = 0;
	// index of settings option to be displayed at the top of the screen
	// used for scrolling thru options
	int settingsTopIndex = 0;


	// foreground color palette
	std::string colorForePalette[PALETTE_SIZE] = {
		ANSI::red,            ANSI::green,        ANSI::blue,         ANSI::yellow, 
		ANSI::magenta,        ANSI::cyan,         ANSI::white,        ANSI::black, 
		ANSI::red_bright,     ANSI::green_bright, ANSI::blue_bright,  ANSI::yellow_bright, 
		ANSI::magenta_bright, ANSI::cyan_bright,  ANSI::white_bright, ANSI::black_bright
	};
	// background color palette
	std::string colorBackPalette[PALETTE_SIZE] = {
		ANSI::red_back,            ANSI::green_back,        ANSI::blue_back,         ANSI::yellow_back, 
		ANSI::magenta_back,        ANSI::cyan_back,         ANSI::white_back,        ANSI::black_back, 
		ANSI::red_back_bright,     ANSI::green_back_bright, ANSI::blue_back_bright,  ANSI::yellow_back_bright, 
		ANSI::magenta_back_bright, ANSI::cyan_back_bright,  ANSI::white_back_bright, ANSI::black_back_bright
	};
	size_t colorForeIndex = 0, colorBackIndex = 0;

	if (DEBUG_REPORT_LEVEL >= 2) reportLog("Initialized basic variables in main()");


	// terminal raw mode
	// TODO put this in another file
	#ifdef _WIN32
		// files are saved as UTF-8 so dont do this
		//SetConsoleCP(437);
		//SetConsoleOutputCP(437);
		SetConsoleCP(65001);       
		SetConsoleOutputCP(65001); 

		// raw output
		
		HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		
		DWORD dwOutputMode = 0;
		GetConsoleMode(hOutput, &dwOutputMode);
		dwOutputMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(hOutput, dwOutputMode);

		if (DEBUG_REPORT_LEVEL >= 2) reportLog("\tConsole output mode set");

		// raw input

		HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
		DWORD originalMode;
		GetConsoleMode(hInput, &originalMode);

		DWORD dwInputMode = 0;
		GetConsoleMode(hInput, &dwInputMode);
		dwInputMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);
		dwInputMode |= ENABLE_VIRTUAL_TERMINAL_INPUT | ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT;
		SetConsoleMode(hInput, dwInputMode);

		if (DEBUG_REPORT_LEVEL >= 2) reportLog("\tConsole input mode set");

		// hide cursor

		CONSOLE_CURSOR_INFO cursorInfo;
    
		GetConsoleCursorInfo(hOutput, &cursorInfo);
		cursorInfo.bVisible = FALSE;
		SetConsoleCursorInfo(hOutput, &cursorInfo);

		if (DEBUG_REPORT_LEVEL >= 2) reportLog("\tConsole cursor hidden");

		if (hInput == INVALID_HANDLE_VALUE || hOutput == INVALID_HANDLE_VALUE) return -1;
	#else
		system("setterm -cursor off");
	
		// set terminal to raw mode and turn off echo
		// most of these are the same as doing "raw" just without -opost because that messes shit up
		// also allowing 'isig' for ctrl+C
		system("stty -ignbrk -brkint -ignpar -inlcr -icanon -ixoff -igncr -icrnl -parmrk -inpck -istrip -ixon isig -iuclc -ixany -imaxbel -xcase min 1 time 0 -echo");

		if (DEBUG_REPORT_LEVEL >= 2) reportLog("\tRaw mode enabled");
		
		if (!INPUT_SAFE_MODE) {
			// find keyboard
		
			std::string handler = getKeyboardHandler();
			
			const std::string keyboardPath = "/dev/input/event" + handler;
			std::cout << "Keyboard path: " << keyboardPath << '\n';

			if (DEBUG_REPORT_LEVEL >= 2) reportLog("\tKeyboard handler found at: " + keyboardPath);
			
			keyChecker = open(keyboardPath.c_str(), O_RDONLY | O_NONBLOCK);
			if (keyChecker == -1) {
				reportLog("\t!!!Keyboard not found. errno=" + std::strerror(errno));
				std::cerr 
					<< "COULD NOT LOAD KEYBOARD!\nPath: '" << keyboardPath << "'" 
					<< "\nErr: " << std::strerror(errno)
					<< std::endl;
			}
		}
	#endif
	if (INPUT_SAFE_MODE) {
		std::jthread(safeModeInputHelper).detach();
		if (DEBUG_REPORT_LEVEL >= 2) reportLog("\tInput safemode thread detached");
	} else {
		if (USE_THREADED_INPUT) std::jthread(thread_doKeyStates, std::ref(keyStates), std::ref(keyStates_slow), keyChecker).detach();
		if (DEBUG_REPORT_LEVEL >= 2) reportLog("\tInput thread detached");
	}

	if (DEBUG_REPORT_LEVEL >= 2) reportLog("Set up terminal raw mode");
	

	// TODO: needed?
	//std::cout << "\x1b[>4;2m" << std::flush;
	//std::cout << "\x1b[?9001h" << std::flush;
	//system("Remove-PSReadLineKeyHandler -Chord 'Shift+LeftArrow'; Remove-PSReadLineKeyHandler -Chord 'Shift+RightArrow'");
	//std::cout << "\x1b[?1000h\x1b[?1006h" << std::flush;
	std::cout << ANSI::cursor_invisible << std::flush;

	clear();
	

	// -- PRE CALCULATED STUFF -- 

	CellString sunriseAnsi = getSunriseAnsi();

	CellString colorCatalogue_4bit = getColorCatalogue_4bit();
	CellString colorCatalogue_8bit = getColorCatalogue_8bit();
	CellString colorCatalogue_24bit = getColorCatalogue_24bit();

	CellString charCatalogue = getCharCatalogue();

	if (DEBUG_REPORT_LEVEL >= 2) reportLog("Pre-calculated items");


	// -- LOOP -- 

	// get rid o this
	// do NOT get rid o this
	// nah get rid o it
	//std::this_thread::sleep_for(std::chrono::milliseconds(100));

	if (DEBUG_REPORT_LEVEL >= 2) {
		if (RUNNING) reportLog("Entering main loop...");
		else reportLog("NOT entering main loop: RUNNING is false");
	}
	while (RUNNING) {
		std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();
		ART.changeFlag = false;

		SCREEN_TOO_SMALL = false;

		// -- INPUTS -- 

		/*
		* [←↕→] OR [HJKL]: cursor
		* holding [Alt]: fast cursor
		* holding [Ctrl]: move art
		* 
		* [0-9]: set character
		* 
		* [QE/W]: change/set background color
		* [AD/S]: change/set foreground color
		* 
		* [N]: pick color into foreground palette
		* [M]: pick color into background palette
		* 
		* [C]: clear color
		* [Bksp]: clear character
		* 
		* [}]: open color catalogue
		* 		[,]: change catalogue left
		* 		[.]: change catalogue right
		* 		[W]: apply color to foreground palette
		* 		[S]: apply color to background palette
		* 		[Q/E]: change active foreground palette color
		* 		[A/D]: change active background palette color
		* [{]: open character catalogue
		* 
		* [Del]: reset art
		* 
		* [Entr]: settings
		* 		[←→]: edit setting
		* 		[↕]: change which setting
		* 
		* [/]: export
		* 		[Space]: change mode
		* 		[Entr]: confirm
		* [\]: import
		* 		[Space]: change mode
		* 		[Z]: edit path
		* 		[Entr]: confirm
		* 
		* [Ctrl]+[S]: save art
		* [Ctrl]+[A]: toggle ASCII mode
		**/

		//setKeyStatesOff(keyStates);
		//setKeyStatesOff(keyStates_slow);
		if (!USE_THREADED_INPUT) {
			if (INPUT_SAFE_MODE) updateKeyStates_SAFE(keyStates, keyStates_slow);
			else updateKeyStates(keyStates, keyStates_slow, keyChecker);
		}

		// ignore inputs if window not in focus
		// TODO linux implementation
		#ifdef _WIN32
			if (!windowIsFocused(GetConsoleWindow())  // TODO this is broken??
		#else
			if (false 
		#endif
		|| ignoreInputFrames) {  // ignore first couple inputs, trust me ok
			setKeyStatesOff(keyStates);
			setKeyStatesOff(keyStates_slow);
			if (DEBUG_REPORT_LEVEL >= 4) reportLog("Ignoring input... " + std::to_string(ignoreInputFrames));
		}
		if (ignoreArrowFrames) {
			keyStates[Key::UP] = false;
			keyStates[Key::DOWN] = false;
			keyStates[Key::LEFT] = false;
			keyStates[Key::RIGHT] = false;
		}

		std::pair<int,int> upd = ART.toArtSpace(cursorX, cursorY);
		settingsEdit = 0;

		if (keyStates_slow[Key::ESC]) {
			if (sidePanelMode == 0) {
				if (killingArt) {
					killingArt = false;
					popupShowing = 0;
				}
				else if (!exitingProgram) {
					exitingProgram = true;
					popupShowing = 2;
					inputPopupTextDisplayed = "Exit program?";
					popupFrameStarted = frame;
				} else if (exitingProgram) {
					exitingProgram = false;
					popupShowing = 0;
				}
			} else sidePanelMode = 0;
		}

		if (keyStates[Key::CTRL]) {
			if (keyStates_slow[Key::A]) {
				// toggle ASCII mode
				ASCII_MODE = !ASCII_MODE;
				ignoreInputFrames += 6;
			}
			if (keyStates_slow[Key::S]) {
				// save art
				saveArtToSession(ART);
				ignoreInputFrames += 6;
			}
		}

		if (keyStates[Key::BACKSPACE]) {
			ART.edit(upd.first, upd.second, " ", 2);
		}

		if (keyStates[Key::ALT] ? (!ASCII_MODE && keyStates[Key::H]) || keyStates[Key::LEFT]  : (!ASCII_MODE && keyStates_slow[Key::H]) || keyStates_slow[Key::LEFT])  {
			if (sidePanelMode == 0) {
				if (keyStates[Key::CTRL]) {
					ART.x++; cursorAnim = 3; //cursorX++;
				} else {
					cursorX--; cursorAnim = 3;
				}
			} else if (sidePanelMode == 1) {
				if      (colorCatalogueType == 0) catalogue4bIndexX--;
				else if (colorCatalogueType == 1) catalogue8bIndexX--;
				else if (colorCatalogueType == 2) catalogue24bIndexX--;
			} else if (sidePanelMode == 2) charCatalogueIndexX-=2;
			else if (sidePanelMode == 5) settingsEdit = 1;
		}
		if (keyStates[Key::ALT] ? (!ASCII_MODE && keyStates[Key::L]) || keyStates[Key::RIGHT] : (!ASCII_MODE && keyStates_slow[Key::L]) || keyStates_slow[Key::RIGHT]) {
			if (sidePanelMode == 0) {
				if (keyStates[Key::CTRL]) {
					ART.x--; cursorAnim = 3; //cursorX--;
				} else {
					cursorX++; cursorAnim = 3;
				}
			} else if (sidePanelMode == 1) {
				if      (colorCatalogueType == 0) catalogue4bIndexX++;
				else if (colorCatalogueType == 1) catalogue8bIndexX++;
				else if (colorCatalogueType == 2) catalogue24bIndexX++;
			} else if (sidePanelMode == 2) charCatalogueIndexX+=2;
			else if (sidePanelMode == 5) settingsEdit = 2;
		}
		if (keyStates[Key::ALT] ? (!ASCII_MODE && keyStates[Key::J]) || keyStates[Key::UP]    : (!ASCII_MODE && keyStates_slow[Key::J]) || keyStates_slow[Key::UP])    {
			if (sidePanelMode == 0) {
				if (keyStates[Key::CTRL]) {
					ART.y++; cursorAnim = 3; //cursorY++;
				} else {
					cursorY--; cursorAnim = 3;
				}
			} else if (sidePanelMode == 1) {
				if      (colorCatalogueType == 0) catalogue4bIndexY--;
				else if (colorCatalogueType == 1) catalogue8bIndexY--;
				else if (colorCatalogueType == 2) catalogue24bIndexY--;
			} else if (sidePanelMode == 2) charCatalogueIndexY--;
			else if (sidePanelMode == 5) settingsIndex--;
		}
		if (keyStates[Key::ALT] ? (!ASCII_MODE && keyStates[Key::K]) || keyStates[Key::DOWN]  : (!ASCII_MODE && keyStates_slow[Key::K]) || keyStates_slow[Key::DOWN])  {
			if (sidePanelMode == 0) {
				if (keyStates[Key::CTRL]) {
					ART.y--; cursorAnim = 3; //cursorY--;
				} else {
					cursorY++; cursorAnim = 3;
				}
			} else if (sidePanelMode == 1) {
				if      (colorCatalogueType == 0) catalogue4bIndexY++;
				else if (colorCatalogueType == 1) catalogue8bIndexY++;
				else if (colorCatalogueType == 2) catalogue24bIndexY++;
			} else if (sidePanelMode == 2) charCatalogueIndexY++;
			else if (sidePanelMode == 5) settingsIndex++;
		}

		// dont check for these inputs in ASCII mode
		if (!ASCII_MODE && !ignoreInputFrames) {
			if (keyStates[Key::_1] || keyStates[Key::KP_1]) {
				if (sidePanelMode == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_1, 2);
				else if (sidePanelMode == 2) HOTKEY_CHAR_1 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
			}
			if (keyStates[Key::_2] || keyStates[Key::KP_2]) {
				if (sidePanelMode == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_2, 2);
				else if (sidePanelMode == 2) HOTKEY_CHAR_2 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
			}
			if (keyStates[Key::_3] || keyStates[Key::KP_3]) {
				if (sidePanelMode == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_3, 2);
				else if (sidePanelMode == 2) HOTKEY_CHAR_3 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
			}
			if (keyStates[Key::_4] || keyStates[Key::KP_4]) {
				if (sidePanelMode == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_4, 2);
				else if (sidePanelMode == 2) HOTKEY_CHAR_4 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
			}
			if (keyStates[Key::_5] || keyStates[Key::KP_5]) {
				if (sidePanelMode == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_5, 2);
				else if (sidePanelMode == 2) HOTKEY_CHAR_5 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
			}
			if (keyStates[Key::_6] || keyStates[Key::KP_6]) {
				if (sidePanelMode == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_6, 2);
				else if (sidePanelMode == 2) HOTKEY_CHAR_6 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
			}
			if (keyStates[Key::_7] || keyStates[Key::KP_7]) {
				if (sidePanelMode == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_7, 2);
				else if (sidePanelMode == 2) HOTKEY_CHAR_7 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
			}
			if (keyStates[Key::_8] || keyStates[Key::KP_8]) {
				if (sidePanelMode == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_8, 2);
				else if (sidePanelMode == 2) HOTKEY_CHAR_8 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
			}
			if (keyStates[Key::_9] || keyStates[Key::KP_9]) {
				if (sidePanelMode == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_9, 2);
				else if (sidePanelMode == 2) HOTKEY_CHAR_9 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
			}
			if (keyStates[Key::_0] || keyStates[Key::KP_0]) {
				if (sidePanelMode == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_0, 2);
				else if (sidePanelMode == 2) HOTKEY_CHAR_0 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
			}
																					// TODO this is sloppy
			if (keyStates_slow[Key::Q] && popupShowing == 0) { colorForeIndex--; if (colorForeIndex > 500) colorForeIndex = PALETTE_SIZE-1; }
			if (keyStates_slow[Key::E] && popupShowing == 0) { colorForeIndex++; if (colorForeIndex > PALETTE_SIZE-1) colorForeIndex = 0; }
			if (keyStates[Key::W] && popupShowing == 0) {
				if (sidePanelMode == 0) {
					ART.edit(upd.first, upd.second, colorForePalette[colorForeIndex], 0);
					cursorAnim = 1;
				} else if (sidePanelMode == 1) {
					if (colorCatalogueType == 0) {  // 4-bit catalogue
						colorForePalette[colorForeIndex] = ANSI::invertColor(colorCatalogue_4bit[catalogue4bIndexY*COLOR_CATALOGUE_4B_X + catalogue4bIndexX].color_back);
					} else if (colorCatalogueType == 1) {  // 8-bit catalogue
						colorForePalette[colorForeIndex] = ANSI::invertColor(colorCatalogue_8bit[catalogue8bIndexY*COLOR_CATALOGUE_8B_X + catalogue8bIndexX].color_back);
					} else if (colorCatalogueType == 2) {  // 24-bit catalogue
						colorForePalette[colorForeIndex] = ANSI::invertColor(colorCatalogue_24bit[catalogue24bIndexY*COLOR_CATALOGUE_24B_X + catalogue24bIndexX].color_back);
					} else if (colorCatalogueType == 3) {  // 8-bit picker
						colorForePalette[colorForeIndex] = ANSI::Color_8bit::makeColor(colorPicker8bit_r, colorPicker8bit_g, colorPicker8bit_b, false);
					} else if (colorCatalogueType == 4) {  // 24-bit picker
						colorForePalette[colorForeIndex] = ANSI::Color_24bit::makeColor(colorPicker24bit_r, colorPicker24bit_g, colorPicker24bit_b, false);
					}
				}
			}
																					// TODO this is sloppy
			if (keyStates_slow[Key::A] && popupShowing == 0) { colorBackIndex--; if (colorBackIndex > 500) colorBackIndex = PALETTE_SIZE-1; }
			if (keyStates_slow[Key::D] && popupShowing == 0) { colorBackIndex++; if (colorBackIndex > PALETTE_SIZE-1) colorBackIndex = 0; }
			if (keyStates[Key::S] && popupShowing == 0) {
				if (sidePanelMode == 0) {
					ART.edit(upd.first, upd.second, colorBackPalette[colorBackIndex], 1);
					cursorAnim = 1;
				} else if (sidePanelMode == 1) {
					if (colorCatalogueType == 0) {  // 4-bit catalogue
						colorBackPalette[colorBackIndex] = colorCatalogue_4bit[catalogue4bIndexY*COLOR_CATALOGUE_4B_X + catalogue4bIndexX].color_back;
					} else if (colorCatalogueType == 1) {  // 8-bit catalogue
						colorBackPalette[colorBackIndex] = colorCatalogue_8bit[catalogue8bIndexY*COLOR_CATALOGUE_8B_X + catalogue8bIndexX].color_back;
					} else if (colorCatalogueType == 2) {  // 24-bit catalogue
						colorBackPalette[colorBackIndex] = colorCatalogue_24bit[catalogue24bIndexY*COLOR_CATALOGUE_24B_X + catalogue24bIndexX].color_back;
					} else if (colorCatalogueType == 3) {  // 8-bit picker
						colorBackPalette[colorBackIndex] = ANSI::Color_8bit::makeColor(colorPicker8bit_r, colorPicker8bit_g, colorPicker8bit_b, true);
					} else if (colorCatalogueType == 4) {  // 24-bit picker
						colorBackPalette[colorBackIndex] = ANSI::Color_24bit::makeColor(colorPicker24bit_r, colorPicker24bit_g, colorPicker24bit_b, true);
					}
				}
			}

			if (keyStates[Key::C] && popupShowing == 0) {
				ART.edit(upd.first, upd.second, "", 0);
				ART.edit(upd.first, upd.second, "", 1);
				cursorAnim = 1;
			}

			if (keyStates_slow[Key::LBRACKET]) {
				if (sidePanelMode == 1) sidePanelMode = 0;
				else sidePanelMode = 1;
			}
			if (keyStates_slow[Key::RBRACKET]) {
				if (sidePanelMode == 2) sidePanelMode = 0;
				else sidePanelMode = 2;
			}

			if (keyStates_slow[Key::COMMA]) {
				if (sidePanelMode == 1) {
					colorCatalogueType--;
				}
			}
			if (keyStates_slow[Key::PERIOD]) {
				if (sidePanelMode == 1) {
					colorCatalogueType++;
				}
			}

			if (keyStates_slow[Key::SLASH] && popupShowing == 0) {
				// import
				if (sidePanelMode == 3) sidePanelMode = 0;
				else sidePanelMode = 3;
			}

			if (keyStates_slow[Key::Z] && popupShowing == 0) {
				if (sidePanelMode == 4) {
					popupShowing = 1;
					inputPopupTextDisplayed = "Enter file path for import";
					popupFrameStarted = frame;
				}
			}

			if (keyStates_slow[Key::BSLASH]) {
				// export
				if (sidePanelMode == 4) sidePanelMode = 0;
				else sidePanelMode = 4;
			}

			if (keyStates_slow[Key::R]) {
				if (sidePanelMode == 1 && colorCatalogueType == 3) {
					popupShowing = 1;
					inputPopupTextDisplayed = "Enter RED value (0-5)";
					popupFrameStarted = frame;
					popupFlag = 1;
				} else if (sidePanelMode == 1 && colorCatalogueType == 4) {
					popupShowing = 1;
					inputPopupTextDisplayed = "Enter RED value (0-255)";
					popupFrameStarted = frame;
					popupFlag = 1;
				}
			}
			if (keyStates_slow[Key::G]) {
				if (sidePanelMode == 1 && colorCatalogueType == 3) {
					popupShowing = 1;
					inputPopupTextDisplayed = "Enter GREEN value (0-5)";
					popupFrameStarted = frame;
					popupFlag = 2;
				} else if (sidePanelMode == 1 && colorCatalogueType == 4) {
					popupShowing = 1;
					inputPopupTextDisplayed = "Enter GREEN value (0-255)";
					popupFrameStarted = frame;
					popupFlag = 2;
				}
			}
			if (keyStates_slow[Key::B]) {
				if (sidePanelMode == 1 && colorCatalogueType == 3) {
					popupShowing = 1;
					inputPopupTextDisplayed = "Enter BLUE value (0-5)";
					popupFrameStarted = frame;
					popupFlag = 3;
				} else if (sidePanelMode == 1 && colorCatalogueType == 4) {
					popupShowing = 1;
					inputPopupTextDisplayed = "Enter BLUE value (0-255)";
					popupFrameStarted = frame;
					popupFlag = 3;
				}
			}
			if (keyStates_slow[Key::N]) {
				if (sidePanelMode == 0) {
					if (DEBUG_REPORT_LEVEL >= 2) reportLog("Picking color: " + ART.get(upd.first, upd.second).color_fore);
					colorForePalette[colorForeIndex] = ART.get(upd.first, upd.second).color_fore;
				}
			}
			if (keyStates_slow[Key::M]) {
				if (sidePanelMode == 0) {
					if (DEBUG_REPORT_LEVEL >= 2) reportLog("Picking color: " + ART.get(upd.first, upd.second).color_back);
					colorBackPalette[colorBackIndex] = ART.get(upd.first, upd.second).color_back;
				}
			}
			if (keyStates_slow[Key::SPACE]) {
				if (sidePanelMode == 3 && popupShowing == 0) {
					// change export mode
					exportMode++;
					clamp_rollover(exportMode, 0, 2);
				} else if (sidePanelMode == 4 && popupShowing == 0) {
					// change import mode
					importMode++;
					clamp_rollover(importMode, 0, 2);
				}
			}
		}  // ASCII mode check

		if (keyStates_slow[Key::DEL] && popupShowing == 0) {
			if (!killingArt) {
				killingArt = true;
				popupShowing = 2;
				inputPopupTextDisplayed = "Reset art?";
				popupFrameStarted = frame;
			}
		}

		if (keyStates_slow[Key::ENTER] || keyStates_slow[Key::KP_ENTER]) {
			// i dont even remember why i put this directive here
			// ah nvm i do
			//#ifndef NDEBUG
			//	if (frame > 10) {
			//#endif
			if (sidePanelMode == 0) {
				if (popupShowing) {
					if (exitingProgram) {
						// confirm program death
						RUNNING = false;
						if (DEBUG_REPORT_LEVEL >= 2) reportLog("Exit confirmed, saving and terminating...");
						saveArtToSession(ART);
					} else if (killingArt) {
						// TODO center of screen instead?
						ART.reset(cursorX, cursorY);
						popupShowing = 0;
						killingArt = false;
					}
				} else {
					// settings
					sidePanelMode = 5;
				}
			} else if (sidePanelMode == 1) {
				bool failed = false;
				// color catalogue picker
				int inputVal = 0;

				try {
					inputVal = std::stoi(inputPopupText);
					reportLog("CHECK: \"" + inputPopupText + "\" -> " + std::to_string(inputVal));
				} catch(...) {
					if (DEBUG_REPORT_LEVEL >= 2) reportLog("!!! Failed to convert user input to int: \"" + inputPopupText + "\"");
					failed = true;
				}

				if (colorCatalogueType == 3) {
					// 8-bit
					if (!failed) {
						clamp(inputVal, 0, 5);
						if (DEBUG_REPORT_LEVEL >= 2) reportLog("User value for 8-bit color: \"" + inputPopupText + "\" -> " + std::to_string(inputVal));
						switch (popupFlag) {
							case 1: colorPicker8bit_r = inputVal; break;
							case 2: colorPicker8bit_g = inputVal; break;
							case 3: colorPicker8bit_b = inputVal; break;
						}
					}
				} else if (colorCatalogueType == 4) {
					// 24-bit
					if (!failed) {
						clamp(inputVal, 0, 255);
						if (DEBUG_REPORT_LEVEL >= 2) reportLog("User value for 24-bit color: \"" + inputPopupText + "\" -> " + std::to_string(inputVal));
						switch (popupFlag) {
							case 1: colorPicker24bit_r = inputVal; break;
							case 2: colorPicker24bit_g = inputVal; break;
							case 3: colorPicker24bit_b = inputVal; break;
						}
					}
				}
				if (!failed) {
					popupShowing = 0;
					inputPopupText = "";
				}
			} else if (sidePanelMode == 3) {
				if (exportMode == 0) {
					// export art
					std::string filename = "Export/Art/Exported_Art_" + getTimestamp() + ".ans";
					
					if (!loadArtIntoFile(ART, filename)) {
						showExportFail = 1;
						//if (DEBUG_REPORT_LEVEL >= 1) reportLog("!!! Failure to export file: " + filename);
					} else {
						sidePanelMode = 0;
					}
				} else if (exportMode == 1 || exportMode == 2) {
					// export palette
					std::string filename = "Export/Palette/Exported_Palette_" + getTimestamp() + ".plt";

					if (!loadPaletteIntoFile(exportMode == 1 ? colorForePalette : colorBackPalette, filename, exportMode == 2)) {
						showExportFail = 2;
					} else {
						sidePanelMode = 0;
					}
				}
			} else if (sidePanelMode == 4) {
				if (popupShowing) {
					popupShowing = 0;
					importPathString = inputPopupText;
				} else {
					if (importMode == 0) {
						if (!loadArtFromFile(importPathString, ART)) {
							showImportFail = 1;
						} else {
							sidePanelMode = 0;
						}
					} else if (importMode == 1 || importMode == 2) {
						if (!loadPaletteFromFile(importPathString, importMode == 1 ? colorForePalette : colorBackPalette, importMode == 2)) {
							showImportFail = 2;
						} else {
							sidePanelMode = 0;
						}
					}
				}
			} else if (sidePanelMode == 5) sidePanelMode = 0;
			//#ifndef NDEBUG
			//	}
			//#endif
		}

		if (keyStates_slow[Key::INSERT]) {
			ART.trim();
		}

		//if (keyStates_slow[Key::HOME]) {
		//	// save art
		//	saveArtToSession(ART);
		//}

		//if (keyStates[Key::H]) { ART.resize(1, 0, 0, 0); }
		//if (keyStates[Key::J]) { ART.resize(0, 1, 0, 0); }
		//if (keyStates[Key::K]) { ART.resize(0, 0, 1, 0); }
		//if (keyStates[Key::L]) { ART.resize(0, 0, 0, 1); }


		// -- CLAMP VALUES -- 

		clamp(cursorX, 0, SCREEN_WIDTH-1);
		clamp(cursorY, 0, SCREEN_HEIGHT-1);

		clamp(colorCatalogueType, 0, 4);

		clamp(catalogue4bIndexX, 0, static_cast<int>(COLOR_CATALOGUE_4B_X)-1);
		clamp(catalogue4bIndexY, 0, static_cast<int>(COLOR_CATALOGUE_4B_Y)-1);
		clamp(catalogue8bIndexX, 0, static_cast<int>(COLOR_CATALOGUE_8B_X)-1);
		clamp(catalogue8bIndexY, 0, static_cast<int>(COLOR_CATALOGUE_8B_Y)-1);
		clamp(catalogue24bIndexX, 0, static_cast<int>(COLOR_CATALOGUE_24B_X)-1);
		clamp(catalogue24bIndexY, 0, static_cast<int>(COLOR_CATALOGUE_24B_Y)-1);

		clamp(charCatalogueIndexY, 0, 16-1);
		clamp(charCatalogueIndexX, 1, 32-1);

		clamp_rollover(settingsIndex, 0, SETTINGS_AMOUNT-1);


		// -- EVALUATE SOME INPUTS --

		if (ASCII_MODE && !ignoreInputFrames) {
			std::string asciiString = "";
			pollInputsIntoString(keyStates, keyStates_slow, asciiString);

			if (asciiString.size()) {
				asciiString = asciiString[0];  // first index in case of multiple inputs
				// set cell to character
				ART.edit(upd.first, upd.second, asciiString, 2);
				cursorAnim = 1;
			}
		}

		// screen scrolling
		if (cursorX < 1) {
			ART.x += SCREEN_WIDTH - PANEL_SIZE-2;
			cursorX = SCREEN_WIDTH - PANEL_SIZE-2;
		}
		else if (cursorX >= SCREEN_WIDTH - PANEL_SIZE-1) {
			ART.x -= SCREEN_WIDTH - PANEL_SIZE-2;
			cursorX = 1;
		}
		if (cursorY < 1) {
			ART.y += SCREEN_HEIGHT - BOTTOM_PANEL_SIZE-2;
			cursorY = SCREEN_HEIGHT - BOTTOM_PANEL_SIZE-2;
		}
		else if (cursorY >= SCREEN_HEIGHT - BOTTOM_PANEL_SIZE-1) {
			ART.y -= SCREEN_HEIGHT - BOTTOM_PANEL_SIZE-2;
			cursorY = 1;
		}

		// editing settings
		if (settingsEdit > 0) {
			switch (settingsIndex) {
				case 0: {  // color mode
					int oldMode = ART_COLOR_MODE;
					if (settingsEdit == 1) ART_COLOR_MODE--;
					else ART_COLOR_MODE++;

					clamp_rollover(ART_COLOR_MODE, 0, 3);

					for (size_t c = 0; c < ART.map.size(); ++c) {
						ART.map[c] = clampColor(ART.map[c], ART_COLOR_MODE);
					}

					reportLog("Color mode changed: " + std::to_string(oldMode) + " -> " + std::to_string(ART_COLOR_MODE));

					break;
				}
				case 1: {  // encoding
					int oldEncoding = ART_ENCODING;
					if (settingsEdit == 1) ART_ENCODING--;
					else ART_ENCODING++;

					clamp_rollover(ART_ENCODING, 0, 1);
					
					reportLog("Encoding changed: " + std::to_string(oldEncoding) + " -> " + std::to_string(ART_ENCODING));

					break;
				}
				case 2: {  // input safe mode
					INPUT_SAFE_MODE = !INPUT_SAFE_MODE;
					
					reportLog("Safemode toggled: " + std::string(INPUT_SAFE_MODE ? "ON" : "OFF"));

					break;
				}
			}
		}


		// -- RENDERING --

		// analysis frame
		if (!(frame % ANALYSIS_FREQUENCY)) {
			// find proper color mode
			int foundColorMode = findHighestColorCode(CellString{ART.map});
			
			if (foundColorMode != ART_COLOR_MODE) {
				reportLog("Analysis - color mode change: " + std::to_string(ART_COLOR_MODE) + " -> " + std::to_string(foundColorMode));
			}
			ART_COLOR_MODE = foundColorMode;
		}
		// if art changed, trim
		if (ART.changeFlag) {
			ART.trim();
		}

		render.clear();

		//DEBUG_STR = std::to_string(ART.x);
		//DEBUG_STR += ",";
		//DEBUG_STR += std::to_string(ART.y);

		std::pair<int,int> check = getTerminalDimensions();
		if (SCREEN_WIDTH != check.first || SCREEN_HEIGHT != check.second) {
			if (DEBUG_REPORT_LEVEL >= 3) reportLog(
				"Screen size changed from " + 
				std::to_string(SCREEN_WIDTH)+"x"+std::to_string(SCREEN_HEIGHT) + 
				" to " + 
				std::to_string(check.first)+"x"+std::to_string(check.second)
			);

			// minuz 1  << OLD COMMENT, NOT ANYMORE!!!
			SCREEN_WIDTH = check.first;
			SCREEN_HEIGHT = check.second;
			render.resize(SCREEN_WIDTH, SCREEN_HEIGHT);

			// put art in center if first time
			if (frame < 4) {  // no, this isnt good
				ART.x = SCREEN_WIDTH/2 - ART.width/2 - PANEL_SIZE/2;
				ART.y = SCREEN_HEIGHT/2 - ART.height/2 - BOTTOM_PANEL_SIZE;
				// cursor as well, only if program just opened
				if (!frame) cursorX = ART.x + ART.width/2;
				if (!frame) cursorY = ART.y + ART.height/2;
			}

			render.clear();
		}

				
		// -- SIDE PANEL --

		if (SCREEN_HEIGHT < 6) {
			sidePanelMode = -1;
			SCREEN_TOO_SMALL = true;
		}
		int thisX = SCREEN_WIDTH-1 - PANEL_SIZE + 2;
		
		if (sidePanelMode == 0) {  // standard panel
			int yLevel = 1;

			// sunrise text
			render.put(thisX, yLevel, sunriseAnsi);
			yLevel += 3;


			// this looks like shit kinda
			CellString text {" [1][2][3][4][5][6][7][8][9][0]", KEY_COLOR};
			render.put(thisX, yLevel, text);
			yLevel++;

			// char hotkeys
			text.clear(); text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_1, KEY_COLOR, ""}; text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_2, KEY_COLOR, ""}; text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_3, KEY_COLOR, ""}; text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_4, KEY_COLOR, ""}; text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_5, KEY_COLOR, ""}; text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_6, KEY_COLOR, ""}; text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_7, KEY_COLOR, ""}; text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_8, KEY_COLOR, ""}; text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_9, KEY_COLOR, ""}; text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_0, KEY_COLOR, ""}; text += " ";
			render.put(thisX, yLevel, text);
			yLevel += 2;


			// color mode
			text.clear(); text += "Color mode: ";
				 if (ART_COLOR_MODE == 0) text += "NONE";
			else if (ART_COLOR_MODE == 1) text += CellString{"4-BIT", DISPLAY_COLOR_4BIT, ""};
			else if (ART_COLOR_MODE == 2) text += CellString{"8-BIT", DISPLAY_COLOR_8BIT, ""};
			else if (ART_COLOR_MODE == 3) text += CellString{"24-BIT", DISPLAY_COLOR_24BIT, ""};
			render.put(thisX, yLevel, text);
			yLevel++;

			// encoding
			text.clear(); text += "Encoding: ";
				 if (ART_ENCODING == 0) text += CellString{"UTF-8", DISPLAY_ENCODING_UTF8};
			else if (ART_ENCODING == 1) text += CellString{"CP437", DISPLAY_ENCODING_CP437};
			render.put(thisX, yLevel, text);
			yLevel++;

			// art dimensions
			text.clear(); text += "Dimensions: ";
			text += std::to_string(ART.width);
			text += "x";
			text += std::to_string(ART.height);
			render.put(thisX, yLevel, text);
			yLevel += 2;


			text.clear(); text += "ASCII mode: ";
			text += CellString{ASCII_MODE ? "ON" : "OFF", ASCII_MODE ? SETTINGS_COLOR_ON : SETTINGS_COLOR_OFF};
			render.put(thisX, yLevel, text);
			yLevel += 2;


			// enter to export
			text.clear();
			text += CellString{"[Entr]", KEY_COLOR};
			text += " for settings";
			render.put(thisX, yLevel, text);
			yLevel++;

			// slash to import
			text.clear();
			text += CellString{"[/]", KEY_COLOR};
			text += " to export";
			render.put(thisX, yLevel, text);
			yLevel++;

			// backslash for settings
			text.clear();
			text += CellString{"[\\]", KEY_COLOR};
			text += " to import";
			render.put(thisX, yLevel, text);
			yLevel += 2;


			// backspace to reset
			text.clear();
			text += CellString{"[Bksp]", KEY_COLOR};
			text += " to clear art";
			render.put(thisX, yLevel, text);
		} 
		else if (sidePanelMode == 1) {  // color catalogue
			#define colorCatalogueLineNo 6
			int catIndexX, catIndexY;
			float catSizeX, catSizeY;
			CellString catStr;
			CellString catName {"("};

			if (colorCatalogueType == 0) {  // 4-bit
				catIndexX = catalogue4bIndexX;
				catIndexY = catalogue4bIndexY;
				catSizeX = COLOR_CATALOGUE_4B_X;
				catSizeY = COLOR_CATALOGUE_4B_Y;
				catStr = colorCatalogue_4bit;
				catName += CellString{"4-BIT", DISPLAY_COLOR_4BIT};
			} else if (colorCatalogueType == 1) {  // 8-bit
				catIndexX = catalogue8bIndexX;
				catIndexY = catalogue8bIndexY;
				catSizeX = COLOR_CATALOGUE_8B_X;
				catSizeY = COLOR_CATALOGUE_8B_Y;
				catStr = colorCatalogue_8bit;
				catName += CellString{"8-BIT", DISPLAY_COLOR_8BIT};
			} else if (colorCatalogueType == 2) {  // 24-bit
				catIndexX = catalogue24bIndexX;
				catIndexY = catalogue24bIndexY;
				catSizeX = COLOR_CATALOGUE_24B_X;
				catSizeY = COLOR_CATALOGUE_24B_Y;
				catStr = colorCatalogue_24bit;
				catName += CellString{"24-BIT", DISPLAY_COLOR_24BIT};
			} else if (colorCatalogueType == 3) {  // 8-bit picker
				catIndexX = -1; catIndexY = -1;
				catSizeX = -1; catSizeY = -1;
				catStr = {};
				catName += CellString{"8-BIT PICK", DISPLAY_COLOR_8BIT};
			} else if (colorCatalogueType == 4) {  // 24-bit picker
				catIndexX = -1; catIndexY = -1;
				catSizeX = -1; catSizeY = -1;
				catStr = {};
				catName += CellString{"24-BIT PICK", DISPLAY_COLOR_24BIT};
			}
			catName += ")";

			CellString text {};

			render.put(thisX, 1, CellString{"== COLOR CATALOGUE ==", PANEL_HEADER_COLOR});

			text += CellString{"[,]", KEY_COLOR, ""};
			text += " ";
			text += CellString{"[.]", KEY_COLOR, ""};
			text += " to change";

			// current catalog type
			render.put(thisX, colorCatalogueLineNo-2, catName);
			render.put(thisX+15, colorCatalogueLineNo-2, text);

			// for catalogue rendering
			if (catIndexX != -1) {
				//if (SCREEN_HEIGHT > colorCatalogueLineNo + COLOR_CATALOGUE_LARGEST_Y + 2) {}

				// X arrows
				render.put(thisX + catIndexX, colorCatalogueLineNo-1, Cell{"▼", ANSI::bold, ""});  // TODO make settings color
				render.put(thisX + catIndexX, colorCatalogueLineNo+catSizeY, Cell{"▲", ANSI::bold, ""});
				// Y arrows
				render.put(thisX-1, colorCatalogueLineNo + catIndexY, Cell{"►", ANSI::bold, ""});
				render.put(thisX+catSizeX, colorCatalogueLineNo + catIndexY, Cell{"◄", ANSI::bold, ""});

				// render catalogue
				for (int yc = 0; yc < catSizeY; ++yc) {
					for (int xc = 0; xc < catSizeX; ++xc) {
						render.put(xc+thisX, yc+colorCatalogueLineNo, catStr[yc*catSizeX + xc]);
					}
				}

				// current color
				text.clear(); text += "Current color: ";
				text += CellString{"   ", "", catStr[catIndexY*catSizeX + catIndexX].color_back};
				render.put(thisX, colorCatalogueLineNo + COLOR_CATALOGUE_LARGEST_Y + 2, text);
			} else {
				// picker rendering
				/*
				 *  [R] Red   [G] Green   [B] Blue
				 *   rrrrr      ggggg       bbbbb
				 *   r---r      g---g       b---b
				 *   rrrrr      ggggg       bbbbb
				 * 
				 * Value range: 0-5 OR 0-255
				**/
				text.clear(); text += " [R] Red   [G] Green   [B] Blue";
				render.put(thisX, colorCatalogueLineNo+1, text);

				if (colorCatalogueType == 3) {  // 8-bit picker
					// box with value in it

					text.clear(); text += "  ";
					text += CellString{"     ", "", ANSI::Color_8bit::makeColor(colorPicker8bit_r, 0, 0, true)};
					text += "      ";
					text += CellString{"     ", "", ANSI::Color_8bit::makeColor(0, colorPicker8bit_g, 0, true)};
					text += "       ";
					text += CellString{"     ", "", ANSI::Color_8bit::makeColor(0, 0, colorPicker8bit_b, true)};
					render.put(thisX, colorCatalogueLineNo+3, text);

					text.clear(); text += "  ";
					text += CellString{" ", "", ANSI::Color_8bit::makeColor(colorPicker8bit_r, 0, 0, true)};
					std::string num = std::to_string(colorPicker8bit_r); num.resize(3, ' ');
					text += CellString{num, "", ANSI::Color_8bit::makeColor(colorPicker8bit_r, 0, 0, true)};
					text += CellString{" ", "", ANSI::Color_8bit::makeColor(colorPicker8bit_r, 0, 0, true)};
					text += "      ";
					text += CellString{" ", "", ANSI::Color_8bit::makeColor(0, colorPicker8bit_g, 0, true)};
					num = std::to_string(colorPicker8bit_g); num.resize(3, ' ');
					text += CellString{num, "", ANSI::Color_8bit::makeColor(0, colorPicker8bit_g, 0, true)};
					text += CellString{" ", "", ANSI::Color_8bit::makeColor(0, colorPicker8bit_g, 0, true)};
					text += "       ";
					text += CellString{" ", "", ANSI::Color_8bit::makeColor(0, 0, colorPicker8bit_b, true)};
					num = std::to_string(colorPicker8bit_b); num.resize(3, ' ');
					text += CellString{num, "", ANSI::Color_8bit::makeColor(0, 0, colorPicker8bit_b, true)};
					text += CellString{" ", "", ANSI::Color_8bit::makeColor(0, 0, colorPicker8bit_b, true)};
					render.put(thisX, colorCatalogueLineNo+4, text);

					text.clear(); text += "  ";
					text += CellString{"     ", "", ANSI::Color_8bit::makeColor(colorPicker8bit_r, 0, 0, true)};
					text += "      ";
					text += CellString{"     ", "", ANSI::Color_8bit::makeColor(0, colorPicker8bit_g, 0, true)};
					text += "       ";
					text += CellString{"     ", "", ANSI::Color_8bit::makeColor(0, 0, colorPicker8bit_b, true)};
					render.put(thisX, colorCatalogueLineNo+5, text);

					CellString colorBlock {"     ", "", ANSI::Color_8bit::makeColor(colorPicker8bit_r, colorPicker8bit_g, colorPicker8bit_b, true)};
					render.put(thisX, colorCatalogueLineNo+7, CellString{"Current color:  "} + colorBlock);
					render.put(thisX, colorCatalogueLineNo+8, CellString{"                "} + colorBlock);
				} else {  // 24-bit picker
					// box with value in it
					
					text.clear(); text += "  ";
					text += CellString{"     ", "", ANSI::Color_24bit::makeColor(colorPicker24bit_r, 0, 0, true)};
					text += "      ";
					text += CellString{"     ", "", ANSI::Color_24bit::makeColor(0, colorPicker24bit_g, 0, true)};
					text += "       ";
					text += CellString{"     ", "", ANSI::Color_24bit::makeColor(0, 0, colorPicker24bit_b, true)};
					render.put(thisX, colorCatalogueLineNo+3, text);

					text.clear(); text += "  ";
					text += CellString{" ", "", ANSI::Color_24bit::makeColor(colorPicker24bit_r, 0, 0, true)};
					std::string num = std::to_string(colorPicker24bit_r); num.resize(3, ' ');
					text += CellString{num, "", ANSI::Color_24bit::makeColor(colorPicker24bit_r, 0, 0, true)};
					text += CellString{" ", "", ANSI::Color_24bit::makeColor(colorPicker24bit_r, 0, 0, true)};
					text += "      ";
					text += CellString{" ", "", ANSI::Color_24bit::makeColor(0, colorPicker24bit_g, 0, true)};
					num = std::to_string(colorPicker24bit_g); num.resize(3, ' ');
					text += CellString{num, "", ANSI::Color_24bit::makeColor(0, colorPicker24bit_g, 0, true)};
					text += CellString{" ", "", ANSI::Color_24bit::makeColor(0, colorPicker24bit_g, 0, true)};
					text += "       ";
					text += CellString{" ", "", ANSI::Color_24bit::makeColor(0, 0, colorPicker24bit_b, true)};
					num = std::to_string(colorPicker24bit_b); num.resize(3, ' ');
					text += CellString{num, "", ANSI::Color_24bit::makeColor(0, 0, colorPicker24bit_b, true)};
					text += CellString{" ", "", ANSI::Color_24bit::makeColor(0, 0, colorPicker24bit_b, true)};
					render.put(thisX, colorCatalogueLineNo+4, text);

					text.clear(); text += "  ";
					text += CellString{"     ", "", ANSI::Color_24bit::makeColor(colorPicker24bit_r, 0, 0, true)};
					text += "      ";
					text += CellString{"     ", "", ANSI::Color_24bit::makeColor(0, colorPicker24bit_g, 0, true)};
					text += "       ";
					text += CellString{"     ", "", ANSI::Color_24bit::makeColor(0, 0, colorPicker24bit_b, true)};
					render.put(thisX, colorCatalogueLineNo+5, text);

					CellString colorBlock {"     ", "", ANSI::Color_24bit::makeColor(colorPicker24bit_r, colorPicker24bit_g, colorPicker24bit_b, true)};
					render.put(thisX, colorCatalogueLineNo+7, CellString{"Current color: "} + colorBlock);
					render.put(thisX, colorCatalogueLineNo+8, CellString{"               "} + colorBlock);
				}
			}

			// to select
			text.clear();
			text += CellString{"[W]", KEY_COLOR};
			text += " / ";
			text += CellString{"[S]", KEY_COLOR};
			text += " to apply to palette";
			render.put(thisX, colorCatalogueLineNo + COLOR_CATALOGUE_LARGEST_Y + 5, text);
		} else if (sidePanelMode == 2) {  // characters
			#define charCatalogueLineNo 7

			CellString text {};

			render.put(thisX, 1, CellString{"== CHAR CATALOGUE ==", PANEL_HEADER_COLOR});

			// show hotkeys
			text.clear(); text += CellString{" [1][2][3][4][5][6][7][8][9][0]", KEY_COLOR};
			render.put(thisX, 4, text);

			text.clear(); text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_1, ANSI::bold, ""}; text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_2, ANSI::bold, ""}; text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_3, ANSI::bold, ""}; text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_4, ANSI::bold, ""}; text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_5, ANSI::bold, ""}; text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_6, ANSI::bold, ""}; text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_7, ANSI::bold, ""}; text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_8, ANSI::bold, ""}; text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_9, ANSI::bold, ""}; text += " ";
			text += " "; text += Cell{HOTKEY_CHAR_0, ANSI::bold, ""}; text += " ";
			render.put(thisX, 5, text);

			// character table
			//if (SCREEN_HEIGHT > 16+charCatalogueLineNo + 1) {}
			for (int yc = 0; yc < 16; ++yc) {
				for (int xc = 0; xc < 32; ++xc) {
					render.put(xc+thisX, yc+charCatalogueLineNo, charCatalogue[yc*32 + xc]);
					// signify current character
					if (xc == charCatalogueIndexX && yc == charCatalogueIndexY) {
						render.edit(xc+thisX, yc+charCatalogueLineNo, ANSI::red_back, 1);
					}
				}
			}

			// select keys
			text.clear();
			text += CellString{"[0-9]", KEY_COLOR};
			text += " to set char";
			render.put(thisX, charCatalogueLineNo + 16 + 2, text);
		} else if (sidePanelMode == 3) {  // exporting
			CellString text {};
			int yLevel = 1;
			
			render.put(thisX, yLevel, CellString{"== EXPORTING ==", PANEL_HEADER_COLOR});
			yLevel += 3;


			text += "Mode: ";
			switch (exportMode) {
				case 0: text += CellString{"ART (.ans)", ANSI::underline}; break;
				case 1: text += CellString{"FORE PLT (.plt)", ANSI::underline}; break;
				case 2: text += CellString{"BACK PLT (.plt)", ANSI::underline}; break;
			}
			render.put(thisX, yLevel, text);
			yLevel += 2;


			if (exportMode == 0) {
				// art size
				text.clear(); text += "Dimensions: ";
				text += std::to_string(SCREEN_WIDTH);
				text += "x";
				text += std::to_string(SCREEN_HEIGHT);
				render.put(thisX, yLevel, text);
				yLevel++;

				// encoding
				text.clear(); text += "Encoding: ";
				text += ART_ENCODING ? CellString{"CP437", DISPLAY_ENCODING_CP437, ""} : CellString{"UTF-8", DISPLAY_ENCODING_UTF8, ""};
				render.put(thisX, yLevel, text);
				yLevel++;

				// color mode
				CellString artColorCellstr;
					 if (ART_COLOR_MODE == 0) { artColorCellstr = CellString{"NONE"}; }
				else if (ART_COLOR_MODE == 1) { artColorCellstr = CellString{"4-BIT",  DISPLAY_COLOR_4BIT};  }
				else if (ART_COLOR_MODE == 2) { artColorCellstr = CellString{"8-BIT",  DISPLAY_COLOR_8BIT};  }
				else { artColorCellstr = CellString{"24-BIT", DISPLAY_COLOR_24BIT}; }

				text.clear(); text += "Color (highest): ";
				text += artColorCellstr;
				render.put(thisX, yLevel, text);
				yLevel += 2;
			} else if (exportMode == 1 || exportMode == 2) {
				text.clear();
				text += Cell{"↓", "", ""};
				text += " Palette ";
				text += Cell{"↓", "", ""};
				render.put(thisX, yLevel, text);
				yLevel++;

				text.clear();
				for (size_t c = 0; c < PALETTE_SIZE; ++c) {
					text += Cell{" ", "", ANSI::invertColor((exportMode == 1 ? colorForePalette : colorBackPalette)[c])};
				}
				render.put(thisX, yLevel, text);
				yLevel += 2;
			}

			// do it?
			text.clear();
			text += CellString{"[Entr]", KEY_COLOR};
			text += " to export to ";
			text += exportMode == 0 ? ".ans" : ".plt";
			render.put(thisX, yLevel, text);
			yLevel += 2;


			// export failure
			if (showExportFail) {
				render.put(thisX, yLevel, CellString{"EXPORT FAILURE!", "", ERROR_COLOR});  // TODO diff error for plt/ans?
			}
		} else if (sidePanelMode == 4) {  // importing
			CellString text {};
			int yLevel = 1;
			
			render.put(thisX, yLevel, CellString{"== IMPORTING ==", PANEL_HEADER_COLOR});
			yLevel += 3;

			text += "Mode: ";
			switch (importMode) {
				// TODO this AND export, standardize color here
				case 0: text += CellString{"ART (.ans)", ANSI::underline}; break;
				case 1: text += CellString{"FORE PLT (.plt)", ANSI::underline}; break;
				case 2: text += CellString{"BACK PLT (.plt)", ANSI::underline}; break;
			}
			render.put(thisX, yLevel, text);
			yLevel += 2;


			// import path
			text.clear(); text += Cell{"↓", "", ""};
			text += "  Import Path  ";
			text += Cell{"↓", "", ""};
			render.put(thisX, yLevel, text);
			yLevel++;

			// path
			render.put(thisX, yLevel, CellString{limitString(importPathString, PANEL_SIZE - 3)});
			yLevel += 2;


			// edit path key
			render.put(thisX, yLevel, CellString{"[Z]", KEY_COLOR} + CellString{" to edit path"});
			yLevel++;

			// import
			render.put(thisX, yLevel, CellString{"[Enter]", KEY_COLOR} + CellString{" to import file"});
			yLevel += 2;


			// import failure
			if (showImportFail) {
				render.put(thisX, 9, CellString{"IMPORT FAILURE!", "", ERROR_COLOR});
			}
		} else if (sidePanelMode == 5) {  // settings
			render.put(thisX, 1, CellString{"== SETTINGS ==", PANEL_HEADER_COLOR});

			settingsTopIndex;

			// color mode
			CellString colorMode;
				 if (ART_COLOR_MODE == 0) colorMode = CellString{"NONE"  , SETTINGS_OPTION_COLOR};
			else if (ART_COLOR_MODE == 1) colorMode = CellString{"4-BIT" , SETTINGS_OPTION_COLOR};
			else if (ART_COLOR_MODE == 2) colorMode = CellString{"8-BIT" , SETTINGS_OPTION_COLOR};
			else if (ART_COLOR_MODE == 3) colorMode = CellString{"24-BIT", SETTINGS_OPTION_COLOR};
			render.put(thisX, 3, CellString{std::string(settingsIndex == 0 ? ">" : " ") + " Color mode: "} + colorMode);

			// encoding
			render.put(thisX, 4, CellString{std::string(settingsIndex == 1 ? ">" : " ") + " Encoding: "} + CellString{ART_ENCODING == 0 ? "UTF-8" : "CP437", SETTINGS_OPTION_COLOR});

			// input safe mode
			render.put(thisX, 5, CellString{std::string(settingsIndex == 2 ? ">" : " ") + " Input safemode: "} + CellString{INPUT_SAFE_MODE ? "ON" : "OFF", INPUT_SAFE_MODE ? SETTINGS_COLOR_ON : SETTINGS_COLOR_OFF});
		}

		
		// -- BOTTOM PANEL --

		int thisY = SCREEN_HEIGHT-1 - BOTTOM_PANEL_SIZE + 1;
		// 0 = show everything (width >= 109)
		// 1 = show everything, small palette keybinds (width >= 73)
		// 2 = show palettes (width >= 54)
		// 3 = show palettes no keybinds (width >= 37)
		char mode = 0;
		const int space = SCREEN_WIDTH - 2 - PANEL_SIZE;
		if (space >= 109) ;
		else if (space >= 73) mode = 1;
		else if (space >= 54) mode = 2;
		else if (space >= 37) mode = 3;
		else {
			SCREEN_TOO_SMALL = true;
			mode = 4;
		}

		//DEBUG_STR = std::to_string(space);

		CellString colors;

		if (mode < 3) {
			colors += "[";
			colors += Cell{"Q", KEY_COLOR, ""};
			colors += Cell{"E", KEY_COLOR, ""};
			colors += "/";
			colors += Cell{"W", KEY_COLOR, ""};
			colors += "]: ";
		}
		for (size_t c = 0; c < PALETTE_SIZE; ++c) {
			colors += Cell{colorForeIndex == c ? "█" : "▄", colorForePalette[c], ""};
		} colors += Cell{" ", ANSI::reset, ""}; colors += "   ";
		
		if (mode < 3) {
			colors += "[";
			colors += Cell{"A", KEY_COLOR, ""};
			colors += Cell{"D", KEY_COLOR, ""};
			colors += "/";
			colors += Cell{"S", KEY_COLOR, ""};
			colors += "]: ";
		}
		for (size_t c = 0; c < PALETTE_SIZE; ++c) {
			colors += Cell{colorBackIndex == c ? "█" : "▄", ANSI::invertColor(colorBackPalette[c]), ""};
		} colors += Cell{" ", ANSI::reset, ""};
		if (mode < 2) colors += "   ";


		if (mode < 1) {
			colors += "        [";
			colors += Cell{"{", KEY_COLOR, ""}; 
			colors += "]: color catalogue    [";
			colors += Cell{"}", KEY_COLOR, ""};
			colors += "]: char catalogue";
		} else if (mode < 2) {
			colors += "        [";
			colors += Cell{"{", KEY_COLOR, ""}; 
			colors += "]  [";
			colors += Cell{"}", KEY_COLOR, ""};
			colors += "]";
		}
		
		render.put(2, thisY, colors);


		// -- ART AND BORDERS --

		for (int y = 0; y < SCREEN_HEIGHT; ++y) {
			for (int x = 0; x < SCREEN_WIDTH; ++x) {
				/*        SX
				############
				#          #
				#    $@@@  #
				#    @@@@  #
				#    @@@@  #
				#          #
				############ SY

				art: 4x3
				window: 12, 7
				top left: 5, 2

				bottom right: 8, 4 global  --  top left + (art dimensions - 1)
							: 3, 2 local  --  art dimensions - 1
				*/
			
				// art
				// TODO change this? idk

				if (y < (SCREEN_HEIGHT-1 - BOTTOM_PANEL_SIZE) && x < (SCREEN_WIDTH-1 - PANEL_SIZE + 1)) {
					if (ART.inBounds(x, y) && x < (SCREEN_WIDTH-1 - PANEL_SIZE) && y < (SCREEN_HEIGHT-1 - BOTTOM_PANEL_SIZE)) {
						// in bounds of art
						//reportLog("== x,y=" + std::to_string(x) + "," + std::to_string(y) + "  art: " + std::to_string(ART.x) + "," + std::to_string(ART.y) + "  (" + std::to_string(ART.width) + "x" + std::to_string(ART.height) + ")");
						render.put(x, y, ART.map[(y-ART.y)*ART.width + (x-ART.x)]);
						//render.put(x, y, Cell{"!",ANSI::reset,""});
					}
				}

				// screen borders
				// TODO restructure this

				if (y == 0) {
					if (x == 0)                                render.put(0, 0, Cell{"╔", BORDER_COLOR, ""});
					else if (x == SCREEN_WIDTH-1 - PANEL_SIZE) render.put(x, 0, Cell{"╦", BORDER_COLOR, ""});
					else if (x == SCREEN_WIDTH-1)              render.put(x, 0, Cell{"╗", BORDER_COLOR, ""});
					else                                       render.put(x, 0, Cell{"═", BORDER_COLOR, ""});
				}
				else if (y == SCREEN_HEIGHT-1 - BOTTOM_PANEL_SIZE) {
					if (x == 0)                                render.put(0, y, Cell{"╠", BORDER_COLOR, ""});
					else if (x == SCREEN_WIDTH-1 - PANEL_SIZE) render.put(x, y, Cell{"╣", BORDER_COLOR, ""});
					else if (x < SCREEN_WIDTH-1 - PANEL_SIZE)  render.put(x, y, Cell{"═", BORDER_COLOR, ""});
					else if (x == SCREEN_WIDTH-1)              render.put(x, y, Cell{"║", BORDER_COLOR, ""});
				}
				else if (y == SCREEN_HEIGHT-1) {
					if (x == 0)                                render.put(0, y, Cell{"╚", BORDER_COLOR, ""});
					else if (x == SCREEN_WIDTH-1 - PANEL_SIZE) render.put(x, y, Cell{"╩", BORDER_COLOR, ""});
					else if (x == SCREEN_WIDTH-1)              render.put(x, y, Cell{"╝", BORDER_COLOR, ""});
					else                                       render.put(x, y, Cell{"═", BORDER_COLOR, ""});
				}
				else if (x == 0 || x == SCREEN_WIDTH-1 || x == SCREEN_WIDTH-1 - PANEL_SIZE) {
					render.put(x, y, Cell{"║", BORDER_COLOR, ""});
				}
			}
		}


		// -- CURSOR --
		if (cursorAnim > 1 && !sidePanelMode) {
			render.edit(cursorX, cursorY, CURSOR_COLOR, 0);
			render.edit(cursorX, cursorY, CURSOR_COLOR_BACK, 1);
		}
		if (cursorAnim == 0) cursorAnim = 2;


		// -- POPUP --

		if (popupShowing > 0) {
			int xMid = SCREEN_WIDTH/2, yMid = SCREEN_HEIGHT/2;
			int pwidth = SCREEN_WIDTH * POPUP_WIDTH_SCALE, pheight = SCREEN_HEIGHT * POPUP_HEIGHT_SCALE;
			
			CellString topBar {Cell{"╔", "", ""}};
			LOOP(static_cast<size_t>(pwidth-2)) topBar += Cell{"═", "", ""};
			topBar += Cell{"╗", "", ""};
			render.put(xMid - pwidth/2, yMid - pheight/2, topBar);

			for (int i = 1; i < pheight-1; ++i) {
				CellString midBar = {Cell{"║", "", ""}};
				LOOP(static_cast<size_t>(pwidth-2)) midBar += " ";
				midBar += Cell{"║", "", ""};
				render.put(xMid - pwidth/2, yMid - pheight/2 + i, midBar);
			}

			CellString endBar {Cell{"╚", "", ""}};
			LOOP(static_cast<size_t>(pwidth-2)) endBar += Cell{"═", "", ""};
			endBar += Cell{"╝", "", ""};
			render.put(xMid - pwidth/2, yMid + pheight/2 - 1, endBar);

			if (popupShowing == 1) {
				if (frame > popupFrameStarted + POPUP_INPUT_DELAY) {
					pollInputsIntoString(keyStates, keyStates_slow, inputPopupText);
				}

				int thisx = xMid - pwidth/2 + 2;
				int thisy = yMid - pheight/2 + 1;

				// query text
				render.put(thisx, thisy, CellString{inputPopupTextDisplayed});

				// input from tha user
				CellString userIn = CellString{limitString(inputPopupText, pwidth-4)} + CellString{Cell{cursorAnim > 1 ? "▄" : "_", "", ""}};
				render.put(thisx, thisy+1, userIn);

				// finished yet?
				render.put(thisx, thisy+3, CellString{"[Enter] when done"});
			} else if (popupShowing == 2) {
				int thisx = xMid - inputPopupTextDisplayed.size()/2;
				int thisy = yMid - pheight/2 + 1;

				CellString text1 = CellString{"["} + CellString{"Entr", KEY_COLOR} + CellString{"]"} + CellString{" to confirm"};
				CellString text2 = CellString{"["} + CellString{"Esc", KEY_COLOR} + CellString{"]"} + CellString{" to cancel"};

				render.put(thisx, thisy, CellString{inputPopupTextDisplayed});
				render.put(xMid - text1.size()/2, thisy + pheight/3, text1);
				render.put(xMid - text2.size()/2, thisy + pheight/3 + 1, text2);
			}
		}


		// screen is too small to properly render panels
		if (SCREEN_TOO_SMALL) {
			render.fill(Cell{" ", "", ANSI::red_back_bright});
			render.put(0, 0, CellString{"SCREEN"});
			render.put(0, 1, CellString{"TOO SMALL"});
			// TODO add debug thing here
			// also TODO this sucks
		} else if (sidePanelMode == -1) {
			sidePanelMode = 0;
		}
		
		render.render();
		
		if (cursorAnim && !(frame % static_cast<int>(ANIM_CURSOR*FPS))) cursorAnim--;
		
		// assure proper FPS
		double delta = (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count())/1e6;
		if (delta < static_cast<double>(1.0f/(FPS))) {
			std::this_thread::sleep_for(std::chrono::microseconds(
				static_cast<int>(1e6*(static_cast<double>((1.0f/(FPS))) - delta))
			));
		}
		frame++;
		ignoreInputFrames--;
		ignoreArrowFrames--;
		if (ignoreArrowFrames < 0) ignoreArrowFrames = FAST_KEYSTATE_ARROW_DELAY;
		if (ignoreInputFrames < 0) ignoreInputFrames = 0;
	}


	// -- CLEAN -- 

	if (DEBUG_REPORT_LEVEL >= 2) reportLog("Cleaning up...");

	//clear()
	// TODO: needed?
	//std::cout << "\x1b[>4;m" << std::flush;
	//std::cout << "\x1b[?9001l" << std::flush;
	//system("Set-PSReadLineOption -EditMode Windows");
	std::cout << ANSI::reset << ANSI::cursor_visible << std::flush;

	//std::cin.clear();
	//if (std::cin.rdbuf()->in_avail() > 0) {
	//	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	//}
	//fflush(stdin);
	#ifdef _WIN32
		FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
	#else
		tcflush(STDIN_FILENO, TCIFLUSH);
	#endif
	if (DEBUG_REPORT_LEVEL >= 2) reportLog("\tInput buffer cleared");

	#ifdef _WIN32
		SetConsoleMode(hInput, originalMode);

		cursorInfo.bVisible = TRUE;
		SetConsoleCursorInfo(hOutput, &cursorInfo);
	#else
		system("setterm -cursor on");
		system("stty sane");

		close(keyChecker);
	#endif
	if (DEBUG_REPORT_LEVEL >= 2) reportLog("\tConsole mode reverted");
	
	if (INPUT_SAFE_MODE) {}
	
	
	if (DEBUG_REPORT_LEVEL >= 4) reportLog("\tHave a pleasant day ☺");
	reportLog("END SESSION");

	return 0;
}

// phew!