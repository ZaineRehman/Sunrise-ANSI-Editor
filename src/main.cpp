// Zaine Rehman, 6/19/2026

#include <iostream>
#include <atomic>
#include <csignal>
#include <thread>
#include <cstring>
#include <chrono>

#include "input.hpp"
#include "output.hpp"
#include "settings.hpp"

#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <linux/input.h>
	#include <fcntl.h>
	#include <termios.h>
#endif


std::unordered_map<Key,bool> keyStates;
int keyChecker;


std::atomic<bool> RUNNING(true);

void signalHandler(int signal) {
	switch (signal) {
		case SIGINT: 
		case SIGTERM: 
		case SIGABRT: 
			RUNNING = false;
			break;
	}
}

// inclusive
template <typename T>
inline void clamp(T& num, const T& lower, const T& upper) {
	     if (num < lower) num = lower;
	else if (num > upper) num = upper;
}

std::string concat(const std::string& str1, const char* str2) {
	return str1 + str2;
}

class Art {
public: 
	std::vector<Cell> map;
	int width, height;
	int x, y;

	Art() = default;
	Art(int w, int h, const Cell& def) {
		map.resize(w*h, def);
	}
};


Art ART (20, 5, Cell{"@", "", ""});
int cursorX = SCREEN_WIDTH/2, cursorY = SCREEN_HEIGHT/2;

std::string colorForePalette[16] = {
	ANSI::red, ANSI::green, ANSI::blue, ANSI::yellow, 
	ANSI::magenta, ANSI::cyan, ANSI::white, ANSI::black, 
	ANSI::red_bright, ANSI::green_bright, ANSI::blue_bright, ANSI::yellow_bright, 
	ANSI::magenta_bright, ANSI::cyan_bright, ANSI::white_bright, ANSI::black_bright
};
std::string colorBackPalette[16] = {
	ANSI::red_back, ANSI::green_back, ANSI::blue_back, ANSI::yellow_back, 
	ANSI::magenta_back, ANSI::cyan_back, ANSI::white_back, ANSI::black_back, 
	ANSI::red_back_bright, ANSI::green_back_bright, ANSI::blue_back_bright, ANSI::yellow_back_bright, 
	ANSI::magenta_back_bright, ANSI::cyan_back_bright, ANSI::white_back_bright, ANSI::black_back_bright
};
int colorForeIndex = 0, colorBackIndex = 0;


int main() {

	//  -- SETUP -- 

	srand(time(0));
	std::ios_base::sync_with_stdio(false);

	std::signal(SIGINT,  signalHandler);
	std::signal(SIGTERM, signalHandler);
	std::signal(SIGABRT, signalHandler);

	Renderer render {static_cast<uint32_t>(SCREEN_WIDTH), static_cast<uint32_t>(SCREEN_HEIGHT)};

	// terminal raw mode
	#ifdef _WIN32
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
		// start = 255, 230, 0
		// end   = 145, 225, 255
		sunriseAnsi.internal[i].color_fore = ANSI::Color_24bit::makeColor(
			static_cast<uint8_t>(r), 
			static_cast<uint8_t>(g), 
			static_cast<uint8_t>(b), 
			false
		);
		r -= (255.0f - 145.0f) / static_cast<float>(sunriseAnsi.size());
		g -= (200.0f - 225.0f) / static_cast<float>(sunriseAnsi.size());
		b -=   (0.0f - 255.0f) / static_cast<float>(sunriseAnsi.size());
	}


	//  -- LOOP -- 

	int frame = 0;
	int cursorAnim = 0;

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
		* [Z]: open character catalogue
		* [X]: open color catalogue
		**/

		setKeyStatesOff(keyStates);
		if (INPUT_SAFE_MODE) updateKeyStates_SAFE(keyStates);
		else updateKeyStates(keyStates, keyChecker);

		if (keyStates[Key::ESC]) RUNNING = false;

		if (keyStates[Key::LEFT])  { cursorX--; cursorAnim = 3; }
		if (keyStates[Key::RIGHT]) { cursorX++; cursorAnim = 3; }
		if (keyStates[Key::UP])    { cursorY--; cursorAnim = 3; }
		if (keyStates[Key::DOWN])  { cursorY++; cursorAnim = 3; }

		if (keyStates[Key::_1] || keyStates[Key::KP_1]) { ART.map[cursorY*ART.width + cursorX].ch = HOTKEY_CHAR_1; }
		if (keyStates[Key::_2] || keyStates[Key::KP_2]) { ART.map[cursorY*ART.width + cursorX].ch = HOTKEY_CHAR_2; }
		if (keyStates[Key::_3] || keyStates[Key::KP_3]) { ART.map[cursorY*ART.width + cursorX].ch = HOTKEY_CHAR_3; }
		if (keyStates[Key::_4] || keyStates[Key::KP_4]) { ART.map[cursorY*ART.width + cursorX].ch = HOTKEY_CHAR_4; }
		if (keyStates[Key::_5] || keyStates[Key::KP_5]) { ART.map[cursorY*ART.width + cursorX].ch = HOTKEY_CHAR_5; }
		if (keyStates[Key::_6] || keyStates[Key::KP_6]) { ART.map[cursorY*ART.width + cursorX].ch = HOTKEY_CHAR_6; }
		if (keyStates[Key::_7] || keyStates[Key::KP_7]) { ART.map[cursorY*ART.width + cursorX].ch = HOTKEY_CHAR_7; }
		if (keyStates[Key::_8] || keyStates[Key::KP_8]) { ART.map[cursorY*ART.width + cursorX].ch = HOTKEY_CHAR_8; }
		if (keyStates[Key::_9] || keyStates[Key::KP_9]) { ART.map[cursorY*ART.width + cursorX].ch = HOTKEY_CHAR_9; }
		if (keyStates[Key::_0] || keyStates[Key::KP_0]) { ART.map[cursorY*ART.width + cursorX].ch = HOTKEY_CHAR_0; }

		clamp(cursorX, 0, SCREEN_WIDTH-1);
		clamp(cursorY, 0, SCREEN_HEIGHT-1);

		//  -- RENDER --

		render.clear();

		std::pair<int,int> check = getTerminalDimensions();
		if (SCREEN_WIDTH != check.first || SCREEN_HEIGHT != check.second) {
			// minuz 1
			SCREEN_WIDTH = check.first - 1;
			SCREEN_HEIGHT = check.second - 1;
			render.resize(SCREEN_WIDTH, SCREEN_HEIGHT);

			// also put art in center (for now)
			ART.x = SCREEN_WIDTH/2 - ART.width/2;
			ART.y = SCREEN_HEIGHT/2 - ART.height/2;
			// cursor as well, only if program just opened
			if (!frame) cursorX = ART.x + ART.width/2;
			if (!frame) cursorY = ART.y + ART.height/2;
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
					if (x == 0) render.put(0, 0, Cell{"╔", ANSI::bold, ""});
					else if (x == SCREEN_WIDTH-1 - PANEL_SIZE) render.put(x, 0, Cell{"╦", ANSI::bold, ""});
					else if (x == SCREEN_WIDTH-1) render.put(x, 0, Cell{"╗", ANSI::bold, ""});
					else render.put(x, 0, Cell{"═", ANSI::bold, ""});
				}
				else if (y == SCREEN_HEIGHT-1 - BOTTOM_PANEL_SIZE) {
					if (x == 0) render.put(0, y, Cell{"╠",ANSI::bold, ""});
					else if (x == SCREEN_WIDTH-1 - PANEL_SIZE) render.put(x, y, Cell{"╣",ANSI::bold, ""});
					else if (x < SCREEN_WIDTH-1 - PANEL_SIZE) render.put(x, y, Cell{"═", ANSI::bold, ""});
					else if (x == SCREEN_WIDTH-1) render.put(x, y, Cell{"╣",ANSI::bold, ""});
				}
				else if (y == SCREEN_HEIGHT-1) {
					if (x == 0) render.put(0, y, Cell{"╚", ANSI::bold, ""});
					else if (x == SCREEN_WIDTH-1 - PANEL_SIZE) render.put(x, y, Cell{"╩", ANSI::bold, ""});
					else if (x == SCREEN_WIDTH-1) render.put(x, y, Cell{"╝", ANSI::bold, ""});
					else render.put(x, y, Cell{"═", ANSI::bold, ""});
				}
				else if (x == 0 || x == SCREEN_WIDTH-1 || x == SCREEN_WIDTH-1 - PANEL_SIZE) {
					render.put(x, y, Cell{"║", ANSI::bold, ""});
				}
				
				// art
				else {
					if (
						x >= ART.x && x < ART.x+ART.width &&
						y >= ART.y && y < ART.y+ART.height
					) {
						// in bounds of art
						render.put(x, y, ART.map[(y-ART.y)*ART.width + (x-ART.x)]);
						//render.put(x, y, Cell{"!",ANSI::reset,""});
					}
				}

				// cursor
				if (x == cursorX && y == cursorY) {
					if (cursorAnim > 1) {
						render.edit(x, y, ANSI::yellow_back_bright, 2);
					}
					if (cursorAnim == 0) cursorAnim = 2;
				}

				//  -- TEXT --

				// side panel
				if (y == 1) {
					if (x == SCREEN_WIDTH-1 - PANEL_SIZE + 2) {
						render.putString(x, y, sunriseAnsi);
					}
					else if (x == SCREEN_WIDTH-1 - PANEL_SIZE + 4) {
						render.putString(x, y, static_cast<std::string>(ART.x + ", " + ART.y));
					}
				}

				// bottom panel
				else if (y == SCREEN_HEIGHT-1 - BOTTOM_PANEL_SIZE + 1) {
					if (x == 2) {
						//render.putString(2, y, CellString{
						//	"[0-9]: characters        [Q-Y]: foreground colors        [A-H]: background colors"  // len = 81
						//});
						//render.putString(2+81, y, CellString{
						//	"[←↕→]: cursor        "
						//});
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
		#ifdef INPUT_SAFE_MODE
			
		#endif
	#else
		#ifdef INPUT_SAFE_MODE
			
		#endif
		system("setterm -cursor on");
		system("stty sane");

		close(keyChecker);
	#endif

	return 0;
}