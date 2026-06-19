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

// gets the keyboard handler by 
std::string getKeyboardHandler();

#endif



// store keypress
extern std::atomic<int> KEY;
// for keys that store multiple things on the input buffer at once
extern std::atomic<int> KEY2;
extern std::atomic<int> KEY3;
extern std::atomic<bool> INPUT_THREAD_RUNNING;

// 
void inputHelper();

// 
void setKeyStatesOff(std::unordered_map<Key,bool>& keyStates);

// 
void updateKeyStates_SAFE(std::unordered_map<Key,bool>& keyStates);



// 
void updateKeyStates();

// 
std::string getActiveKeys(std::unordered_map<Key,bool>& keyStates);