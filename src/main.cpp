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


std::vector<Cell> artMapping (ART_WIDTH*ART_HEIGHT, Cell{"@", ""});
int artGlobalX = 0, artGlobalY = 0;
int cursorX = SCREEN_WIDTH/2, cursorY = SCREEN_HEIGHT/2;


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


	//  -- LOOP -- 

	int frame = 0;
	int cursorAnim = 0;

	while (RUNNING) {
		std::chrono::time_point<std::chrono::steady_clock> timeStart = std::chrono::steady_clock::now();

		//  -- INPUTS -- 

		setKeyStatesOff(keyStates);
		if (INPUT_SAFE_MODE) updateKeyStates_SAFE(keyStates);
		else updateKeyStates(keyStates, keyChecker);

		if (keyStates[Key::ESC]) RUNNING = false;

		if (keyStates[Key::LEFT])  cursorX--;
		if (keyStates[Key::RIGHT]) cursorX++;
		if (keyStates[Key::UP])    cursorY--;
		if (keyStates[Key::DOWN])  cursorY++;

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
			artGlobalX = SCREEN_WIDTH/2 - ART_WIDTH/2;
			artGlobalY = SCREEN_HEIGHT/2 - ART_HEIGHT/2;
			// cursor as well
			cursorX = artGlobalX + ART_WIDTH/2;
			cursorY = artGlobalY + ART_HEIGHT/2;
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
					if (x == 0) render.put(0, 0, Cell{"╔", ANSI::bold});
					else if (x != SCREEN_WIDTH-1) render.put(x, 0, Cell{"═", ANSI::bold});
					else render.put(x, 0, Cell{"╗", ANSI::bold});
				}
				else if (y == SCREEN_HEIGHT-1) {
					if (x == 0) render.put(0, y, Cell{"╚", ANSI::bold});
					else if (x != SCREEN_WIDTH-1) render.put(x, y, Cell{"═", ANSI::bold});
					else render.put(x, y, Cell{"╝", ANSI::bold});
				}
				else if (x == 0 || x == SCREEN_WIDTH-1) {
					render.put(x, y, Cell{"║", ANSI::bold});
				}
				
				// art
				else {
					if (
						x >= artGlobalX && x < artGlobalX+ART_WIDTH &&
						y >= artGlobalY && y < artGlobalY+ART_HEIGHT
					) {
						// in bounds of art
						render.put(x, y, artMapping[(y-artGlobalY)*ART_WIDTH + (x-artGlobalX)]);
						//render.put(x, y, Cell{"!",""});
					}
				}

				// cursor
				if (x == cursorX && y == cursorY) {
					if (frame && cursorAnim == 2) {
						render.edit(x, y, ANSI::yellow_back_bright);
					}
					if (cursorAnim == 0) cursorAnim = 2;
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