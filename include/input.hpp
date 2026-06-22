#pragma once

#include <iostream>
#include <unordered_map>

#include "settings.hpp"

enum class Key {
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, 
	SPACE, ESC, SHIFT, CTRL, ALT, R_ALT, R_CTRL, R_SHIFT, BACK, 
	INSERT, HOME, PGUP, PGDOWN, END, DEL, 
	Q,W,E,R,T,Y,U,I,O,P,A,S,D,F,G,H,J,K,L,Z,X,C,V,B,N,M, 
	_1,_2,_3,_4,_5,_6,_7,_8,_9,_0, 
	UP, DOWN, LEFT, RIGHT
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
void inputHelper();

// you will never fucking guess what this does
void setKeyStatesOff(std::unordered_map<Key,bool>& keyStates);

// INPUT_SAFE_MODE version of updateKeyStates
void updateKeyStates_SAFE(std::unordered_map<Key,bool>& keyStates);



// updates the key states, $0.34 is charged to your bank account for each function call
void updateKeyStates(std::unordered_map<Key,bool>& keyStates, int keyChecker);

// for debugging, call it and find out what it does
std::string getActiveKeys(std::unordered_map<Key,bool>& keyStates);