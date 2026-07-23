/* 
 * Small functions
**/

#include <csignal>
#include <string>

#include "lib.hpp"

void signalHandler(int signal) {
	switch (signal) {
		case SIGINT: 
		case SIGTERM: 
		case SIGABRT: 
			RUNNING = false;
			break;
	}
}

std::string concat(const std::string& str1, const char* str2) {
	return str1 + str2;
}