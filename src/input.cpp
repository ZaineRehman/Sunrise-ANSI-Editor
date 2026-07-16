#include <iostream>
#include <fstream>
#include <atomic>
#include <array>

#include "input.hpp"
#include "settings.hpp"

#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <linux/input.h>
#endif


void KeyStates::set(Key key, bool val) {
	internal[static_cast<size_t>(key)].store(val, std::memory_order_relaxed);
}
bool KeyStates::get(Key key) const {
	return internal[static_cast<size_t>(key)].load(std::memory_order_relaxed);;
}

void KeyStates::toggle(Key key) {
	set(key, !get(key));
}

KeyStates::_KeyStatesProxy KeyStates::operator[](Key key) {
	return _KeyStatesProxy{internal[static_cast<size_t>(key)]};
}
KeyStates::_KeyStatesProxy KeyStates::operator[](size_t k) {
	return _KeyStatesProxy{internal[k]};
}

bool KeyStates::operator[](Key key) const {
	return internal[static_cast<size_t>(key)].load(std::memory_order_relaxed);
}
bool KeyStates::operator[](size_t k) const {
	return internal[k].load(std::memory_order_relaxed);
}


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

void updateKeyStates_SAFE(KeyStates& keyStates) {
	if (KEY != -1) {
		switch (KEY) {
			case 32: keyStates[Key::SPACE]   = true; break;
			//case 42: keyStates[Key::KP_MUL]  = true; break;
			//case 43: keyStates[Key::KP_PLUS] = true; break;
			case 44: keyStates[Key::COMMA]   = true; break;
			case 45: keyStates[Key::DASH]    = true; break;
			case 46: keyStates[Key::PERIOD]  = true; break;
			case 47: keyStates[Key::SLASH]   = true; break;

			case 48: keyStates[Key::_0] = true; break;
			case 49: keyStates[Key::_1] = true; break;
			case 50: keyStates[Key::_2] = true; break;
			case 51: keyStates[Key::_3] = true; break;
			case 52: keyStates[Key::_4] = true; break;
			case 53: keyStates[Key::_5] = true; break;
			case 54: keyStates[Key::_6] = true; break;
			case 55: keyStates[Key::_7] = true; break;
			case 56: keyStates[Key::_8] = true; break;
			case 57: keyStates[Key::_9] = true; break;

			case 59: keyStates[Key::SEMICOLON] = true; break;
			case 61: keyStates[Key::EQUALS]    = true; break;
			case 91: keyStates[Key::LBRACKET]  = true; break;
			case 92: keyStates[Key::BSLASH]    = true; break;
			case 93: keyStates[Key::RBRACKET]  = true; break;
			case 96: keyStates[Key::TICK]      = true; break;
			
			case 97:  keyStates[Key::A] = true; break;
			case 98:  keyStates[Key::B] = true; break;
			case 99:  keyStates[Key::C] = true; break;
			case 100: keyStates[Key::D] = true; break;
			case 101: keyStates[Key::E] = true; break;
			case 102: keyStates[Key::F] = true; break;
			case 103: keyStates[Key::G] = true; break;
			case 104: keyStates[Key::H] = true; break;
			case 105: keyStates[Key::I] = true; break;
			case 106: keyStates[Key::J] = true; break;
			case 107: keyStates[Key::K] = true; break;
			case 108: keyStates[Key::L] = true; break;
			case 109: keyStates[Key::M] = true; break;
			case 110: keyStates[Key::N] = true; break;
			case 111: keyStates[Key::O] = true; break;
			case 112: keyStates[Key::P] = true; break;
			case 113: keyStates[Key::Q] = true; break;
			case 114: keyStates[Key::R] = true; break;
			case 115: keyStates[Key::S] = true; break;
			case 116: keyStates[Key::T] = true; break;
			case 117: keyStates[Key::U] = true; break;
			case 118: keyStates[Key::V] = true; break;
			case 119: keyStates[Key::W] = true; break;
			case 120: keyStates[Key::X] = true; break;
			case 121: keyStates[Key::Y] = true; break;
			case 122: keyStates[Key::Z] = true; break;

			case 27:
				switch (KEY2) {
					case 27: keyStates[Key::ESC] = true; break;
					
					case 91:
						switch (KEY3) {
							case 65: keyStates[Key::UP]    = true; break;
							case 66: keyStates[Key::DOWN]  = true; break;
							case 67: keyStates[Key::RIGHT] = true; break;
							case 68: keyStates[Key::LEFT]  = true; break;
						}
						break;
				case 79:
					switch (KEY3) {
							case 80: keyStates[Key::F1] = true; break;
							case 81: keyStates[Key::F2] = true; break;
							case 82: keyStates[Key::F3] = true; break;
							case 83: keyStates[Key::F4] = true; break;
						}
						break;
				}
				break;
		}
		
		KEY = -1;
		KEY2 = -1;
		KEY3 = -1;
	}
}


void setKeyStatesOff(KeyStates& keyStates) {
	for (size_t i = 0; i < static_cast<size_t>(Key::_KEY_COUNT); ++i) {
		keyStates[i] = false;
	}
}

void updateKeyStates(KeyStates& keyStates, int keyChecker) {
	#ifdef _WIN32
		if (GetAsyncKeyState('A') & 0x8000) keyStates[Key::A] = true;
		if (GetAsyncKeyState('B') & 0x8000) keyStates[Key::B] = true;
		if (GetAsyncKeyState('C') & 0x8000) keyStates[Key::C] = true;
		if (GetAsyncKeyState('D') & 0x8000) keyStates[Key::D] = true;
		if (GetAsyncKeyState('E') & 0x8000) keyStates[Key::E] = true;
		if (GetAsyncKeyState('F') & 0x8000) keyStates[Key::F] = true;
		if (GetAsyncKeyState('G') & 0x8000) keyStates[Key::G] = true;
		if (GetAsyncKeyState('H') & 0x8000) keyStates[Key::H] = true;
		if (GetAsyncKeyState('I') & 0x8000) keyStates[Key::I] = true;
		if (GetAsyncKeyState('J') & 0x8000) keyStates[Key::J] = true;
		if (GetAsyncKeyState('K') & 0x8000) keyStates[Key::K] = true;
		if (GetAsyncKeyState('L') & 0x8000) keyStates[Key::L] = true;
		if (GetAsyncKeyState('M') & 0x8000) keyStates[Key::M] = true;
		if (GetAsyncKeyState('N') & 0x8000) keyStates[Key::N] = true;
		if (GetAsyncKeyState('O') & 0x8000) keyStates[Key::O] = true;
		if (GetAsyncKeyState('P') & 0x8000) keyStates[Key::P] = true;
		if (GetAsyncKeyState('Q') & 0x8000) keyStates[Key::Q] = true;
		if (GetAsyncKeyState('R') & 0x8000) keyStates[Key::R] = true;
		if (GetAsyncKeyState('S') & 0x8000) keyStates[Key::S] = true;
		if (GetAsyncKeyState('T') & 0x8000) keyStates[Key::T] = true;
		if (GetAsyncKeyState('U') & 0x8000) keyStates[Key::U] = true;
		if (GetAsyncKeyState('V') & 0x8000) keyStates[Key::V] = true;
		if (GetAsyncKeyState('W') & 0x8000) keyStates[Key::W] = true;
		if (GetAsyncKeyState('X') & 0x8000) keyStates[Key::X] = true;
		if (GetAsyncKeyState('Y') & 0x8000) keyStates[Key::Y] = true;
		if (GetAsyncKeyState('Z') & 0x8000) keyStates[Key::Z] = true;
		
		if (GetAsyncKeyState('1') & 0x8000) keyStates[Key::_1] = true;
		if (GetAsyncKeyState('2') & 0x8000) keyStates[Key::_2] = true;
		if (GetAsyncKeyState('3') & 0x8000) keyStates[Key::_3] = true;
		if (GetAsyncKeyState('4') & 0x8000) keyStates[Key::_4] = true;
		if (GetAsyncKeyState('5') & 0x8000) keyStates[Key::_5] = true;
		if (GetAsyncKeyState('6') & 0x8000) keyStates[Key::_6] = true;
		if (GetAsyncKeyState('7') & 0x8000) keyStates[Key::_7] = true;
		if (GetAsyncKeyState('8') & 0x8000) keyStates[Key::_8] = true;
		if (GetAsyncKeyState('9') & 0x8000) keyStates[Key::_9] = true;
		if (GetAsyncKeyState('0') & 0x8000) keyStates[Key::_0] = true;

		if (GetAsyncKeyState(VK_LBUTTON)  & 0x8000) keyStates[Key::L_MOUSE]  = true;
		if (GetAsyncKeyState(VK_RBUTTON)  & 0x8000) keyStates[Key::R_MOUSE]  = true;
		if (GetAsyncKeyState(VK_MBUTTON)  & 0x8000) keyStates[Key::M_MOUSE]  = true;
		if (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) keyStates[Key::B1_MOUSE] = true;
		if (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) keyStates[Key::B2_MOUSE] = true;

		if (GetAsyncKeyState(VK_BACK)     & 0x8000) keyStates[Key::BACKSP]   = true;
		if (GetAsyncKeyState(VK_TAB)      & 0x8000) keyStates[Key::TAB]      = true;
		if (GetAsyncKeyState(VK_LSHIFT)   & 0x8000) keyStates[Key::SHIFT]    = true;
		if (GetAsyncKeyState(VK_LCONTROL) & 0x8000) keyStates[Key::CTRL]     = true;
		if (GetAsyncKeyState(VK_LMENU)    & 0x8000) keyStates[Key::ALT]      = true;
		if (GetAsyncKeyState(VK_RSHIFT)   & 0x8000) keyStates[Key::R_SHIFT]  = true;
		if (GetAsyncKeyState(VK_RCONTROL) & 0x8000) keyStates[Key::R_CTRL]   = true;
		if (GetAsyncKeyState(VK_RMENU)    & 0x8000) keyStates[Key::R_ALT]    = true;
		if (GetAsyncKeyState(VK_CAPITAL)  & 0x8000) keyStates[Key::CAPSLOCK] = true;
		if (GetAsyncKeyState(VK_ESCAPE)   & 0x8000) keyStates[Key::ESC]      = true;
		if (GetAsyncKeyState(VK_SPACE)    & 0x8000) keyStates[Key::SPACE]    = true;
		if (GetAsyncKeyState(VK_LWIN)     & 0x8000) keyStates[Key::META]     = true;

		if (GetAsyncKeyState(VK_PAUSE)    & 0x8000) keyStates[Key::PAUSE]    = true;
		if (GetAsyncKeyState(VK_PRINT)    & 0x8000) keyStates[Key::PRINT]    = true;
		if (GetAsyncKeyState(VK_SNAPSHOT) & 0x8000) keyStates[Key::PRINTSCR] = true;
		
		if (GetAsyncKeyState(VK_PRIOR)  & 0x8000) keyStates[Key::PGUP]   = true;
		if (GetAsyncKeyState(VK_NEXT)   & 0x8000) keyStates[Key::PGDOWN] = true;
		if (GetAsyncKeyState(VK_END)    & 0x8000) keyStates[Key::END]    = true;
		if (GetAsyncKeyState(VK_HOME)   & 0x8000) keyStates[Key::HOME]   = true;
		if (GetAsyncKeyState(VK_INSERT) & 0x8000) keyStates[Key::INSERT] = true;
		if (GetAsyncKeyState(VK_DELETE) & 0x8000) keyStates[Key::DEL]    = true;

		if (GetAsyncKeyState(VK_UP)    & 0x8000) keyStates[Key::UP]    = true;
		if (GetAsyncKeyState(VK_DOWN)  & 0x8000) keyStates[Key::DOWN]  = true;
		if (GetAsyncKeyState(VK_LEFT)  & 0x8000) keyStates[Key::LEFT]  = true;
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000) keyStates[Key::RIGHT] = true;

		if (GetAsyncKeyState(VK_NUMPAD0) & 0x8000) keyStates[Key::KP_0] = true;
		if (GetAsyncKeyState(VK_NUMPAD1) & 0x8000) keyStates[Key::KP_1] = true;
		if (GetAsyncKeyState(VK_NUMPAD2) & 0x8000) keyStates[Key::KP_2] = true;
		if (GetAsyncKeyState(VK_NUMPAD3) & 0x8000) keyStates[Key::KP_3] = true;
		if (GetAsyncKeyState(VK_NUMPAD4) & 0x8000) keyStates[Key::KP_4] = true;
		if (GetAsyncKeyState(VK_NUMPAD5) & 0x8000) keyStates[Key::KP_5] = true;
		if (GetAsyncKeyState(VK_NUMPAD6) & 0x8000) keyStates[Key::KP_6] = true;
		if (GetAsyncKeyState(VK_NUMPAD7) & 0x8000) keyStates[Key::KP_7] = true;
		if (GetAsyncKeyState(VK_NUMPAD8) & 0x8000) keyStates[Key::KP_8] = true;
		if (GetAsyncKeyState(VK_NUMPAD9) & 0x8000) keyStates[Key::KP_9] = true;

		if (GetAsyncKeyState(VK_MULTIPLY)  & 0x8000) keyStates[Key::KP_MUL]  = true;
		if (GetAsyncKeyState(VK_ADD)       & 0x8000) keyStates[Key::KP_PLUS] = true;
		if (GetAsyncKeyState(VK_SUBTRACT)  & 0x8000) keyStates[Key::KP_MIN]  = true;
		if (GetAsyncKeyState(VK_DECIMAL)   & 0x8000) keyStates[Key::KP_PER]  = true;
		if (GetAsyncKeyState(VK_DIVIDE)    & 0x8000) keyStates[Key::KP_DIV]  = true;
		
		if (GetAsyncKeyState(VK_F1)  & 0x8000) keyStates[Key::F1]  = true;
		if (GetAsyncKeyState(VK_F2)  & 0x8000) keyStates[Key::F2]  = true;
		if (GetAsyncKeyState(VK_F3)  & 0x8000) keyStates[Key::F3]  = true;
		if (GetAsyncKeyState(VK_F4)  & 0x8000) keyStates[Key::F4]  = true;
		if (GetAsyncKeyState(VK_F5)  & 0x8000) keyStates[Key::F5]  = true;
		if (GetAsyncKeyState(VK_F6)  & 0x8000) keyStates[Key::F6]  = true;
		if (GetAsyncKeyState(VK_F7)  & 0x8000) keyStates[Key::F7]  = true;
		if (GetAsyncKeyState(VK_F8)  & 0x8000) keyStates[Key::F8]  = true;
		if (GetAsyncKeyState(VK_F9)  & 0x8000) keyStates[Key::F9]  = true;
		if (GetAsyncKeyState(VK_F10) & 0x8000) keyStates[Key::F10] = true;
		if (GetAsyncKeyState(VK_F11) & 0x8000) keyStates[Key::F11] = true;
		if (GetAsyncKeyState(VK_F12) & 0x8000) keyStates[Key::F12] = true;
		
		if (GetAsyncKeyState(VK_NUMLOCK) & 0x8000) keyStates[Key::NUMLOCK] = true;
		if (GetAsyncKeyState(VK_SCROLL)  & 0x8000) keyStates[Key::SCRLOCK] = true;

		// (most!) legacy keys
		//  ^^^^^ because i still check for some for some reason
		//if (GetAsyncKeyState(VK_SELECT) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_SEPARATOR) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_EXECUTE) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_HELP) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_SLEEP)  & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_KANA) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_HANGUL) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_IME_ON) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_JUNJA) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_FINAL) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_HANJA) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_KANJI) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_IME_OFF) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_CONVERT) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_NONCONVERT) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_ACCEPT) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_MODECHANGE) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_LWIN) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_APPS) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_F13) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_F14) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_F15) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_F16) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_F17) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_F18) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_F19) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_F20) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_F21) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_F22) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_F23) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_F24) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_BROWSER_BACK) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_BROWSER_FORWARD) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_BROWSER_REFRESH) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_BROWSER_STOP) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_BROWSER_SEARCH) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_BROWSER_FAVORITES) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_BROWSER_HOME) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_VOLUME_MUTE) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_VOLUME_DOWN) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_VOLUME_UP) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_MEDIA_NEXT_TRACK) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_MEDIA_PREV_TRACK) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_MEDIA_STOP) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_MEDIA_PLAY_PAUSE) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_LAUNCH_MAIL) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_LAUNCH_MEDIA_SELECT) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_LAUNCH_APP1) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_LAUNCH_APP2) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_OEM_1) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_OEM_PLUS) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_OEM_COMMA) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_OEM_MINUS) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_OEM_PERIOD) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_OEM_2) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_OEM_3) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_A) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_B) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_X) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_Y) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_RIGHT_SHOULDER) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_LEFT_SHOULDER) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_LEFT_TRIGGER) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_RIGHT_TRIGGER) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_DPAD_UP) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_DPAD_DOWN) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_DPAD_LEFT) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_DPAD_RIGHT) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_MENU) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_VIEW) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_LEFT_THUMBSTICK_UP) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_LEFT_THUMBSTICK_DOWN) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_LEFT_THUMBSTICK_LEFT) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_RIGHT_THUMBSTICK_UP) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_OEM_4) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_OEM_5) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_OEM_6) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_OEM_7) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_OEM_8) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_OEM_102) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_PROCESSKEY) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_PACKET) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_ATTN) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_CRSEL) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_EXSEL) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_EREOF) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_PLAY) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_ZOOM) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_NONAME) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_PA1) & 0x8000) keyStates[Key::] = true;
		//if (GetAsyncKeyState(VK_OEM_CLEAR) & 0x8000) keyStates[Key::] = true;

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

/*std::string getActiveKeys(std::unordered_map<Key,bool>& keyStates) {
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
}*/