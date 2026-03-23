#pragma once
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <chrono>

class Logger {
    private:
        std::string program;
        std::ofstream file;

    public:
        Logger() {};

        void init(const std::string& program, const std::string& path);
        void log(const std::string& event, int32_t seq, const std::string& extra);
        void log(const std::string& event, const std::string& extra);
        std::string now_ts();
};

