#include "../packet.h"
#include "../logger.h"
#include <iostream>
#include <unordered_map>
#include <sys/socket.h>
#include <unistd.h>
#include <queue>

static std::string IP;
static int PORT;
static int TIMEOUT;
static int MAX_RETRIES;

static uint32_t seq = 0;
static std::queue<Packet> sent_queue;

enum class Flag {
    TargetIP,
    TargetPort,
    Timeout,
    MaxRetries,
    Unknown
};

struct Argument {
    Flag flag;
    void* value;
    Argument(Flag flag, void* value) : flag(flag), value(value) {}
};

static const std::unordered_map<std::string, Argument> LEGAL_FLAGS = {
    {"--target-ip",   {Flag::TargetIP, &IP}},
    {"--target-port", {Flag::TargetPort, &PORT}},
    {"--timeout",     {Flag::Timeout, &TIMEOUT}},
    {"--max-retries", {Flag::MaxRetries, &MAX_RETRIES}}
};

void parse_args(int argc, char* argv[]) {
    // Start loop at 1 to skip program name
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        // Check if argv[i + 1] is within bounds
        if (i + 1 < argc) {
            switch (LEGAL_FLAGS.count(arg) ? LEGAL_FLAGS.at(arg).flag : Flag::Unknown) {
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

int wait_for_ack(int& sockfd, sockaddr_in& server_address) {
    uint32_t buf[4];
    socklen_t len = sizeof(server_address);
    int netSeq;
    int n = recvfrom(sockfd, buf, sizeof(buf), MSG_WAITALL, (sockaddr*) &server_address, &len);
    std::memcpy(&netSeq, buf, 4);
    return n < 0 ? n : (int) ntohl(netSeq);
}

int main(int argc, char* argv[]) {

    std::shared_ptr<Logger> logger = std::make_shared<Logger>();
    logger->init("CLIENT", "client.log");
    Packet::setLogger(logger);

    parse_args(argc, argv);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        std::cerr << "[ERROR] Failed to create socket"<< std::endl;
        logger->log("ERROR", "Failed to create socket");
        return EXIT_FAILURE;
    }

    struct timeval tv;
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        std::cerr << "[ERROR] Failed to set timeout"<< std::endl;
        logger->log("ERROR", "Failed to set timeout");
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, IP.c_str(), &server_address.sin_addr) <= 0) {
        std::cerr << "[ERROR] Invalid target IP" << std::endl;
        logger->log("ERROR", "Invalid target IP");
        return EXIT_FAILURE;
    }

    if (connect(sockfd, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        std::cerr << "[Error] Connecting to server failed" << std::endl;
        logger->log("ERROR", "Connecting to server failed");
        return EXIT_FAILURE;
    }

    logger->log("CONNECTED", "target=" + IP + ":" + std::to_string(PORT)
             + " - timeout=" + std::to_string(TIMEOUT)
             + " - max_retries=" + std::to_string(MAX_RETRIES));

    std::string input;
    int ack;

    while (true) {
        std::getline(std::cin, input);
        int offset = 0;
        int tries = 0;

        while (offset < input.length()) {
            Packet packet(seq++, input.length());
            offset = packet.setPayload(input, offset);
            sent_queue.push(packet);
            std::vector<uint8_t> buf = packet.serialize();

            logger->log("SENDING PACKET", packet.to_string());
            sendto(sockfd, buf.data(), buf.size(), 0, (sockaddr*) &server_address, sizeof(server_address));
            
            while ((ack = wait_for_ack(sockfd, server_address)) < 0 && tries < MAX_RETRIES) {
                logger->log("ACK TIMEOUT", "Did not receive ack for seq: " + std::to_string(packet.getSeq()));
                sendto(sockfd, buf.data(), buf.size(), 0, (sockaddr*) &server_address, sizeof(server_address));
                tries++;
                logger->log("RE-SENT PACKET", "Re-sending packet -- try count: " + std::to_string(tries));
            }

            if (tries == MAX_RETRIES) {
                logger->log("FAILURE", "Failed to send message");
                break;
            } else {
                tries = 0;
            }

            if (!(ack < 0)) {
                logger->log("RECEIVED ACK", "Received ack for seq: " + std::to_string(sent_queue.front().getSeq()));
                sent_queue.pop();
            }
        }
    }

    close(sockfd);

    return EXIT_SUCCESS;
}

