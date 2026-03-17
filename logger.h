#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <chrono>
#include <netinet/in.h>
#include <arpa/inet.h>

class Logger {
    private:
        std::string program;
        std::ofstream file;

    public:
        Logger() {};

        void init(const std::string& program, const std::string& path);
        void log(const std::string& event, uint32_t seq, const std::string& extra);
        void log(const std::string& event, const std::string& extra);
        std::string now_ts();
};

