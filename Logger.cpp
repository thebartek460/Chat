#pragma once
#include <string>
#include <mutex>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace ChatShared {

    class Logger {
    private:
        std::string logFileName;
        std::mutex logMutex;

    public:
        // Konstruktor
        Logger(const std::string& filename) : logFileName(filename) {}

        // funkcja do logow
        void log(const std::string& from, const std::string& to,
            const std::string& type, const std::string& text)
        {
            std::lock_guard<std::mutex> lock(logMutex);

            std::ofstream logFile(logFileName, std::ios::app);
            if (logFile.is_open()) {
                auto now = std::chrono::system_clock::now();
                time_t t = std::chrono::system_clock::to_time_t(now);//pobranie czasu

                tm tm_struct;
                gmtime_s(&tm_struct, &t);//konwersja na utc

                std::ostringstream oss;//formatowanie wpisu do logu
                oss << "[" << std::put_time(&tm_struct, "%Y-%m-%dT%H:%M:%SZ") << "] "
                    << "[" << type << "] from:" << from << " to:" << to
                    << " text:\"" << text << "\"";

                logFile << oss.str() << std::endl;//zapis do pliku
            }
        }
    };
}