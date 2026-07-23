// Zaine Rehman, 6/19/2026

#include <iostream>
#include <csignal>
#include <thread>
#include <cstring>
#include <chrono>
#include <atomic>
#include <array>

#include "input.hpp"
#include "output.hpp"
#include "settings.hpp"
#include "lib.hpp"
#include "precalculate.hpp"

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

	srand(time(0));
	std::ios_base::sync_with_stdio(false);

	std::signal(SIGINT,  signalHandler);
	std::signal(SIGTERM, signalHandler);
	std::signal(SIGABRT, signalHandler);

	KeyStates keyStates {};
	KeyStates keyStates_slow {};
	int keyChecker;

	bool SCREEN_TOO_SMALL = false;


	Art ART (20, 5, Cell{DEFAULT_BACK, "", ""});
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
	int catalogueShowing = 1;

	// program frame number
	int frame = 0;
	// animation stage of the cursor
	int cursorAnim = 0;
	// 0 = no, 1 = colors, 2 = chars
	char showingCatalogue = 0;

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

		// raw input

		HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
		DWORD originalMode;
		GetConsoleMode(hInput, &originalMode);

		DWORD dwInputMode = 0;
		GetConsoleMode(hInput, &dwInputMode);
		dwInputMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);
		dwInputMode |= ENABLE_VIRTUAL_TERMINAL_INPUT | ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT;
		SetConsoleMode(hInput, dwInputMode);

		// hide cursor

		CONSOLE_CURSOR_INFO cursorInfo;
    
		GetConsoleCursorInfo(hOutput, &cursorInfo);
		cursorInfo.bVisible = FALSE;
		SetConsoleCursorInfo(hOutput, &cursorInfo);

		if (hInput == INVALID_HANDLE_VALUE || hOutput == INVALID_HANDLE_VALUE) return -1;
	#else
		system("setterm -cursor off");
	
		// set terminal to raw mode and turn off echo
		// most of these are the same as doing "raw" just without -opost because that messes shit up
		// also allowing 'isig' for ctrl+C
		system("stty -ignbrk -brkint -ignpar -inlcr -icanon -ixoff -igncr -icrnl -parmrk -inpck -istrip -ixon isig -iuclc -ixany -imaxbel -xcase min 1 time 0 -echo");
		
		if (!INPUT_SAFE_MODE) {
			// find keyboard
		
			std::string handler = getKeyboardHandler();

			const std::string keyboardPath = "/dev/input/event" + handler;
			std::cout << "Keyboard path: " << keyboardPath << '\n';

			keyChecker = open(keyboardPath.c_str(), O_RDONLY | O_NONBLOCK);
			if (keyChecker == -1) {
				std::cerr 
					<< "COULD NOT LOAD KEYBOARD!\nPath: '" << keyboardPath << "'" 
					<< "\nErr: " << std::strerror(errno)
					<< std::endl;
			}
		}
	#endif
	if (INPUT_SAFE_MODE) {
		std::jthread(safeModeInputHelper).detach();
	} else {
		if (USE_THREADED_INPUT) std::jthread(thread_doKeyStates, std::ref(keyStates), std::ref(keyStates_slow), keyChecker).detach();
	}
	

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


	//  -- LOOP -- 

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
		* 
		* arrows OR [HJKL]: cursor
		* holding [ALT]: fast cursor
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

		if (keyStates[Key::ESC]) RUNNING = false;

		if (keyStates[Key::ALT] ? keyStates[Key::H] || keyStates[Key::LEFT]  : keyStates_slow[Key::H] || keyStates_slow[Key::LEFT])  {
			if (showingCatalogue == 0) {
				cursorX--; cursorAnim = 3;
			} else if (showingCatalogue == 1) {
				if      (catalogueShowing == 0) catalogue4bIndexX--;
				else if (catalogueShowing == 1) catalogue8bIndexX--;
				else if (catalogueShowing == 2) catalogue24bIndexX--;
			} else if (showingCatalogue == 2) charCatalogueIndexX-=2;
		}
		if (keyStates[Key::ALT] ? keyStates[Key::L] || keyStates[Key::RIGHT] : keyStates_slow[Key::L] || keyStates_slow[Key::RIGHT]) {
			if (showingCatalogue == 0) {
				cursorX++; cursorAnim = 3;
			} else if (showingCatalogue == 1) {
				if      (catalogueShowing == 0) catalogue4bIndexX++;
				else if (catalogueShowing == 1) catalogue8bIndexX++;
				else if (catalogueShowing == 2) catalogue24bIndexX++;
			} else if (showingCatalogue == 2) charCatalogueIndexX+=2;
		}
		if (keyStates[Key::ALT] ? keyStates[Key::J] || keyStates[Key::UP]    : keyStates_slow[Key::J] || keyStates_slow[Key::UP])    {
			if (showingCatalogue == 0) {
				cursorY--; cursorAnim = 3;
			} else if (showingCatalogue == 1) {
				if      (catalogueShowing == 0) catalogue4bIndexY--;
				else if (catalogueShowing == 1) catalogue8bIndexY--;
				else if (catalogueShowing == 2) catalogue24bIndexY--;
			} else if (showingCatalogue == 2) charCatalogueIndexY--;
		}
		if (keyStates[Key::ALT] ? keyStates[Key::K] || keyStates[Key::DOWN]  : keyStates_slow[Key::K] || keyStates_slow[Key::DOWN])  {
			if (showingCatalogue == 0) {
				cursorY++; cursorAnim = 3;
			} else if (showingCatalogue == 1) {
				if      (catalogueShowing == 0) catalogue4bIndexY++;
				else if (catalogueShowing == 1) catalogue8bIndexY++;
				else if (catalogueShowing == 2) catalogue24bIndexY++;
			} else if (showingCatalogue == 2) charCatalogueIndexY++;
		}

		if (keyStates[Key::_1] || keyStates[Key::KP_1]) {
			if (showingCatalogue == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_1, 2);
			else if (showingCatalogue == 2) HOTKEY_CHAR_1 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
		}
		if (keyStates[Key::_2] || keyStates[Key::KP_2]) {
			if (showingCatalogue == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_2, 2);
			else if (showingCatalogue == 2) HOTKEY_CHAR_2 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
		}
		if (keyStates[Key::_3] || keyStates[Key::KP_3]) {
			if (showingCatalogue == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_3, 2);
			else if (showingCatalogue == 2) HOTKEY_CHAR_3 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
		}
		if (keyStates[Key::_4] || keyStates[Key::KP_4]) {
			if (showingCatalogue == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_4, 2);
			else if (showingCatalogue == 2) HOTKEY_CHAR_4 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
		}
		if (keyStates[Key::_5] || keyStates[Key::KP_5]) {
			if (showingCatalogue == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_5, 2);
			else if (showingCatalogue == 2) HOTKEY_CHAR_5 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
		}
		if (keyStates[Key::_6] || keyStates[Key::KP_6]) {
			if (showingCatalogue == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_6, 2);
			else if (showingCatalogue == 2) HOTKEY_CHAR_6 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
		}
		if (keyStates[Key::_7] || keyStates[Key::KP_7]) {
			if (showingCatalogue == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_7, 2);
			else if (showingCatalogue == 2) HOTKEY_CHAR_7 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
		}
		if (keyStates[Key::_8] || keyStates[Key::KP_8]) {
			if (showingCatalogue == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_8, 2);
			else if (showingCatalogue == 2) HOTKEY_CHAR_8 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
		}
		if (keyStates[Key::_9] || keyStates[Key::KP_9]) {
			if (showingCatalogue == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_9, 2);
			else if (showingCatalogue == 2) HOTKEY_CHAR_9 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
		}
		if (keyStates[Key::_0] || keyStates[Key::KP_0]) {
			if (showingCatalogue == 0) ART.edit(upd.first, upd.second, HOTKEY_CHAR_0, 2);
			else if (showingCatalogue == 2) HOTKEY_CHAR_0 = charCatalogue[charCatalogueIndexY*32 + charCatalogueIndexX].ch;
		}

		if (keyStates_slow[Key::Q]) { colorForeIndex--; if (colorForeIndex < 0) colorForeIndex = PALETTE_SIZE-1; }
		if (keyStates_slow[Key::E]) { colorForeIndex++; if (colorForeIndex > PALETTE_SIZE-1) colorForeIndex = 0; }
		if (keyStates[Key::W]) {
			if (showingCatalogue == 0) {
				ART.edit(upd.first, upd.second, colorForePalette[colorForeIndex], 0);
				cursorAnim = 1;
			} else if (showingCatalogue == 1) {
				if (catalogueShowing == 0) {
					colorForePalette[colorForeIndex] = ANSI::invertColor(colorCatalogue_4bit[catalogue4bIndexY*COLOR_CATALOGUE_4B_X + catalogue4bIndexX].color_back);
				} else if (catalogueShowing == 1) {
					colorForePalette[colorForeIndex] = ANSI::invertColor(colorCatalogue_8bit[catalogue8bIndexY*COLOR_CATALOGUE_8B_X + catalogue8bIndexX].color_back);
				} else if (catalogueShowing == 2) {
					colorForePalette[colorForeIndex] = ANSI::invertColor(colorCatalogue_24bit[catalogue24bIndexY*COLOR_CATALOGUE_24B_X + catalogue24bIndexX].color_back);
				}
			}
		}
		
		if (keyStates_slow[Key::A]) { colorBackIndex--; if (colorBackIndex < 0) colorBackIndex = PALETTE_SIZE-1; }
		if (keyStates_slow[Key::D]) { colorBackIndex++; if (colorBackIndex > PALETTE_SIZE-1) colorBackIndex = 0; }
		if (keyStates[Key::S]) {
			if (showingCatalogue == 0) {
				ART.edit(upd.first, upd.second, colorBackPalette[colorBackIndex], 1);
				cursorAnim = 1;
			} else if (showingCatalogue == 1) {
				if (catalogueShowing == 0) {
					colorForePalette[colorForeIndex] = colorCatalogue_4bit[catalogue4bIndexY*COLOR_CATALOGUE_4B_X + catalogue4bIndexX].color_back;
				} else if (catalogueShowing == 1) {
					colorForePalette[colorForeIndex] = colorCatalogue_8bit[catalogue8bIndexY*COLOR_CATALOGUE_8B_X + catalogue8bIndexX].color_back;
				} else if (catalogueShowing == 2) {
					colorForePalette[colorForeIndex] = colorCatalogue_24bit[catalogue24bIndexY*COLOR_CATALOGUE_24B_X + catalogue24bIndexX].color_back;
				}
			}
		}

		if (keyStates[Key::C]) {
			ART.edit(upd.first, upd.second, ANSI::reset, 0);
			ART.edit(upd.first, upd.second, ANSI::reset, 1);
			cursorAnim = 1;
		}

		if (keyStates_slow[Key::LBRACKET]) {
			if (showingCatalogue == 1) showingCatalogue = 0;
			else showingCatalogue = 1;
		}
		if (keyStates_slow[Key::RBRACKET]) {
			if (showingCatalogue == 2) showingCatalogue = 0;
			else showingCatalogue = 2;
		}

		if (keyStates_slow[Key::COMMA]) {
			if (showingCatalogue == 1) {
				catalogueShowing--;
			}
		}
		if (keyStates_slow[Key::PERIOD]) {
			if (showingCatalogue == 1) {
				catalogueShowing++;
			}
		}

		//if (keyStates[Key::H]) { ART.resize(1, 0, 0, 0); }
		//if (keyStates[Key::J]) { ART.resize(0, 1, 0, 0); }
		//if (keyStates[Key::K]) { ART.resize(0, 0, 1, 0); }
		//if (keyStates[Key::L]) { ART.resize(0, 0, 0, 1); }

		clamp(cursorX, 0, SCREEN_WIDTH-1);
		clamp(cursorY, 0, SCREEN_HEIGHT-1);

		clamp(catalogueShowing, 0, 2);

		clamp(catalogue4bIndexX, 0, static_cast<int>(COLOR_CATALOGUE_4B_X)-1);
		clamp(catalogue4bIndexY, 0, static_cast<int>(COLOR_CATALOGUE_4B_Y)-1);
		clamp(catalogue8bIndexX, 0, static_cast<int>(COLOR_CATALOGUE_8B_X)-1);
		clamp(catalogue8bIndexY, 0, static_cast<int>(COLOR_CATALOGUE_8B_Y)-1);
		clamp(catalogue24bIndexX, 0, static_cast<int>(COLOR_CATALOGUE_24B_X)-1);
		clamp(catalogue24bIndexY, 0, static_cast<int>(COLOR_CATALOGUE_24B_Y)-1);

		clamp(charCatalogueIndexY, 0, 16-1);
		clamp(charCatalogueIndexX, 1, 32-1);


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

				if (y < (SCREEN_HEIGHT-1 - BOTTOM_PANEL_SIZE) && x < (SCREEN_WIDTH-1 - PANEL_SIZE + 1)) {
					if (ART.inBounds(x, y) && x < (SCREEN_WIDTH-1 - PANEL_SIZE ) && y < (SCREEN_HEIGHT-1 - BOTTOM_PANEL_SIZE)) {
						// in bounds of art
						render.put(x, y, ART.map[(y-ART.y)*ART.width + (x-ART.x)]);
						//render.put(x, y, Cell{"!",ANSI::reset,""});
					}
				}

				// screen borders

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

		int thisX = SCREEN_WIDTH-1 - PANEL_SIZE + 2;

		render.putString(thisX, 1, sunriseAnsi);
		if (showingCatalogue == 0) {  // basic panel
			// this looks like shit
			CellString nums {" [1][2][3][4][5][6][7][8][9][0]"};
			render.putString(thisX, 4, nums);

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
			render.putString(thisX, 5, hotkeys);

			CellString cmode {"Color mode: "};
			if (COLOR_MODE == 0) cmode += "NONE";
			else if (COLOR_MODE == 1) cmode += CellString{"4-BIT", DISPLAY_COLOR_4BIT, ""};
			else if (COLOR_MODE == 2) cmode += CellString{"8-BIT", DISPLAY_COLOR_8BIT, ""};
			else if (COLOR_MODE == 3) cmode += CellString{"24-BIT", DISPLAY_COLOR_24BIT, ""};
			render.putString(thisX, 7, cmode);

			CellString xy {"Size: "};
			xy += std::to_string(SCREEN_WIDTH);
			xy += ", ";
			xy += std::to_string(SCREEN_HEIGHT);
			render.putString(thisX, 9, xy);

			render.putString(thisX, 11, DEBUG_STR);
		} 
		else if (showingCatalogue == 1) {  // color catalogue
			#define colorCatalogueLineNo 6
			int catIndexX, catIndexY;
			float catSizeX, catSizeY;
			CellString catStr;
			CellString catName {"["};

			if (catalogueShowing == 0) {
				catIndexX = catalogue4bIndexX;
				catIndexY = catalogue4bIndexY;
				catSizeX = COLOR_CATALOGUE_4B_X;
				catSizeY = COLOR_CATALOGUE_4B_Y;
				catStr = colorCatalogue_4bit;
				catName += CellString{"4-BIT", DISPLAY_COLOR_4BIT};
			} else if (catalogueShowing == 1) {
				catIndexX = catalogue8bIndexX;
				catIndexY = catalogue8bIndexY;
				catSizeX = COLOR_CATALOGUE_8B_X;
				catSizeY = COLOR_CATALOGUE_8B_Y;
				catStr = colorCatalogue_8bit;
				catName += CellString{"8-BIT", DISPLAY_COLOR_8BIT};
			} else if (catalogueShowing == 2) {
				catIndexX = catalogue24bIndexX;
				catIndexY = catalogue24bIndexY;
				catSizeX = COLOR_CATALOGUE_24B_X;
				catSizeY = COLOR_CATALOGUE_24B_Y;
				catStr = colorCatalogue_24bit;
				catName += CellString{"24-BIT", DISPLAY_COLOR_24BIT};
			}
			catName += "}";

			CellString toChangeCatStr {"["};
			toChangeCatStr += Cell{",", KEY_COLOR, ""};
			toChangeCatStr += "] [";
			toChangeCatStr += Cell{".", KEY_COLOR, ""};
			toChangeCatStr += "] to change";

			// current catalog type
			render.putString(thisX, colorCatalogueLineNo-2, catName);
			render.putString(thisX+14, colorCatalogueLineNo-2, toChangeCatStr);

			// X arrows
			render.put(thisX + catIndexX, colorCatalogueLineNo-1, Cell{"▼", ANSI::bold, ""});
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
			CellString currentColorStr {"Current color: "};
			LOOP(3) currentColorStr += Cell{" ", "", catStr[catIndexY*catSizeX + catIndexX].color_back};
			render.putString(thisX, colorCatalogueLineNo + COLOR_CATALOGUE_LARGEST_Y + 2, currentColorStr);

			// to select
			CellString toSelectStr {"["};
			toSelectStr += Cell{"W", KEY_COLOR, ""};
			toSelectStr += "] and [";
			toSelectStr += Cell{"S", KEY_COLOR, ""};
			toSelectStr += "] to apply to palette";
			render.putString(thisX, colorCatalogueLineNo + COLOR_CATALOGUE_LARGEST_Y + 3, toSelectStr);
		} else if (showingCatalogue == 2) {  // characters
			#define charCatalogueLineNo 7

			// show hotkeys
			CellString nums {" [1][2][3][4][5][6][7][8][9][0]"};
			render.putString(thisX, 4, nums);

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
			render.putString(thisX, 5, hotkeys);

			// character table
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
			CellString toSelectStr {"["};
			toSelectStr += Cell{"0", KEY_COLOR, ""};
			toSelectStr += "-";
			toSelectStr += Cell{"9", KEY_COLOR, ""};
			toSelectStr += "] to set char";
			render.putString(thisX, charCatalogueLineNo + 16 + 2, toSelectStr);
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
		
		render.putString(2, thisY, colors);


		// cursor
		if (cursorAnim > 1 && !showingCatalogue) {
			render.edit(cursorX, cursorY, CURSOR_COLOR, 0);
			render.edit(cursorX, cursorY, CURSOR_COLOR_BACK, 1);
		}
		if (cursorAnim == 0) cursorAnim = 2;


		// screen is too small to properly render panels
		if (SCREEN_TOO_SMALL) {
			render.fill(Cell{" ", "", ANSI::red_back_bright});
			render.putString(0, 0, CellString{"SCREEN"});
			render.putString(0, 1, CellString{"TOO SMALL"});
		}
		
		render.render();
		
		frame++;
		if (cursorAnim && !(frame % static_cast<int>(ANIM_CURSOR*FPS))) cursorAnim--;

		// assure proper FPS
		double delta = (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count())/1e6;
		if (delta < static_cast<double>(1.0f/(FPS))) {
			std::this_thread::sleep_for(std::chrono::microseconds(
				static_cast<int>(1e6*(static_cast<double>((1.0f/(FPS))) - delta))
			));
		}
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

	if (INPUT_SAFE_MODE) {}

	return 0;
}