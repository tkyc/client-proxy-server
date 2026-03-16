#include "../packet.h"
#include <iostream>
#include <unordered_map>
#include <sys/socket.h>
#include <unistd.h>

static std::string IP;
static int PORT;
static int TIMEOUT;
static int MAX_RETRIES;
static uint32_t seq = 0;

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

void print_args() {
    std::cout << IP << std::endl;
    std::cout << PORT << std::endl;
    std::cout << TIMEOUT << std::endl;
    std::cout << MAX_RETRIES << std::endl;
}

int main(int argc, char* argv[]) {
    parse_args(argc, argv);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        std::cout << "ERROR CREATING SOCKET..." << std::endl;
        // throw exception here
        return 1;
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, IP.c_str(), &server_address.sin_addr) <= 0) {
        std::cerr << "INVALID ADDRESS..." << std::endl;
        return 1;
    }

    if (connect(sockfd, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        std::cerr << "CONNECTION FAILED..." << std::endl;
        return 1;
    }

    std::cout << "Connected to server!" << std::endl;
    std::string input;

    while (true) {
        std::getline(std::cin, input);
        int offset = 0;

        while (offset < input.length()) {
            Packet packet(seq++);
            offset = packet.setPayload(input, offset);
            std::vector<uint8_t> buf = packet.serialize();
            sendto(sockfd, buf.data(), buf.size(), 0, (sockaddr*) &server_address, sizeof(server_address));
        }
    }

    close(sockfd);

    return 0;
}

