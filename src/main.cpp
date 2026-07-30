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
	
	//  -- SETUP -- 

	// make sure all directories exist
	if (!std::filesystem::is_directory("Export")) {
		std::filesystem::create_directory("Export");
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
	int colorCatalogueType = 1;

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

	// program frame number
	int frame = 0;
	// animation stage of the cursor
	int cursorAnim = 0;
	// 0 = no, 1 = colors, 2 = chars, 3 = exporting, 4 = importing, 5 = settings
	char sidePanelMode = 0;

	// double-check to make sure art isnt accidentally reset
	bool killingArt = false;

	// if true, failed on export, so say that
	bool showExportFail = false;

	// path for import
	std::string importPathString = "";
	// if true, failed on import, so say that
	bool showImportFail = false;

	// settings index
	// 0 = color mode
	// 1 = encoding
	int settingsIndex = 0;
	// if editing settings
	// 0 = no, 1 = left, 2 = right
	int settingsEdit = 0;


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
	int colorForeIndex = 0, colorBackIndex = 0;

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
				if (DEBUG_REPORT_LEVEL >= 1) reportLog("\t!!!Keyboard not found. errno=" + std::strerror(errno));
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
	

	//  -- PRE CALCULATED STUFF -- 

	CellString sunriseAnsi = getSunriseAnsi();

	CellString colorCatalogue_4bit = getColorCatalogue_4bit();
	CellString colorCatalogue_8bit = getColorCatalogue_8bit();
	CellString colorCatalogue_24bit = getColorCatalogue_24bit();

	CellString charCatalogue = getCharCatalogue();

	if (DEBUG_REPORT_LEVEL >= 2) reportLog("Pre-calculated items");


	//  -- LOOP -- 

	// get rid o this
	//std::this_thread::sleep_for(std::chrono::milliseconds(100));

	if (DEBUG_REPORT_LEVEL >= 2) {
		if (RUNNING) reportLog("Entering main loop...");
		else reportLog("NOT entering main loop: RUNNING is false");
	}
	while (RUNNING) {
		std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

		SCREEN_TOO_SMALL = false;

		//  -- INPUTS -- 

		/*
		* [0-9]: set character
		* 
		* [QE/W]: change/set background color
		* [AD/S]: change/set foreground color
		* 
		* [C]: clear color
		* 
		* [Entr]: export mode
		* 
		* [/]: import mode
		* 		[Z]:  edit path
		* 
		* [\]: settings
		* 
		* [Home] or [Ctrl]+[S]: save
		* 
		* [Bksp]: reset
		* 
		* arrows OR [HJKL]: cursor
		* holding [Alt]: fast cursor
		* 
		* [{]: open character catalogue
		* [}]: open color catalogue
		**/

		//setKeyStatesOff(keyStates);
		//setKeyStatesOff(keyStates_slow);
		if (!USE_THREADED_INPUT) {
			if (INPUT_SAFE_MODE) updateKeyStates_SAFE(keyStates, keyStates_slow);
			else updateKeyStates(keyStates, keyStates_slow, keyChecker);
		}

		std::pair<int,int> upd = ART.toArtSpace(cursorX, cursorY);

		if (keyStates[Key::ESC]) {
			RUNNING = false;

			reportLog("ESC pressed, saving and terminating...");
			saveArtToSession(ART);
		}

		if (keyStates[Key::ALT] ? keyStates[Key::H] || keyStates[Key::LEFT]  : keyStates_slow[Key::H] || keyStates_slow[Key::LEFT])  {
			if (sidePanelMode == 0) {
				cursorX--; cursorAnim = 3;
			} else if (sidePanelMode == 1) {
				if      (colorCatalogueType == 0) catalogue4bIndexX--;
				else if (colorCatalogueType == 1) catalogue8bIndexX--;
				else if (colorCatalogueType == 2) catalogue24bIndexX--;
			} else if (sidePanelMode == 2) charCatalogueIndexX-=2;
			else if (sidePanelMode == 5) settingsEdit = 1;
		}
		if (keyStates[Key::ALT] ? keyStates[Key::L] || keyStates[Key::RIGHT] : keyStates_slow[Key::L] || keyStates_slow[Key::RIGHT]) {
			if (sidePanelMode == 0) {
				cursorX++; cursorAnim = 3;
			} else if (sidePanelMode == 1) {
				if      (colorCatalogueType == 0) catalogue4bIndexX++;
				else if (colorCatalogueType == 1) catalogue8bIndexX++;
				else if (colorCatalogueType == 2) catalogue24bIndexX++;
			} else if (sidePanelMode == 2) charCatalogueIndexX+=2;
			else if (sidePanelMode == 5) settingsEdit = 2;
		}
		if (keyStates[Key::ALT] ? keyStates[Key::J] || keyStates[Key::UP]    : keyStates_slow[Key::J] || keyStates_slow[Key::UP])    {
			if (sidePanelMode == 0) {
				cursorY--; cursorAnim = 3;
			} else if (sidePanelMode == 1) {
				if      (colorCatalogueType == 0) catalogue4bIndexY--;
				else if (colorCatalogueType == 1) catalogue8bIndexY--;
				else if (colorCatalogueType == 2) catalogue24bIndexY--;
			} else if (sidePanelMode == 2) charCatalogueIndexY--;
			else if (sidePanelMode == 5) settingsIndex--;
		}
		if (keyStates[Key::ALT] ? keyStates[Key::K] || keyStates[Key::DOWN]  : keyStates_slow[Key::K] || keyStates_slow[Key::DOWN])  {
			if (sidePanelMode == 0) {
				cursorY++; cursorAnim = 3;
			} else if (sidePanelMode == 1) {
				if      (colorCatalogueType == 0) catalogue4bIndexY++;
				else if (colorCatalogueType == 1) catalogue8bIndexY++;
				else if (colorCatalogueType == 2) catalogue24bIndexY++;
			} else if (sidePanelMode == 2) charCatalogueIndexY++;
			else if (sidePanelMode == 5) settingsIndex++;
		}

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

		if (keyStates_slow[Key::Q] && popupShowing == 0) { colorForeIndex--; if (colorForeIndex < 0) colorForeIndex = PALETTE_SIZE-1; }
		if (keyStates_slow[Key::E] && popupShowing == 0) { colorForeIndex++; if (colorForeIndex > PALETTE_SIZE-1) colorForeIndex = 0; }
		if (keyStates[Key::W] && popupShowing == 0) {
			if (sidePanelMode == 0) {
				ART.edit(upd.first, upd.second, colorForePalette[colorForeIndex], 0);
				cursorAnim = 1;
			} else if (sidePanelMode == 1) {
				if (colorCatalogueType == 0) {
					colorForePalette[colorForeIndex] = ANSI::invertColor(colorCatalogue_4bit[catalogue4bIndexY*COLOR_CATALOGUE_4B_X + catalogue4bIndexX].color_back);
				} else if (colorCatalogueType == 1) {
					colorForePalette[colorForeIndex] = ANSI::invertColor(colorCatalogue_8bit[catalogue8bIndexY*COLOR_CATALOGUE_8B_X + catalogue8bIndexX].color_back);
				} else if (colorCatalogueType == 2) {
					colorForePalette[colorForeIndex] = ANSI::invertColor(colorCatalogue_24bit[catalogue24bIndexY*COLOR_CATALOGUE_24B_X + catalogue24bIndexX].color_back);
				}
			}
		}
		
		if (keyStates_slow[Key::A] && popupShowing == 0) { colorBackIndex--; if (colorBackIndex < 0) colorBackIndex = PALETTE_SIZE-1; }
		if (keyStates_slow[Key::D] && popupShowing == 0) { colorBackIndex++; if (colorBackIndex > PALETTE_SIZE-1) colorBackIndex = 0; }
		if (keyStates[Key::S] && popupShowing == 0) {
			if (sidePanelMode == 0) {
				ART.edit(upd.first, upd.second, colorBackPalette[colorBackIndex], 1);
				cursorAnim = 1;
			} else if (sidePanelMode == 1) {
				if (colorCatalogueType == 0) {
					colorForePalette[colorForeIndex] = colorCatalogue_4bit[catalogue4bIndexY*COLOR_CATALOGUE_4B_X + catalogue4bIndexX].color_back;
				} else if (colorCatalogueType == 1) {
					colorForePalette[colorForeIndex] = colorCatalogue_8bit[catalogue8bIndexY*COLOR_CATALOGUE_8B_X + catalogue8bIndexX].color_back;
				} else if (colorCatalogueType == 2) {
					colorForePalette[colorForeIndex] = colorCatalogue_24bit[catalogue24bIndexY*COLOR_CATALOGUE_24B_X + catalogue24bIndexX].color_back;
				}
			}
		}

		if (keyStates[Key::C] && popupShowing == 0) {
			ART.edit(upd.first, upd.second, ANSI::reset, 0);
			ART.edit(upd.first, upd.second, ANSI::reset, 1);
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

		if (keyStates_slow[Key::BACKSPACE] && popupShowing == 0) {
			if (killingArt) {
				// TODO center of screen instead?
				ART.reset(cursorX, cursorY);
				killingArt = false;
			} else {
				killingArt = true;
			}
		}
		
		if (keyStates_slow[Key::ESC]) {
			if (killingArt) {
				killingArt = false;
			}
		}

		if (keyStates_slow[Key::ENTER] || keyStates_slow[Key::KP_ENTER]) {
			// i dont even remember why i put this directive here
			#ifndef NDEBUG
				if (frame > 10) {
			#endif
			if (sidePanelMode == 0) sidePanelMode = 3;
			else if (sidePanelMode == 3) {
				// export art
				std::string filename = "Export/Exported_Art_" + getTimestamp() + ".ans";
				
				if (!loadArtIntoFile(ART, filename)) {
					showExportFail = true;
					if (DEBUG_REPORT_LEVEL >= 1) reportLog("!!! Failure to export file: " + filename);
				} else {
					sidePanelMode = 0;
				}
			} else if (sidePanelMode == 4) {
				if (popupShowing) {
					popupShowing = 0;
					importPathString = inputPopupText;
				} else {
					if (!loadArtFromFile(importPathString, ART)) {
						showImportFail = true;
						if (DEBUG_REPORT_LEVEL >= 1) reportLog("!!! Failure to import file: " + importPathString);
					} else {
						sidePanelMode = 0;
					}
				}
			}
			#ifndef NDEBUG
				}
			#endif
		}

		if (keyStates_slow[Key::HOME] || (keyStates[Key::CTRL] && keyStates_slow[Key::S])) {
			saveArtToSession(ART);
		}

		if (keyStates_slow[Key::SLASH] && popupShowing == 0) {
			if (sidePanelMode == 4) sidePanelMode = 0;
			else sidePanelMode = 4;
		}

		if (keyStates_slow[Key::Z] && popupShowing == 0) {
			if (sidePanelMode == 4) {
				popupShowing = 1;
				inputPopupTextDisplayed = "Enter file path for import";
				popupFrameStarted = frame;
			}
		}

		if (keyStates_slow[Key::BSLASH]) {
			if (sidePanelMode == 5) sidePanelMode = 0;
			else sidePanelMode = 5;
		}

		//if (keyStates[Key::H]) { ART.resize(1, 0, 0, 0); }
		//if (keyStates[Key::J]) { ART.resize(0, 1, 0, 0); }
		//if (keyStates[Key::K]) { ART.resize(0, 0, 1, 0); }
		//if (keyStates[Key::L]) { ART.resize(0, 0, 0, 1); }


		// -- CLAMP VALUES -- 

		clamp(cursorX, 0, SCREEN_WIDTH-1);
		clamp(cursorY, 0, SCREEN_HEIGHT-1);

		clamp(colorCatalogueType, 0, 2);

		clamp(catalogue4bIndexX, 0, static_cast<int>(COLOR_CATALOGUE_4B_X)-1);
		clamp(catalogue4bIndexY, 0, static_cast<int>(COLOR_CATALOGUE_4B_Y)-1);
		clamp(catalogue8bIndexX, 0, static_cast<int>(COLOR_CATALOGUE_8B_X)-1);
		clamp(catalogue8bIndexY, 0, static_cast<int>(COLOR_CATALOGUE_8B_Y)-1);
		clamp(catalogue24bIndexX, 0, static_cast<int>(COLOR_CATALOGUE_24B_X)-1);
		clamp(catalogue24bIndexY, 0, static_cast<int>(COLOR_CATALOGUE_24B_Y)-1);

		clamp(charCatalogueIndexY, 0, 16-1);
		clamp(charCatalogueIndexX, 1, 32-1);

		clamp_rollover(settingsIndex, 0, SETTINGS_AMOUNT-1);

		// screen scrolling
		if (cursorX < 1) {
			ART.x += SCREEN_WIDTH - PANEL_SIZE-2;
			cursorX = SCREEN_WIDTH - PANEL_SIZE-2;
		}
		else if (cursorX >= SCREEN_WIDTH - PANEL_SIZE-1) {
			ART.x -= SCREEN_WIDTH-1 - PANEL_SIZE-2;
			cursorX = 1;
		}
		if (cursorY < 1) {
			ART.y += SCREEN_HEIGHT-1 - BOTTOM_PANEL_SIZE-2;
			cursorY = SCREEN_HEIGHT - BOTTOM_PANEL_SIZE-2;
		}
		else if (cursorY >= SCREEN_HEIGHT - BOTTOM_PANEL_SIZE-1) {
			ART.y -= SCREEN_HEIGHT-1 - BOTTOM_PANEL_SIZE-2;
			cursorY = 1;
		}

		// editing settings
		if (settingsEdit > 0) {
			switch (settingsIndex) {
				case 0:  // color mode
					if (settingsEdit == 1) COLOR_MODE--;
					else COLOR_MODE++;

					clamp_rollover(COLOR_MODE, 0, 3);

					break;
				case 1:  // encoding
					if (settingsEdit == 1) ART_ENCODING--;
					else ART_ENCODING++;

					clamp_rollover(ART_ENCODING, 0, 1);

					break;
			}
		}


		//  -- RENDER --

		if (!(frame % ANALYSIS_FREQUENCY)) {
			COLOR_MODE = findHighestColorCode(CellString{ART.map});
		}

		render.clear();

		//DEBUG_STR = std::to_string(ART.x);
		//DEBUG_STR += ",";
		//DEBUG_STR += std::to_string(ART.y);

		std::pair<int,int> check = getTerminalDimensions();
		if (SCREEN_WIDTH != check.first || SCREEN_HEIGHT != check.second) {
			// minuz 1
			SCREEN_WIDTH = check.first;
			SCREEN_HEIGHT = check.second;
			render.resize(SCREEN_WIDTH, SCREEN_HEIGHT);

			// put art in center if first time
			if (frame < 5) {  // no, this isnt right
				ART.x = SCREEN_WIDTH/2 - ART.width/2 - PANEL_SIZE/2;
				ART.y = SCREEN_HEIGHT/2 - ART.height/2 - BOTTOM_PANEL_SIZE;
				// cursor as well, only if program just opened
				if (!frame) cursorX = ART.x + ART.width/2;
				if (!frame) cursorY = ART.y + ART.height/2;
			}

			render.clear();
		}

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

		//  -- TEXT --
				
		// side panel

		if (SCREEN_HEIGHT < 3) {
			sidePanelMode = -1;
			SCREEN_TOO_SMALL = true;
		}
		int thisX = SCREEN_WIDTH-1 - PANEL_SIZE + 2;
		
		if (sidePanelMode == 0) {  // basic panel
			// sunrise text
			render.put(thisX, 1, sunriseAnsi);

			// this looks like shit
			// TODO only use 1 CellString for the whole thing
			CellString nums {" [1][2][3][4][5][6][7][8][9][0]"};
			render.put(thisX, 4, nums);

			// char hotkeys
			CellString hotkeys {" "};
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_1, KEY_COLOR, ""}; hotkeys += " ";
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_2, KEY_COLOR, ""}; hotkeys += " ";
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_3, KEY_COLOR, ""}; hotkeys += " ";
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_4, KEY_COLOR, ""}; hotkeys += " ";
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_5, KEY_COLOR, ""}; hotkeys += " ";
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_6, KEY_COLOR, ""}; hotkeys += " ";
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_7, KEY_COLOR, ""}; hotkeys += " ";
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_8, KEY_COLOR, ""}; hotkeys += " ";
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_9, KEY_COLOR, ""}; hotkeys += " ";
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_0, KEY_COLOR, ""}; hotkeys += " ";
			render.put(thisX, 5, hotkeys);

			// color mode
			CellString cmode {"Color mode: "};
				 if (COLOR_MODE == 0) cmode += "NONE";
			else if (COLOR_MODE == 1) cmode += CellString{"4-BIT", DISPLAY_COLOR_4BIT, ""};
			else if (COLOR_MODE == 2) cmode += CellString{"8-BIT", DISPLAY_COLOR_8BIT, ""};
			else if (COLOR_MODE == 3) cmode += CellString{"24-BIT", DISPLAY_COLOR_24BIT, ""};
			render.put(thisX, 7, cmode);

			// encoding
			CellString encoding {"Encoding: "};
				 if (ART_ENCODING == 0) encoding += CellString{"UTF-8", DISPLAY_ENCODING_UTF8};
			else if (ART_ENCODING == 1) encoding += CellString{"CP437", DISPLAY_ENCODING_CP437};
			render.put(thisX, 8, encoding);

			// art dimensions
			CellString xy {"Size: "};
			xy += std::to_string(SCREEN_WIDTH);
			xy += "x";
			xy += std::to_string(SCREEN_HEIGHT);
			render.put(thisX, 10, xy);

			// enter to export
			CellString toExportStr {"["};
			toExportStr.append("Entr", KEY_COLOR, "");
			toExportStr += "] to export";
			render.put(thisX, 12, toExportStr);

			// slash to import
			CellString toImportStr {"["};
			toImportStr.append("/", KEY_COLOR, "");
			toImportStr += "] to import";
			render.put(thisX, 13, toImportStr);

			// backspace to reset
			CellString toResetStr {"["};
			toResetStr.append("Bksp", KEY_COLOR, "");
			toResetStr += "] to clear art";
			render.put(thisX, 15, toResetStr);
		} 
		else if (sidePanelMode == 1) {  // color catalogue
			#define colorCatalogueLineNo 6
			int catIndexX, catIndexY;
			float catSizeX, catSizeY;
			CellString catStr;
			CellString catName {"["};

			if (colorCatalogueType == 0) {
				catIndexX = catalogue4bIndexX;
				catIndexY = catalogue4bIndexY;
				catSizeX = COLOR_CATALOGUE_4B_X;
				catSizeY = COLOR_CATALOGUE_4B_Y;
				catStr = colorCatalogue_4bit;
				catName += CellString{"4-BIT", DISPLAY_COLOR_4BIT};
			} else if (colorCatalogueType == 1) {
				catIndexX = catalogue8bIndexX;
				catIndexY = catalogue8bIndexY;
				catSizeX = COLOR_CATALOGUE_8B_X;
				catSizeY = COLOR_CATALOGUE_8B_Y;
				catStr = colorCatalogue_8bit;
				catName += CellString{"8-BIT", DISPLAY_COLOR_8BIT};
			} else if (colorCatalogueType == 2) {
				catIndexX = catalogue24bIndexX;
				catIndexY = catalogue24bIndexY;
				catSizeX = COLOR_CATALOGUE_24B_X;
				catSizeY = COLOR_CATALOGUE_24B_Y;
				catStr = colorCatalogue_24bit;
				catName += CellString{"24-BIT", DISPLAY_COLOR_24BIT};
			}
			catName += "}";

			render.put(thisX, 1, CellString{"== COLOR CATALOGUE ==", PANEL_HEADER_COLOR});

			CellString toChangeCatStr {"["};
			toChangeCatStr += Cell{",", KEY_COLOR, ""};
			toChangeCatStr += "] [";
			toChangeCatStr += Cell{".", KEY_COLOR, ""};
			toChangeCatStr += "] to change";

			if (SCREEN_HEIGHT > colorCatalogueLineNo + COLOR_CATALOGUE_LARGEST_Y + 2) { 
				// current catalog type
				render.put(thisX, colorCatalogueLineNo-2, catName);
				render.put(thisX+14, colorCatalogueLineNo-2, toChangeCatStr);

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
			}

			// current color
			CellString currentColorStr {"Current color: "};
			LOOP(3) currentColorStr += Cell{" ", "", catStr[catIndexY*catSizeX + catIndexX].color_back};
			render.put(thisX, colorCatalogueLineNo + COLOR_CATALOGUE_LARGEST_Y + 2, currentColorStr);

			// to select
			CellString toSelectStr {"["};
			toSelectStr += Cell{"W", KEY_COLOR, ""};
			toSelectStr += "] and [";
			toSelectStr += Cell{"S", KEY_COLOR, ""};
			toSelectStr += "] to apply to palette";
			render.put(thisX, colorCatalogueLineNo + COLOR_CATALOGUE_LARGEST_Y + 3, toSelectStr);
		} else if (sidePanelMode == 2) {  // characters
			#define charCatalogueLineNo 7

			render.put(thisX, 1, CellString{"== CHAR CATALOGUE ==", PANEL_HEADER_COLOR});

			// show hotkeys
			CellString nums {" [1][2][3][4][5][6][7][8][9][0]"};
			render.put(thisX, 4, nums);

			CellString hotkeys {" "};
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_1, ANSI::bold, ""}; hotkeys += " ";
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_2, ANSI::bold, ""}; hotkeys += " ";
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_3, ANSI::bold, ""}; hotkeys += " ";
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_4, ANSI::bold, ""}; hotkeys += " ";
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_5, ANSI::bold, ""}; hotkeys += " ";
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_6, ANSI::bold, ""}; hotkeys += " ";
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_7, ANSI::bold, ""}; hotkeys += " ";
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_8, ANSI::bold, ""}; hotkeys += " ";
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_9, ANSI::bold, ""}; hotkeys += " ";
			hotkeys += " "; hotkeys += Cell{HOTKEY_CHAR_0, ANSI::bold, ""}; hotkeys += " ";
			render.put(thisX, 5, hotkeys);

			// character table
			if (SCREEN_HEIGHT > 16+charCatalogueLineNo + 1) {
				for (int yc = 0; yc < 16; ++yc) {
					for (int xc = 0; xc < 32; ++xc) {
						render.put(xc+thisX, yc+charCatalogueLineNo, charCatalogue[yc*32 + xc]);
						// signify current character
						if (xc == charCatalogueIndexX && yc == charCatalogueIndexY) {
							render.edit(xc+thisX, yc+charCatalogueLineNo, ANSI::red_back, 1);
						}
					}
				}
			}

			// select keys
			CellString toSelectStr {"["};
			toSelectStr += Cell{"0", KEY_COLOR, ""};
			toSelectStr += "-";
			toSelectStr += Cell{"9", KEY_COLOR, ""};
			toSelectStr += "] to set char";
			render.put(thisX, charCatalogueLineNo + 16 + 2, toSelectStr);
		} else if (sidePanelMode == 3) {  // exporting
			render.put(thisX, 1, CellString{"== EXPORTING ==", PANEL_HEADER_COLOR});

			// art size
			CellString artSize {"Dimensions: "};
			artSize += std::to_string(SCREEN_WIDTH);
			artSize += "x";
			artSize += std::to_string(SCREEN_HEIGHT);
			render.put(thisX, 4, artSize);

			// do it?
			CellString doIt {"["};
			doIt += CellString{"Entr", KEY_COLOR};
			doIt += "] to export to .ans";
			render.put(thisX, 6, doIt);

			// export failure
			if (showExportFail) {
				render.put(thisX, 8, CellString{"EXPORT FAILURE!", "", ERROR_COLOR});  // TODO put color in settings
			}
		} else if (sidePanelMode == 4) {  // importing
			render.put(thisX, 1, CellString{"== IMPORTING ==", PANEL_HEADER_COLOR});

			// import path
			CellString topTextStr {Cell{"↓", "", ""}};
			topTextStr += "  Import Path  ";
			topTextStr += Cell{"↓", "", ""};
			render.put(thisX, 3, topTextStr);

			// path
			render.put(thisX, 4, CellString{limitString(importPathString, PANEL_SIZE - 3)});

			// edit path key
			render.put(thisX, 6, CellString{"[Z] to edit path"});

			// import
			render.put(thisX, 7, CellString{"[Enter] to import .ans file"});

			// import failure
			if (showImportFail) {
				render.put(thisX, 9, CellString{"IMPORT FAILURE!", "", ERROR_COLOR});
			}
		} else if (sidePanelMode == 5) {  // settings
			render.put(thisX, 1, CellString{"== SETTINGS ==", PANEL_HEADER_COLOR});

			// color mode
			CellString colorMode;
				 if (COLOR_MODE == 0) colorMode = CellString{"NONE"  , SETTINGS_OPTION_COLOR};
			else if (COLOR_MODE == 1) colorMode = CellString{"4-BIT" , SETTINGS_OPTION_COLOR};
			else if (COLOR_MODE == 2) colorMode = CellString{"8-BIT" , SETTINGS_OPTION_COLOR};
			else if (COLOR_MODE == 3) colorMode = CellString{"24-BIT", SETTINGS_OPTION_COLOR};
			render.put(thisX, 3, CellString{std::string(settingsIndex == 0 ? ">" : " ") + " Color mode: "} + colorMode);

			// encoding
			render.put(thisX, 4, CellString{std::string(settingsIndex == 1 ? ">" : " ") + " Encoding: "} + CellString{ART_ENCODING == 0 ? "UTF-8" : "CP437", SETTINGS_OPTION_COLOR});

			// 
		}

		
		// bottom panel

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
		for (int c = 0; c < PALETTE_SIZE; ++c) {
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
		for (int c = 0; c < PALETTE_SIZE; ++c) {
			colors += Cell{colorBackIndex == c ? "█" : "▄", ANSI::invertColor(colorBackPalette[c]), ""};
		} colors += Cell{" ", ANSI::reset, ""}; colors += "   ";


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


		// cursor
		if (cursorAnim > 1 && !sidePanelMode) {
			render.edit(cursorX, cursorY, CURSOR_COLOR, 0);
			render.edit(cursorX, cursorY, CURSOR_COLOR_BACK, 1);
		}
		if (cursorAnim == 0) cursorAnim = 2;

		// popup
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

			if (frame > popupFrameStarted + POPUP_INPUT_DELAY) {
				pollInputsIntoString(keyStates, keyStates_slow, inputPopupText);
			}
			// Export/Exported_Art_2026-7-23_15-26-9.ans
			// Export/Exported_Art_2026-7-23_15-26-9.ans

			int thisx = xMid - pwidth/2 + 2;
			int thisy = yMid - pheight/2 + 1;

			// query text
			render.put(thisx, thisy, CellString{inputPopupTextDisplayed});

			// input from tha user
			CellString userIn = CellString{limitString(inputPopupText, pwidth-4)} + CellString{Cell{cursorAnim > 1 ? "▄" : "_", "", ""}};
			render.put(thisx, thisy+1, userIn);

			// finished yet?
			render.put(thisx, thisy+3, CellString{"[Enter] when done"});
		}


		// screen is too small to properly render panels
		if (SCREEN_TOO_SMALL) {
			render.fill(Cell{" ", "", ANSI::red_back_bright});
			render.put(0, 0, CellString{"SCREEN"});
			render.put(0, 1, CellString{"TOO SMALL"});
			// TODO add debug thing here
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
	}


	//  -- CLEAN -- 

	//clear()
	// TODO: needed?
	//std::cout << "\x1b[>4;m" << std::flush;
	//std::cout << "\x1b[?9001l" << std::flush;
	//system("Set-PSReadLineOption -EditMode Windows");
	std::cout << ANSI::reset << ANSI::cursor_visible << std::flush;

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

	reportLog("\tEND SESSION");

	return 0;
}