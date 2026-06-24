#pragma once


// if true, uses buffered terminal inputs instead of raw keyboard keystates
inline bool INPUT_SAFE_MODE = true;

// path to search for input devices
inline const char* INPUT_DEVICE_SEARCH_PATH = "/proc/bus/input/devices";

// fps
inline float FPS = 30;


// animation duration for cursor (seconds)
inline float ANIM_CURSOR = 0.5;


// screen width
inline int SCREEN_WIDTH = 20;
// screen height
inline int SCREEN_HEIGHT = 8;

// width of art
inline int ART_WIDTH = 10;
// height of art
inline int ART_HEIGHT = 5;

// if true, ASCII art instead of ANSI art
inline bool ART_IS_ASCII = false;