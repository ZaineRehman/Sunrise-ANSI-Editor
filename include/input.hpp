#pragma once

#include <iostream>
#include <atomic>
#include <array>

#include "settings.hpp"

enum class Key {
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, 
	SPACE, ESC, SHIFT, CTRL, ALT, R_ALT, R_CTRL, R_SHIFT, BACKSP, 
	INSERT, HOME, PGUP, PGDOWN, END, DEL, 
	Q,W,E,R,T,Y,U,I,O,P,A,S,D,F,G,H,J,K,L,Z,X,C,V,B,N,M, 
	_1,_2,_3,_4,_5,_6,_7,_8,_9,_0, 
	TAB, TICK, DASH, EQUALS, LBRACKET, RBRACKET, BSLASH, SEMICOLON, APOSTROPHE, COMMA, PERIOD, SLASH, CAPSLOCK, 
	PAUSE, PRINT, PRINTSCR, NUMLOCK, SCRLOCK, META, 
	UP, DOWN, LEFT, RIGHT, 
	KP_1,KP_2,KP_3,KP_4,KP_5,KP_6,KP_7,KP_8,KP_9,KP_0, 
	KP_DIV, KP_MUL, KP_MIN, KP_PLUS, KP_ENT, KP_PER, 
	L_MOUSE, R_MOUSE, M_MOUSE, B1_MOUSE, B2_MOUSE, 

	_KEY_COUNT
};


class KeyStates {
public: 
	const size_t size = static_cast<size_t>(Key::_KEY_COUNT);
	std::array<std::atomic<bool>, static_cast<size_t>(Key::_KEY_COUNT)> internal {};

	KeyStates() {}

	class _KeyStatesProxy {
	public: 
		std::atomic<bool>& ref;

		explicit _KeyStatesProxy(std::atomic<bool>& _ref) : ref(_ref) {}

		_KeyStatesProxy& operator=(bool val) {
			ref.store(val, std::memory_order_relaxed);
			return *this;
		}

		operator bool() const {
			return ref.load(std::memory_order_relaxed);
		}

		// dont do this
		_KeyStatesProxy(const _KeyStatesProxy&) = delete;
		_KeyStatesProxy& operator=(const _KeyStatesProxy&) = delete;
	};

	void set(Key key, bool val);
	bool get(Key key) const;

	void toggle(Key key);

	_KeyStatesProxy operator[](Key key);
	_KeyStatesProxy operator[](size_t k);

	bool operator[](Key key) const;
	bool operator[](size_t k) const;
};


#ifdef _WIN32



#else

// gets the keyboard handler by searching through INPUT_DEVICE_SEARCH_PATH until it finds the first keyboard device
std::string getKeyboardHandler();

#endif



// store keypress
extern std::atomic<int> KEY;
// for keys that store multiple things on the input buffer at once
extern std::atomic<int> KEY2;
extern std::atomic<int> KEY3;
extern std::atomic<bool> INPUT_THREAD_RUNNING;

// input function for when INPUT_SAFE_MODE is true
// supposed to be threaded but ig you dont have to
void safeModeInputHelper();



// you will never fucking guess what this does
void setKeyStatesOff(KeyStates& keyStates);

// INPUT_SAFE_MODE version of updateKeyStates
void updateKeyStates_SAFE(KeyStates& keyStates);


// loops updateKeyStates
void thread_doKeyStates(KeyStates& keyStates, int keyChecker);

// updates the key states, $0.34 is charged to your bank account for each function call
void updateKeyStates(KeyStates& keyStates, int keyChecker);

// for debugging, call it and find out what it does
//std::string getActiveKeys(std::array<std::atomic<bool>>& keyStates);