#include "../packet.h"
#include "../logger.h"
#include <iostream>
#include <unordered_map>
#include <sys/socket.h>
#include <unistd.h>
#include <queue>
#include <random>
#include <chrono>
#include <thread>

static std::string LISTEN_IP;
static int LISTEN_PORT;
static std::string TARGET_IP;
static int TARGET_PORT;
static int CLIENT_DROP;
static int CLIENT_DELAY;
static int CLIENT_DELAY_TIME_MIN;
static int CLIENT_DELAY_TIME_MAX;
static int SERVER_DROP;
static int SERVER_DELAY;
static int SERVER_DELAY_TIME_MIN;
static int SERVER_DELAY_TIME_MAX;

enum class Flag {
    ListenIP,
    ListenPort,
    TargetIP,
    TargetPort,
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

static const std::unordered_map<std::string, Argument> LEGAL_FLAGS = {
    {"--listen-ip",   {Flag::ListenIP, &LISTEN_IP}},
    {"--listen-port", {Flag::ListenPort, &LISTEN_PORT}},
    {"--target-ip",   {Flag::TargetIP, &TARGET_IP}},
    {"--target-port", {Flag::TargetPort, &TARGET_PORT}},
    {"--client-drop", {Flag::ClientDrop, &CLIENT_DROP}},
    {"--server-drop", {Flag::ServerDrop, &SERVER_DROP}},
    {"--client-delay", {Flag::ClientDelay, &CLIENT_DELAY}},
    {"--server-delay", {Flag::ServerDelay, &SERVER_DELAY}},
    {"--client-delay-time-min", {Flag::ClientDelayTimeMin, &CLIENT_DELAY_TIME_MIN}},
    {"--client-delay-time-max", {Flag::ClientDelayTimeMax, &CLIENT_DELAY_TIME_MAX}},
    {"--server-delay-time-min", {Flag::ServerDelayTimeMin, &SERVER_DELAY_TIME_MIN}},
    {"--server-delay-time-max", {Flag::ServerDelayTimeMax, &SERVER_DELAY_TIME_MAX}}
};

void parse_args(int argc, char* argv[]) {
    // Start loop at 1 to skip program name
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        // Check if argv[i + 1] is within bounds
        if (i + 1 < argc) {
            switch (LEGAL_FLAGS.count(arg) ? LEGAL_FLAGS.at(arg).flag : Flag::Unknown) {
                case Flag::ListenIP:
                    *(static_cast<std::string*>(LEGAL_FLAGS.at(arg).value)) = argv[++i];
                    break;
                case Flag::TargetIP:
                    *(static_cast<std::string*>(LEGAL_FLAGS.at(arg).value)) = argv[++i];
                    break;
                case Flag::Unknown:
                    // throw unknown argument error
                    break;
                default:
                    *(static_cast<int*>(LEGAL_FLAGS.at(arg).value)) = std::stoi(argv[++i]);
                    break;
            }
        } else {
            // throw out of bounds error
        }
    }
}

int main(int argc, char* argv[]) {

    std::shared_ptr<Logger> logger = std::make_shared<Logger>();
    logger->init("PROXY", "proxy.log");
    Packet::setLogger(logger);

    parse_args(argc, argv);

    int bind_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int connect_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (bind_sockfd < 0) {
        std::cerr << "[ERROR] Failed to create bind socket"<< std::endl;
        logger->log("ERROR", "Failed to create bind socket");
        return EXIT_FAILURE;
    }

    if (connect_sockfd < 0) {
        std::cerr << "[ERROR] Failed to create connect socket"<< std::endl;
        logger->log("ERROR", "Failed to create connect socket");
        return EXIT_FAILURE;
    }

    sockaddr_in listen_address;
    listen_address.sin_family = AF_INET;
    listen_address.sin_port = htons(LISTEN_PORT);

    if (inet_pton(AF_INET, LISTEN_IP.c_str(), &listen_address.sin_addr) <= 0) {
        std::cerr << "[ERROR] Invalid listen IP" << std::endl;
        logger->log("ERROR", "Invalid listen IP");
        return EXIT_FAILURE;
    }

    if (bind(bind_sockfd, (const struct sockaddr *) &listen_address, sizeof(listen_address)) < 0) {
        std::cerr << "[ERROR] Bind failed" << std::endl;
        logger->log("ERROR", "Bind failed");
        return EXIT_FAILURE;
    }

    logger->log("LISTENING FOR CLIENT", "listen=" + LISTEN_IP + ":" + std::to_string(LISTEN_PORT));

    sockaddr_in connect_address;
    connect_address.sin_family = AF_INET;
    connect_address.sin_port = htons(TARGET_PORT);

    if (inet_pton(AF_INET, TARGET_IP.c_str(), &connect_address.sin_addr) <= 0) {
        std::cerr << "[ERROR] Invalid target IP" << std::endl;
        logger->log("ERROR", "Invalid target IP");
        return EXIT_FAILURE;
    }

    if (connect(connect_sockfd, (struct sockaddr*) &connect_address, sizeof(connect_address)) < 0) {
        std::cerr << "[Error] Connecting to server failed" << std::endl;
        logger->log("ERROR", "Connecting to server failed");
        return EXIT_FAILURE;
    }

    logger->log("CONNECTED TO SERVER", "target=" + TARGET_IP + ":" + std::to_string(TARGET_PORT));

    sockaddr_in client_address;
    uint8_t client_buf[1024];
    socklen_t client_len = sizeof(client_address);
    socklen_t server_len = sizeof(connect_address);
    uint8_t server_buf[4];

    int delay;
    std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::bernoulli_distribution client_drop(CLIENT_DROP / 100.0);
    std::bernoulli_distribution client_delay(CLIENT_DELAY / 100.0);
    std::uniform_int_distribution<> client_delay_range(CLIENT_DELAY_TIME_MIN, CLIENT_DELAY_TIME_MAX);
    std::bernoulli_distribution server_drop(SERVER_DROP / 100.0);
    std::bernoulli_distribution server_delay(SERVER_DELAY / 100.0);
    std::uniform_int_distribution<> server_delay_range(SERVER_DELAY_TIME_MIN, SERVER_DELAY_TIME_MAX);

    while (true) {
        int received_from_client = recvfrom(bind_sockfd, client_buf, sizeof(client_buf), MSG_WAITALL, (sockaddr*) &client_address, &client_len);
        Packet packet = Packet::deserialize(client_buf);
        logger->log("RECEIVED PACKET", "Received packet from client: " + packet.to_string());

        if (client_drop(gen)) {
            logger->log("DROPPING PACKET", "Dropping client packet");
            continue;
        } else {
            if (client_delay(gen)) {
                delay = client_delay_range(gen);
                logger->log("DELAYING PACKET", "Delaying client packet to server for: " + std::to_string(delay) + " ms");
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            }
            sendto(connect_sockfd, client_buf, sizeof(client_buf), 0, (sockaddr*) &connect_address, sizeof(connect_address));
            logger->log("FORWARDING PACKET", "Forwarding packet to server");
        }

        int received_from_server = recvfrom(connect_sockfd, server_buf, sizeof(server_buf), MSG_WAITALL, (sockaddr*) &connect_address, &server_len);
        std::string ack = std::to_string(Packet::parse_ack(server_buf));
        logger->log("RECEIVED ACK", "Received ack from server - ack seq: " + ack);

        if (server_drop(gen)) {
            logger->log("DROPPING ACK", "Dropping server - ack seq: " + ack);
        } else {
            if (server_delay(gen)) {
                delay = server_delay_range(gen);
                logger->log("DELAYING ACK", "Delaying server ack seq: " + ack + " to client for: " + std::to_string(delay) + " ms");
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            }
            sendto(bind_sockfd, server_buf, sizeof(server_buf), 0, (sockaddr*) &client_address, sizeof(client_address));
            logger->log("FORWARDING ACK", "Forwarding ack seq: " + ack + " to client");
        }
    }

    close(bind_sockfd);
    close(connect_sockfd);

    return EXIT_SUCCESS;
}

