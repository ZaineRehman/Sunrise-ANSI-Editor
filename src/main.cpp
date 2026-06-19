#include <iostream>
#include <atomic>
#include <csignal>

#include "input.hpp"
#include "output.hpp"
#include "settings.hpp"



std::unordered_map<Key,bool> keyStates;

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



int main() {

	//  -- SETUP -- 

	srand(time(0));
	Renderer render {20, 8};

	// terminal raw mode
	#ifdef _WIN32
		#ifndef INPUT_SAFE_MODE

		#else
			std::thread(inputHelper).detach();
		#endif
	#else
		system("setterm -cursor off");
	
		// set terminal to raw mode and turn off echo
		// most of these are the same as doing "raw" just without -opost because that messes shit up
		// also allowing 'isig' for ctrl+C
		system("stty -ignbrk -brkint -ignpar -inlcr -icanon -ixoff -igncr -icrnl -parmrk -inpck -istrip -ixon isig -iuclc -ixany -imaxbel -xcase min 1 time 0 -echo");
		
		#ifndef INPUT_SAFE_MODE
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
		#else
			std::thread(inputHelper).detach();
		#endif
	#endif

	clear();


	//  -- LOOP --

	while (RUNNING) {
		

		updateKeyStates();
		if (INPUT_SAFE_MODE) updateKeyStates_SAFE(keyStates);

		if (keyStates[Key::ESC]) RUNNING = false;
	}


	//  -- CLEAN -- 

	std::cout << ANSI::reset << ANSI::cursor_visible << std::flush;

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