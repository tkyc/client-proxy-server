#include "common.h"

std::atomic<bool> Common::RUNNING{true};

std::shared_ptr<Logger> Common::LOGGER= std::make_shared<Logger>();

std::unordered_map<std::string, Argument> Common::LEGAL_FLAGS = {
    // Common input flags
    {"--listen-ip",   {Flag::ListenIP, &Common::LISTEN_IP}},
    {"--listen-port", {Flag::ListenPort, &Common::LISTEN_PORT}},
    {"--target-ip",   {Flag::TargetIP, &Common::TARGET_IP}},
    {"--target-port", {Flag::TargetPort, &Common::TARGET_PORT}},

    // Client only input flags
    {"--timeout",     {Flag::Timeout, &Common::TIMEOUT}},
    {"--max-retries", {Flag::MaxRetries, &Common::MAX_RETRIES}},

    // Proxy only input flags
    {"--client-drop", {Flag::ClientDrop, &Common::CLIENT_DROP}},
    {"--server-drop", {Flag::ServerDrop, &Common::SERVER_DROP}},
    {"--client-delay", {Flag::ClientDelay, &Common::CLIENT_DELAY}},
    {"--server-delay", {Flag::ServerDelay, &Common::SERVER_DELAY}},
    {"--client-delay-time-min", {Flag::ClientDelayTimeMin, &Common::CLIENT_DELAY_TIME_MIN}},
    {"--client-delay-time-max", {Flag::ClientDelayTimeMax, &Common::CLIENT_DELAY_TIME_MAX}},
    {"--server-delay-time-min", {Flag::ServerDelayTimeMin, &Common::SERVER_DELAY_TIME_MIN}},
    {"--server-delay-time-max", {Flag::ServerDelayTimeMax, &Common::SERVER_DELAY_TIME_MAX}}
};

// Common input flags
std::string Common::LISTEN_IP;
int Common::LISTEN_PORT;
std::string Common::TARGET_IP;
int Common::TARGET_PORT;

// Client only flags
int Common::TIMEOUT;
int Common::MAX_RETRIES;

// Proxy only flags
int Common::CLIENT_DROP;
int Common::CLIENT_DELAY;
int Common::CLIENT_DELAY_TIME_MIN;
int Common::CLIENT_DELAY_TIME_MAX;
int Common::SERVER_DROP;
int Common::SERVER_DELAY;
int Common::SERVER_DELAY_TIME_MIN;
int Common::SERVER_DELAY_TIME_MAX;

