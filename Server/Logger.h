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
        std::string logFileName;//plik z logami
        std::mutex logMutex;//mutex chroni¹cy dostêp do pliku

    public:
        Logger(const std::string& filename) : logFileName(filename) {// Konstruktor  otwierany na nowo przy ka¿dym wpisie w trybie append
        }

        //zapisuje jedn¹ liniê logu timestamp typ wiadomosci nadawce odbiorece tresc wiadomosci
        void log(const std::string& from, const std::string& to,
            const std::string& type, const std::string& text)
        {
            std::lock_guard<std::mutex> lock(logMutex);//auto blokada mutexa

            std::ofstream logFile(logFileName, std::ios::app);
            if (logFile.is_open()) {
                auto now = std::chrono::system_clock::now();//pobieranie czasu do timestampa
                time_t t = std::chrono::system_clock::to_time_t(now);

                tm tm_struct;
                gmtime_s(&tm_struct, &t);

                std::ostringstream oss;
                oss << "[" << std::put_time(&tm_struct, "%Y-%m-%dT%H:%M:%SZ") << "] "//gotowa linia logu
                    << "[" << type << "] from:" << from << " to:" << to
                    << " text:\"" << text << "\"";

                logFile << oss.str() << std::endl;//zapis do pliku

                //wypiasanie na konsole
                std::cout << oss.str() << std::endl;
            }
        }
    };
}