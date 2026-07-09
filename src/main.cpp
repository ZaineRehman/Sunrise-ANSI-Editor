// Zaine Rehman, 6/19/2026

#include <iostream>
#include <csignal>
#include <thread>
#include <cstring>
#include <chrono>

#include "input.hpp"
#include "output.hpp"
#include "settings.hpp"
#include "lib.hpp"

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

	std::unordered_map<Key,bool> keyStates;
	int keyChecker;


	Art ART (20, 5, Cell{DEFAULT_BACK, "", ""});
	Renderer render {static_cast<uint32_t>(SCREEN_WIDTH), static_cast<uint32_t>(SCREEN_HEIGHT)};
	int cursorX = SCREEN_WIDTH/2, cursorY = SCREEN_HEIGHT/2;

	std::string colorForePalette[PALETTE_SIZE] = {
		ANSI::red,     ANSI::green, ANSI::blue,  ANSI::yellow, 
		ANSI::magenta, ANSI::cyan,  ANSI::white, ANSI::black, 
		ANSI::red_bright,     ANSI::green_bright, ANSI::blue_bright,  ANSI::yellow_bright, 
		ANSI::magenta_bright, ANSI::cyan_bright,  ANSI::white_bright, ANSI::black_bright
	};
	std::string colorBackPalette[PALETTE_SIZE] = {
		ANSI::red_back,     ANSI::green_back, ANSI::blue_back,  ANSI::yellow_back, 
		ANSI::magenta_back, ANSI::cyan_back,  ANSI::white_back, ANSI::black_back, 
		ANSI::red_back_bright,     ANSI::green_back_bright, ANSI::blue_back_bright,  ANSI::yellow_back_bright, 
		ANSI::magenta_back_bright, ANSI::cyan_back_bright,  ANSI::white_back_bright, ANSI::black_back_bright
	};
	int colorForeIndex = 0, colorBackIndex = 0;

	// terminal raw mode
	#ifdef _WIN32
		// files are saved as UTF-8 so dont do this
		//SetConsoleCP(437);
		//SetConsoleOutputCP(437);
		SetConsoleCP(65001);       
		SetConsoleOutputCP(65001); 

		HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
		HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);

		if (hInput == INVALID_HANDLE_VALUE || hOutput == INVALID_HANDLE_VALUE) return -1;

		DWORD dwOutputMode = 0;
		GetConsoleMode(hOutput, &dwOutputMode);
		dwOutputMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(hOutput, dwOutputMode);

		DWORD dwInputMode = 0;
		GetConsoleMode(hInput, &dwInputMode);
		dwInputMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
		SetConsoleMode(hInput, dwInputMode);
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
		std::thread(inputHelper).detach();
	}

	clear();
	

	//  -- PRE CALCULATED STUFF -- 

	CellString sunriseAnsi {concat("Sunrise ANSI Editor   ", VERSION)};

	float r = 255.0f, g = 200.0f, b = 0.0f;
	for (size_t i = 0; i < sunriseAnsi.size(); ++i) {
		// start = 255, 230, 10
		// end   = 135, 205, 255
		sunriseAnsi.internal[i].color_fore = ANSI::Color_24bit::makeColor(
			static_cast<uint8_t>(r), 
			static_cast<uint8_t>(g), 
			static_cast<uint8_t>(b), 
			false
		);
		r -= (255.0f - 135.0f) / static_cast<float>(sunriseAnsi.size());
		g -= (200.0f - 205.0f) / static_cast<float>(sunriseAnsi.size());
		b -=  (10.0f - 255.0f) / static_cast<float>(sunriseAnsi.size());
	}

	CellString catalogueAnsi;

	for (float q = 0.0f; q < COLOR_CATALOGUE_Y; ++q) {
		float specialNumber = (q+1)*(255.0f/COLOR_CATALOGUE_Y);
		//std::cout << specialNumber << '\n';

		if (q == 0.0f) specialNumber += 20.0f;

		float r = specialNumber;
		float g = 0.0f, b = 0.0f;

		for (float i = 0.0f; i < COLOR_CATALOGUE_X; ++i) {
			catalogueAnsi += Cell{" ", "", ANSI::Color_24bit::makeColor(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), true)};

			if (specialNumber == 0.0f) continue;

			if (i < (COLOR_CATALOGUE_X/4.0f)) {
				g += (specialNumber)/(COLOR_CATALOGUE_X/4.0f);
			}
			else if (i < 2.0f*(COLOR_CATALOGUE_X/4.0f)) {
				r -= (specialNumber)/(COLOR_CATALOGUE_X/4.0f);
				b += (specialNumber)/(COLOR_CATALOGUE_X/4.0f);
			}
			else if (i < 3.0f*(COLOR_CATALOGUE_X/4.0f)) {
				g -= (specialNumber)/(COLOR_CATALOGUE_X/4.0f);
			}
			else {
				r += (specialNumber)/(COLOR_CATALOGUE_X/4.0f);
			}
		}
	}

	for (int f = 0; f < COLOR_CATALOGUE_X; ++f) {
		catalogueAnsi += Cell{" ", "", ANSI::Color_24bit::makeColor(f*(255/COLOR_CATALOGUE_X), f*(255/COLOR_CATALOGUE_X), f*(255/COLOR_CATALOGUE_X), true)};
	}
	std::cout << ANSI::reset << std::endl;


	//  -- LOOP -- 

	int frame = 0;
	int cursorAnim = 0;
	// 0 = no, 1 = colors, 2 = chars
	char showingCatalogue = 0;

	while (RUNNING) {
		std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

		//  -- INPUTS -- 

		/*
		* [0-9]: set character
		* 
		* [QE/W]: change/set background color
		* [AD/S]: change/set foreground color
		* 
		* 
		* 
		* arrows OR [UHJK]: cursor
		* 
		* [[]: open character catalogue
		* []]: open color catalogue
		**/

		setKeyStatesOff(keyStates);
		if (INPUT_SAFE_MODE) updateKeyStates_SAFE(keyStates);
		else updateKeyStates(keyStates, keyChecker);

		std::pair<int,int> upd = ART.toArtSpace(cursorX, cursorY);

		if (keyStates[Key::ESC]) RUNNING = false;

		if (keyStates[Key::LEFT])  { cursorX--; cursorAnim = 3; }
		if (keyStates[Key::RIGHT]) { cursorX++; cursorAnim = 3; }
		if (keyStates[Key::UP])    { cursorY--; cursorAnim = 3; }
		if (keyStates[Key::DOWN])  { cursorY++; cursorAnim = 3; }

		if (keyStates[Key::_1] || keyStates[Key::KP_1]) { ART.edit(upd.first, upd.second, HOTKEY_CHAR_1, 2); }
		if (keyStates[Key::_2] || keyStates[Key::KP_2]) { ART.edit(upd.first, upd.second, HOTKEY_CHAR_2, 2); }
		if (keyStates[Key::_3] || keyStates[Key::KP_3]) { ART.edit(upd.first, upd.second, HOTKEY_CHAR_3, 2); }
		if (keyStates[Key::_4] || keyStates[Key::KP_4]) { ART.edit(upd.first, upd.second, HOTKEY_CHAR_4, 2); }
		if (keyStates[Key::_5] || keyStates[Key::KP_5]) { ART.edit(upd.first, upd.second, HOTKEY_CHAR_5, 2); }
		if (keyStates[Key::_6] || keyStates[Key::KP_6]) { ART.edit(upd.first, upd.second, HOTKEY_CHAR_6, 2); }
		if (keyStates[Key::_7] || keyStates[Key::KP_7]) { ART.edit(upd.first, upd.second, HOTKEY_CHAR_7, 2); }
		if (keyStates[Key::_8] || keyStates[Key::KP_8]) { ART.edit(upd.first, upd.second, HOTKEY_CHAR_8, 2); }
		if (keyStates[Key::_9] || keyStates[Key::KP_9]) { ART.edit(upd.first, upd.second, HOTKEY_CHAR_9, 2); }
		if (keyStates[Key::_0] || keyStates[Key::KP_0]) { ART.edit(upd.first, upd.second, HOTKEY_CHAR_0, 2); }

		if (keyStates[Key::Q]) { colorForeIndex--; if (colorForeIndex < 0) colorForeIndex = PALETTE_SIZE-1; }
		if (keyStates[Key::E]) { colorForeIndex++; if (colorForeIndex > PALETTE_SIZE-1) colorForeIndex = 0; }
		if (keyStates[Key::W]) {
			std::pair<int,int> upd = ART.toArtSpace(cursorX, cursorY);
			ART.edit(upd.first, upd.second, colorForePalette[colorForeIndex], 0);
			cursorAnim = 1;
		}

		if (keyStates[Key::A]) { colorBackIndex--; if (colorBackIndex < 0) colorBackIndex = PALETTE_SIZE-1; }
		if (keyStates[Key::D]) { colorBackIndex++; if (colorBackIndex > PALETTE_SIZE-1) colorBackIndex = 0; }
		if (keyStates[Key::S]) {
			ART.edit(upd.first, upd.second, colorBackPalette[colorBackIndex], 1);
			cursorAnim = 1;
		}

		if (keyStates[Key::LBRACKET]) {
			if (showingCatalogue == 1) showingCatalogue = 0;
			else showingCatalogue = 1;
		}
		if (keyStates[Key::RBRACKET]) {
			if (showingCatalogue == 2) showingCatalogue = 0;
			else showingCatalogue = 2;
		}

		if (keyStates[Key::H]) { ART.resize(1, 0, 0, 0); }
		if (keyStates[Key::J]) { ART.resize(0, 1, 0, 0); }
		if (keyStates[Key::K]) { ART.resize(0, 0, 1, 0); }
		if (keyStates[Key::L]) { ART.resize(0, 0, 0, 1); }
		if (keyStates[Key::C]) { DEBUG_STR = ""; }

		clamp(cursorX, 0, SCREEN_WIDTH-1);
		clamp(cursorY, 0, SCREEN_HEIGHT-1);

		//  -- RENDER --

		if (!((frame+1) % ANALYSIS_FREQUENCY)) {
			COLOR_MODE = findHighestColorCode(CellString{ART.map});
		}

		render.clear();

		//DEBUG_STR = std::to_string(ART.x);
		//DEBUG_STR += ",";
		//DEBUG_STR += std::to_string(ART.y);

		std::pair<int,int> check = getTerminalDimensions();
		if (SCREEN_WIDTH != check.first || SCREEN_HEIGHT != check.second) {
			// minuz 1
			SCREEN_WIDTH = check.first - 1;
			SCREEN_HEIGHT = check.second - 1;
			render.resize(SCREEN_WIDTH, SCREEN_HEIGHT);

			// put art in center if first time
			if (frame < 5) {  // no, this isnt right
				ART.x = SCREEN_WIDTH/2 - ART.width;
				ART.y = SCREEN_HEIGHT/2 - ART.height/2;
				// cursor as well, only if program just opened
				if (!frame) cursorX = ART.x + ART.width/2;
				if (!frame) cursorY = ART.y + ART.height/2;
			}
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
				// screen borders

				if (y == 0) {
					if (x == 0) render.put(0, 0, Cell{"╔", BORDER_COLOR, ""});
					else if (x == SCREEN_WIDTH-1 - PANEL_SIZE) render.put(x, 0, Cell{"╦", BORDER_COLOR, ""});
					else if (x == SCREEN_WIDTH-1) render.put(x, 0, Cell{"╗", BORDER_COLOR, ""});
					else render.put(x, 0, Cell{"═", BORDER_COLOR, ""});
				}
				else if (y == SCREEN_HEIGHT-1 - BOTTOM_PANEL_SIZE) {
					if (x == 0) render.put(0, y, Cell{"╠",BORDER_COLOR, ""});
					else if (x == SCREEN_WIDTH-1 - PANEL_SIZE) render.put(x, y, Cell{"╣",BORDER_COLOR, ""});
					else if (x < SCREEN_WIDTH-1 - PANEL_SIZE) render.put(x, y, Cell{"═", BORDER_COLOR, ""});
					else if (x == SCREEN_WIDTH-1) render.put(x, y, Cell{"║",BORDER_COLOR, ""});
				}
				else if (y == SCREEN_HEIGHT-1) {
					if (x == 0) render.put(0, y, Cell{"╚", BORDER_COLOR, ""});
					else if (x == SCREEN_WIDTH-1 - PANEL_SIZE) render.put(x, y, Cell{"╩", BORDER_COLOR, ""});
					else if (x == SCREEN_WIDTH-1) render.put(x, y, Cell{"╝", BORDER_COLOR, ""});
					else render.put(x, y, Cell{"═", BORDER_COLOR, ""});
				}
				else if (x == 0 || x == SCREEN_WIDTH-1 || x == SCREEN_WIDTH-1 - PANEL_SIZE) {
					render.put(x, y, Cell{"║", BORDER_COLOR, ""});
				}
				
				// art
				else {
					if (ART.inBounds(x, y) && x < (SCREEN_WIDTH-1 - PANEL_SIZE ) && y < (SCREEN_HEIGHT-1 - BOTTOM_PANEL_SIZE)) {
						// in bounds of art
						render.put(x, y, ART.map[(y-ART.y)*ART.width + (x-ART.x)]);
						//render.put(x, y, Cell{"!",ANSI::reset,""});
					}
				}

				// cursor
				if (x == cursorX && y == cursorY) {
					if (cursorAnim > 1) {
						render.edit(x, y, ANSI::yellow_bright, 0);
						render.edit(x, y, ANSI::yellow_back_bright, 1);
					}
					if (cursorAnim == 0) cursorAnim = 2;
				}

				//  -- TEXT --

				// side panel
				/*
				 * [x] sunrise text
				 * [x] char hotkeys
				 * [ ] settings button
				 * 		[ ] vim keys
				 * 		[ ] music volume
				 * 		[ ] input safe modefps
				 * 		[ ] cursor animation
				 * 		[ ] border color
				 * 		[ ] key color
				 * 		[ ] side panel size
				 * 		[ ] bottom panel size
				 * [ ] song name
				 * [ ] art size
				 * [ ] export button
				 * [ ] copy/paste button
				 * [ ] load from file button
				 * [ ] animation buttons
				 * [ ] color mode
				 * [ ] direct key input mode
				 * 
				 * [ ] color catalogue
				 * 		[ ] color mode changer
				 * 		[ ] 4-bit code table
				 * 		[ ] 8-bit code table
				 * 		[ ] 8-bit code RGB picker
				 * 		[ ] 24-bit code table
				 * 		[ ] 24-bit code RGB picker
				 * 
				 * [ ] char catalogue
				 * 		[ ] 16x16 grid
				 * 		[ ] hotkey swapper
				**/
				if (x == SCREEN_WIDTH-1 - PANEL_SIZE + 2) {
					if (y == 1) render.putString(x, y, sunriseAnsi);
					if (showingCatalogue == 0) {
						if (y == 4) {
							// this looks like shit
							CellString nums {" [1][2][3][4][5][6][7][8][9][0]"};
							render.putString(x, y, nums);
						} else if (y == 5) {
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
							render.putString(x, y, hotkeys);
						} else if (y == 7) {
							CellString cmode {"Color mode: "};
							     if (COLOR_MODE == 0) cmode += "NONE";
							else if (COLOR_MODE == 1) cmode += CellString{"4-BIT", ANSI::green , ""};
							else if (COLOR_MODE == 2) cmode += CellString{"8-BIT", ANSI::Color_8bit::makeColor(154) , ""};
							else if (COLOR_MODE == 3) cmode += CellString{"24-BIT", ANSI::Color_24bit::makeColor(45, 214, 183) , ""};
							render.putString(x, y, cmode);
						}
					}
					else if (showingCatalogue == 1) {
						catalogueAnsi;
					}
				}

				// bottom panel
				else if (y == SCREEN_HEIGHT-1 - BOTTOM_PANEL_SIZE + 1) {
					if (x == 2) {
						CellString colors;

						colors += "[";
						colors += Cell{"Q", KEY_COLOR, ""};
						colors += Cell{"E", KEY_COLOR, ""};
						colors += "/";
						colors += Cell{"W", KEY_COLOR, ""};
						colors += "]: ";
						for (int c = 0; c < PALETTE_SIZE; ++c) {
							colors += Cell{colorForeIndex == c ? "█" : "▄", colorForePalette[c], ""};
						} colors += Cell{" ", ANSI::reset, ""};  colors += "   ";

						colors += "[";
						colors += Cell{"A", KEY_COLOR, ""};
						colors += Cell{"D", KEY_COLOR, ""};
						colors += "/";
						colors += Cell{"S", KEY_COLOR, ""};
						colors += "]: ";
						for (int c = 0; c < PALETTE_SIZE; ++c) {
							colors += Cell{colorBackIndex == c ? "█" : "▄", ANSI::invertColor(colorBackPalette[c]), ""};
						} colors += Cell{" ", ANSI::reset, ""}; colors += "   ";

						colors += "        [";
						colors += Cell{"{", KEY_COLOR, ""}; 
						colors += "]: color catalogue    [";
						colors += Cell{"}", KEY_COLOR, ""};
						colors += "]: char catalogue";

						// TODO
						colors += DEBUG_STR;

						render.putString(2, y, colors);
					}
				}
			}
		}

		render.render();

		frame++;
		if (cursorAnim && !(frame % static_cast<int>(ANIM_CURSOR*FPS))) cursorAnim--;

		double delta = (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count())/1e6;
		if (delta < static_cast<double>(1.0f/(FPS))) {
			std::this_thread::sleep_for(std::chrono::microseconds(
				static_cast<int>(1e6*(static_cast<double>((1.0f/(FPS))) - delta))
			));
		}
	}


	//  -- CLEAN -- 

	std::cout << ANSI::reset << ANSI::cursor_visible << std::flush;
	//clear()

	#ifdef _WIN32

	#else
		system("setterm -cursor on");
		system("stty sane");

		close(keyChecker);
	#endif

	if (INPUT_SAFE_MODE) {}

	return 0;
}