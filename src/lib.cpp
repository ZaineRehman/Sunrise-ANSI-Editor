/* 
 * Small functions
**/

#include "lib.hpp"

#include <csignal>
#include <string>

#ifdef _WIN32
	#include <windows.h>
#endif

#include "log.hpp"


void signalHandler(int signal) {
	switch (signal) {
		case SIGINT: 
		case SIGTERM: 
		case SIGABRT: 
			RUNNING = false;
			break;
	}
	
	std::string sigstr = "UNKNOWN";
	switch (signal) {
		case SIGINT:  sigstr = "SIGINT";  break;
		case SIGTERM: sigstr = "SIGTERM"; break;
		case SIGABRT: sigstr = "SIGABRT"; break;
	}
	reportLog("!!! signalHandler() recieved signal: " + sigstr);
}

std::string concat(const std::string& str1, const char* str2) {
	return str1 + str2;
}

std::string getTimestamp() {
	auto utcTimeNow = std::chrono::system_clock::now();
    std::chrono::zoned_time localTimeZone{std::chrono::current_zone(), utcTimeNow};
    auto localTime = localTimeZone.get_local_time();
    
    auto localTime_days = std::chrono::floor<std::chrono::days>(localTime);
    std::chrono::year_month_day ymd{localTime_days};
    std::chrono::hh_mm_ss hms{localTime - localTime_days};

	std::string timestamp = "";
    timestamp += std::to_string(static_cast<int>(ymd.year()));
    timestamp += '-';
    timestamp += std::to_string(static_cast<unsigned int>(ymd.month()));
    timestamp += '-';
    timestamp += std::to_string(static_cast<unsigned int>(ymd.day()));
    timestamp += '_';
    timestamp += std::to_string(hms.hours().count());
    timestamp += '-';
    timestamp += std::to_string(hms.minutes().count());
    timestamp += '-';
    timestamp += std::to_string(hms.seconds().count());

	return timestamp;
}

std::string limitString(const std::string& str, size_t limit) {
	if (str.size() <= limit) return str;

	return "..." + str.substr(str.size()-1-limit, limit-3);
}


#ifdef _WIN32
	bool windowIsFocused(HWND handle) {
		return GetForegroundWindow() == handle;
	}
#endif