/*
 * Logging
**/
#include "log.hpp"

#include <fstream>
#include <chrono>
#include <filesystem>

#include "art.hpp"
#include "lib.hpp"
#include "file_io.hpp"


void createSessionID() {
    auto utcTimeNow = std::chrono::system_clock::now();
    std::chrono::zoned_time localTimeZone{std::chrono::current_zone(), utcTimeNow};
    auto localTime = localTimeZone.get_local_time();
    
    auto localTime_days = std::chrono::floor<std::chrono::days>(localTime);
    std::chrono::year_month_day ymd{localTime_days};
    std::chrono::hh_mm_ss hms{localTime - localTime_days};

    std::string sessionName = "s" + getTimestamp();

    SESSION_ID = sessionName;
}

void saveArtToSession(const Art& art) {
    auto utcTimeNow = std::chrono::system_clock::now();
    std::chrono::zoned_time localTimeZone{std::chrono::current_zone(), utcTimeNow};
    auto localTime = localTimeZone.get_local_time();
    
    auto localTime_days = std::chrono::floor<std::chrono::days>(localTime);
    std::chrono::year_month_day ymd{localTime_days};
    std::chrono::hh_mm_ss hms{localTime - localTime_days};

    std::string filename = "Sessions/" + SESSION_ID + "/saved_" + getTimestamp() + ".ans";

    loadArtIntoFile(art, filename);
}

void addtoDebugReport(const std::string& str) {
    std::string debugPath = "Sessions/" + SESSION_ID + "/report";
    std::ofstream file(debugPath, std::ios_base::app);

    file << getTimestamp();
    file << ": ";
    file << str;
    file << "\n";
}

void startSession() {
    std::filesystem::create_directory("Sessions/" + SESSION_ID);
    addtoDebugReport("START SESSION");
}