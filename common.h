#pragma once
#include "packet.h"
#include "logger.h"
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <cstdint>
#include <random>
#include <iostream>
#include <memory>
#include <thread>
#include <csignal>
#include <atomic>
#include <unordered_map>
#include <vector>
#include <queue>

enum class Flag {
    // Common input flags
    ListenIP,
    ListenPort,
    TargetIP,
    TargetPort,

    // Client only input flags
    Timeout,
    MaxRetries,

    // Proxy only input flags
    ClientDrop,
    ClientDelay,
    ClientDelayTimeMin,
    ClientDelayTimeMax,
    ServerDrop,
    ServerDelay,
    ServerDelayTimeMin,
    ServerDelayTimeMax,
    Unknown
};

struct Argument {
    Flag flag;
    void* value;
    Argument(Flag flag, void* value) : flag(flag), value(value) {}
};

class ArgumentException : public std::runtime_error {
    public:
        ArgumentException(const std::string& msg) : std::runtime_error(msg) {}
};

class Common {
    public:
        static std::atomic<bool> RUNNING;
        static std::shared_ptr<Logger> LOGGER;
        static std::unordered_map<std::string, Argument> LEGAL_FLAGS;

        // Common input flags
        static std::string LISTEN_IP;
        static int LISTEN_PORT;
        static std::string TARGET_IP;
        static int TARGET_PORT;

        // Client only flags
        static int TIMEOUT;
        static int MAX_RETRIES;

        // Proxy only flags
        static int CLIENT_DROP;
        static int CLIENT_DELAY;
        static int CLIENT_DELAY_TIME_MIN;
        static int CLIENT_DELAY_TIME_MAX;
        static int SERVER_DROP;
        static int SERVER_DELAY;
        static int SERVER_DELAY_TIME_MIN;
        static int SERVER_DELAY_TIME_MAX;
   
        static void setup_logger(std::string program, std::string logfile) {
            LOGGER->init(program, logfile);
            Packet::setLogger(LOGGER);
        }

        static void setup_signal_handler() {
            struct sigaction sa {};
            sa.sa_handler = handle_sigint;
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = 0;
            sigaction(SIGINT, &sa, nullptr);
        }

        static void parse_args(int argc, char* argv[]) {
            // Start loop at 1 to skip program name
            for (int i = 1; i < argc; i++) {
                std::string arg = argv[i];
        
                // Check if argv[i + 1] is within bounds
                if (i + 1 < argc) {
                    switch (Common::LEGAL_FLAGS.count(arg) ? Common::LEGAL_FLAGS.at(arg).flag : Flag::Unknown) {
                        case Flag::ListenIP:
                            *(static_cast<std::string*>(LEGAL_FLAGS.at(arg).value)) = argv[++i];
                            break;
                        case Flag::TargetIP:
                            *(static_cast<std::string*>(Common::LEGAL_FLAGS.at(arg).value)) = argv[++i];
                            break;
                        case Flag::Unknown:
                            throw ArgumentException("Unknown argument");
                            break;
                        default:
                            *(static_cast<int*>(Common::LEGAL_FLAGS.at(arg).value)) = std::stoi(argv[++i]);
                            break;
                    }
                } else {
                    throw ArgumentException("Too many arguments");
                }
            }
        }

    private:
        static void handle_sigint(int) {
            RUNNING = false;
        }
};

