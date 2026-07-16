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

void safeModeInputHelper() {
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


struct KeyMap {
	int code;
	Key key;
};

static constexpr KeyMap keyMappings[] = {
	{'A', Key::A},  {'B', Key::B},  {'C', Key::C},  {'D', Key::D},  {'E', Key::E},  {'F', Key::F}, 
	{'G', Key::G},  {'H', Key::H},  {'I', Key::I},  {'J', Key::J},  {'K', Key::K},  {'L', Key::L}, 
	{'M', Key::M},  {'N', Key::N},  {'O', Key::O},  {'P', Key::P},  {'Q', Key::Q},  {'R', Key::R}, 
	{'S', Key::S},  {'T', Key::T},  {'U', Key::U},  {'V', Key::V},  {'W', Key::W},  {'X', Key::X}, 
	{'Y', Key::Y},  {'Z', Key::Z},  {'1', Key::_1}, {'2', Key::_2}, {'3', Key::_3}, {'4', Key::_4}, 
	{'5', Key::_5}, {'6', Key::_6}, {'7', Key::_7}, {'8', Key::_8}, {'9', Key::_9}, {'0', Key::_0}, 
	{VK_LBUTTON , Key::L_MOUSE}, {VK_RBUTTON , Key::R_MOUSE},  {VK_MBUTTON , Key::M_MOUSE},  {VK_XBUTTON1, Key::B1_MOUSE}, {VK_XBUTTON2, Key::B2_MOUSE}, {VK_SPACE, Key::SPACE}, 
	{VK_LSHIFT  , Key::SHIFT},   {VK_LCONTROL, Key::CTRL},     {VK_LMENU   , Key::ALT},      {VK_RSHIFT  , Key::R_SHIFT},  {VK_RCONTROL, Key::R_CTRL},   {VK_RMENU, Key::R_ALT}, 
	{VK_TAB     , Key::TAB},     {VK_BACK    , Key::BACKSP},   {VK_CAPITAL , Key::CAPSLOCK}, {VK_ESCAPE  , Key::ESC},      {VK_LWIN    , Key::META},     {VK_PAUSE, Key::PAUSE}, 
	{VK_PRINT   , Key::PRINT},   {VK_SNAPSHOT, Key::PRINTSCR}, {VK_PRIOR   , Key::PGUP},     {VK_NEXT    , Key::PGDOWN},   {VK_END     , Key::END},      {VK_HOME, Key::HOME}, 
	{VK_INSERT  , Key::INSERT},  {VK_DELETE  , Key::DEL},      {VK_UP      , Key::UP},       {VK_DOWN    , Key::DOWN},     {VK_LEFT    , Key::LEFT},     {VK_RIGHT, Key::RIGHT}, 
	{VK_NUMPAD0 , Key::KP_0},    {VK_NUMPAD1 , Key::KP_1},     {VK_NUMPAD2 , Key::KP_2},     {VK_NUMPAD3 , Key::KP_3},     {VK_NUMPAD4 , Key::KP_4}, 
	{VK_NUMPAD5 , Key::KP_5},    {VK_NUMPAD6 , Key::KP_6},     {VK_NUMPAD7 , Key::KP_7},     {VK_NUMPAD8 , Key::KP_8},     {VK_NUMPAD9 , Key::KP_9}, 
	{VK_MULTIPLY, Key::KP_MUL},  {VK_ADD     , Key::KP_PLUS},  {VK_SUBTRACT, Key::KP_MIN},   {VK_DECIMAL , Key::KP_PER},   {VK_DIVIDE  , Key::KP_DIV}, 
	{VK_NUMLOCK , Key::NUMLOCK}, {VK_SCROLL  , Key::SCRLOCK}, 
	{VK_F1, Key::F1}, {VK_F2, Key::F2}, {VK_F3, Key::F3}, {VK_F4 , Key::F4},  {VK_F5 , Key::F5},  {VK_F6 , Key::F6}, 
	{VK_F7, Key::F7}, {VK_F8, Key::F8}, {VK_F9, Key::F9}, {VK_F10, Key::F10}, {VK_F11, Key::F11}, {VK_F12, Key::F12}
};

void thread_doKeyStates(KeyStates& keyStates, int keyChecker) {
	while (1) updateKeyStates(keyStates, keyChecker);
}

void updateKeyStates(KeyStates& keyStates, int keyChecker) {
	#ifdef _WIN32
		BYTE keyboardState[256];

		if (GetKeyboardState(keyboardState)) {
			for (int i = 0; i < 256; ++i) {
				std::cout << (int)keyboardState[i] << " ";
			} std::cout << "[done]" << std::endl;
			for (const KeyMap& m : keyMappings) {
				keyStates.set(m.key, (keyboardState[m.code] & 0x80) != 0);
			}
		}

		// (most!) legacy keys
		//  ^^^^^ because i still check for some for some reason
		//   VK_SELECT  VK_SEPARATOR  VK_EXECUTE  VK_HELP  VK_SLEEP  VK_KANA
		//   VK_HANGUL  VK_IME_ON  VK_JUNJA  VK_FINAL  VK_HANJA  VK_KANJI
		//   VK_IME_OFF  VK_CONVERT  VK_NONCONVERT  VK_ACCEPT  VK_MODECHANGE  VK_LWIN
		//   VK_APPS  VK_F13  VK_F14  VK_F15  VK_F16  VK_F17
		//   VK_F18  VK_F19  VK_F20  VK_F21  VK_F22  VK_F23
		//   VK_F24  VK_BROWSER_BACK  VK_BROWSER_FORWARD  VK_BROWSER_REFRESH  VK_BROWSER_STOP  VK_BROWSER_SEARCH
		//   VK_BROWSER_FAVORITES  VK_BROWSER_HOME  VK_VOLUME_MUTE  VK_VOLUME_DOWN  VK_VOLUME_UP  VK_MEDIA_NEXT_TRACK
		//   VK_MEDIA_PREV_TRACK  VK_MEDIA_STOP  VK_MEDIA_PLAY_PAUSE  VK_LAUNCH_MAIL  VK_LAUNCH_MEDIA_SELECT  VK_LAUNCH_APP1
		//   VK_LAUNCH_APP2  VK_OEM_1  VK_OEM_PLUS  VK_OEM_COMMA  VK_OEM_MINUS  VK_OEM_PERIOD
		//   VK_OEM_2  VK_OEM_3  VK_GAMEPAD_A  VK_GAMEPAD_B  VK_GAMEPAD_X  VK_GAMEPAD_Y
		//   VK_GAMEPAD_RIGHT_SHOULDER  VK_GAMEPAD_LEFT_SHOULDER  VK_GAMEPAD_LEFT_TRIGGER  VK_GAMEPAD_RIGHT_TRIGGER  VK_GAMEPAD_DPAD_UP  VK_GAMEPAD_DPAD_DOWN
		//   VK_GAMEPAD_DPAD_LEFT  VK_GAMEPAD_DPAD_RIGHT  VK_GAMEPAD_MENU  VK_GAMEPAD_VIEW  VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON  VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON
		//   VK_GAMEPAD_LEFT_THUMBSTICK_UP  VK_GAMEPAD_LEFT_THUMBSTICK_DOWN  VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT  
		//   VK_GAMEPAD_LEFT_THUMBSTICK_LEFT  VK_GAMEPAD_RIGHT_THUMBSTICK_UP  VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN
		//   VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT  VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT  VK_OEM_4  VK_OEM_5  VK_OEM_6  VK_OEM_7
		//   VK_OEM_8  VK_OEM_102  VK_PROCESSKEY  VK_PACKET  VK_ATTN  VK_CRSEL
		//   VK_EXSEL  VK_EREOF  VK_PLAY  VK_ZOOM  VK_NONAME  VK_PA1  VK_OEM_CLEAR

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