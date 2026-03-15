#include <iostream>
#include <string>
#include <unordered_map>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

static std::string IP;
static int PORT;

enum class Flag {
    ListenIP,
    ListenPort,
    Unknown
};

struct Argument {
    Flag flag;
    void* value;
    Argument(Flag flag, void* value) : flag(flag), value(value) {}
};

static const std::unordered_map<std::string, Argument> LEGAL_FLAGS = {
    {"--listen-ip",   {Flag::ListenIP, &IP}},
    {"--listen-port", {Flag::ListenPort, &PORT}},
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
    sockaddr_in client_address;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, IP.c_str(), &server_address.sin_addr) <= 0) {
        std::cerr << "INVALID ADDRESS..." << std::endl;
        return -1;
    }

    if (bind(sockfd, (const struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        perror("bind failed");
        return -1;
    }

    std::cout << "LISTENING ON PORT: " << PORT << std::endl;

    char buffer[1024];
    socklen_t len = sizeof(client_address);

    while (true) {
        int n = recvfrom(sockfd, (char*) buffer, 1024, MSG_WAITALL, (sockaddr*) &client_address, &len);
        buffer[n] = '\0';
        std::cout << buffer << std::endl;
    }

    close(sockfd);

    return 0;
}

