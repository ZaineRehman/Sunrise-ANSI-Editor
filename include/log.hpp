/*
 * Logging
**/
#pragma once

#include <string>

#include "art.hpp"


inline std::string SESSION_ID = "?";


// creates a session ID
void createSessionID();

// start session folder and report
void startSession();

// save art
void saveArtToSession(const Art& art);

// add to debug report
void addtoDebugReport(const std::string& str);