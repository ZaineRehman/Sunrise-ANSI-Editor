#include <iostream>
#include <fstream>
#include <atomic>
#include <unordered_map>

#include "input.hpp"
#include "settings.hpp"

#ifdef _WIN32
#else
	#include <unistd.h>
	#include <linux/input.h>
#endif


#ifdef _WIN32
#else

std::string getKeyboardHandler() {
    const char* searchPath = INPUT_DEVICE_SEARCH_PATH;
    std::ifstream file (searchPath);
    std::string str;

    int block = 0;  // device (delim = \n)
    std::string handler;  // taken from 'Handlers= ... event#'

    while (std::getline(file, str)) {
        //std::cout << str << '\n';
        if (!str.size()) block++;
        size_t foundEvent = str.find("event");
        size_t foundEV = str.find("EV=");
        if (str[0] == 'H' && foundEvent != std::string::npos) {
            //std::cout << "Handler: [" << str.substr(foundEvent+5, str.size() - (foundEvent+5)-1) << "]\n";
            handler = str.substr(foundEvent+5, str.size() - (foundEvent+5)-1);
        } else if (str[0] == 'B' && foundEV != std::string::npos) {
            //std::cout << "Check: [" << str.substr(foundEV+3, std::string::npos) << "]\n";
            if (
                str.substr(foundEV+3, std::string::npos) == "120013" ||
                str.substr(foundEV+3, std::string::npos) == "100013"
            ) break;
        } else {
            continue;
        }
    }

    return handler;
}

#endif



// store keypress
std::atomic<int> KEY(-1);
// for keys that store multiple things on the input buffer at once
std::atomic<int> KEY2(-1);
std::atomic<int> KEY3(-1);
std::atomic<bool> INPUT_THREAD_RUNNING(true);

void inputHelper() {
	while (INPUT_THREAD_RUNNING) {
		int key = getchar();
		// do i even need this check though
		if (key != EOF) KEY = key;
		
		// idc it works and it works fine
		// checks for keyboard inputs that put multiple things on the input buffer
		// (esc, F1, arrow keys, etc)
		// still have yet to crack the block of keys with insert, home, pgup
		if (key == 27) KEY2 = getchar();
		if (KEY2 == 91 || KEY2 == 79) KEY3 = getchar();
	}
	// should never get here so ill have fun with random code if i wanna
	// non letter key = 27
	//__asm("nop");
	return;
}

void updateKeyStates_SAFE(std::unordered_map<Key,bool>& keyStates) {
	if (KEY != -1) {
		switch (KEY) {
		case 27:
			switch (KEY2) {
			case 27:
				// escape
				keyStates[Key::ESC] = true;
				break;
				
			case 91:
				// up down right left
				switch (KEY3) {
				case 65:
					keyStates[Key::UP] = true;
					break;
				case 66:
					keyStates[Key::DOWN] = true;
					break;
				case 67:
					keyStates[Key::RIGHT] = true;
					break;
				case 68:
					keyStates[Key::LEFT] = true;
					break;
				}
				break;
			case 79:
				switch (KEY3) {
				case 80:
					keyStates[Key::F1] = true;
					break;
				case 81:
					keyStates[Key::F2] = true;
					break;
				case 82:
					keyStates[Key::F3] = true;
					break;
				case 83:
					keyStates[Key::F4] = true;
					break;
				}
				break;
			}
			break;
		case 119:
			keyStates[Key::W] = true;
			break;
		case 97:
			keyStates[Key::A] = true;
			break;
		case 115:
			keyStates[Key::S] = true;
			break;
		case 100:
			keyStates[Key::D] = true;
			break;
		}
		
		KEY = -1;
		KEY2 = -1;
		KEY3 = -1;
	}
}



void setKeyStatesOff(std::unordered_map<Key,bool>& keyStates) {
	for (const auto& [key, val] : keyStates) {
		keyStates[key] = false;
	}
}

void updateKeyStates(std::unordered_map<Key,bool>& keyStates, int keyChecker) {
	if (INPUT_SAFE_MODE) return;
	#ifdef _WIN32
		
	#else
		struct input_event event;
		while (read(keyChecker, &event, sizeof(struct input_event)) > 0) {
			std::cout << "ISUGHISUHG\n";
			if (event.type == EV_KEY) {
				std::cout << event.code << " ";
				if (event.value == 0) {
					keyStates[Key::SPACE] = false;
				} else if (event.value == 1) {
					keyStates[Key::SPACE] = true;
				}
				//if (event.value == 0)      std::cout << " -> RELEASED\n";
				//else if (event.value == 1) std::cout << " -> PRESSED\n";
				//else if (event.value == 2) std::cout << " -> SUSTAINED HOLD\n";
			}
		}
	#endif
}

std::string getActiveKeys(std::unordered_map<Key,bool>& keyStates) {
	std::string activeKeys = "";

	if (keyStates[Key::F1]) activeKeys += "[F1] ";
	if (keyStates[Key::F2]) activeKeys += "[F2] ";
	if (keyStates[Key::F3]) activeKeys += "[F3] ";
	if (keyStates[Key::F4]) activeKeys += "[F4] ";

	if (keyStates[Key::SPACE]) activeKeys += "[SPACE] ";
	if (keyStates[Key::ESC]) activeKeys += "[ESC] ";

	if (keyStates[Key::W]) activeKeys += "[W] ";
	if (keyStates[Key::A]) activeKeys += "[A] ";
	if (keyStates[Key::S]) activeKeys += "[S] ";
	if (keyStates[Key::D]) activeKeys += "[D] ";

	return activeKeys;
}