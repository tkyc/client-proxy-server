#include "../common.h"

int wait_for_ack(int& sockfd, sockaddr_in& server_address) {
    uint32_t buf[4];
    socklen_t len = sizeof(server_address);
    int netSeq;
    int n = recvfrom(sockfd, buf, sizeof(buf), MSG_DONTWAIT, (sockaddr*) &server_address, &len);
    std::memcpy(&netSeq, buf, 4);
    return n < 0 ? n : (int) ntohl(netSeq);
}

bool read_input(std::string& input) {
    Common::LOGGER->log("WAITING FOR INPUT", "Enter a message to send to the server");
    bool readInput = static_cast<bool>(std::getline(std::cin, input));
    return readInput;
}

int main(int argc, char* argv[]) {

    if (argc < Common::CLIENT_ARG_COUNT) {
        throw ArgumentException("Missing arguments");
    }

    Common::setup_logger("CLIENT", "client.log");
    Common::setup_signal_handler();
    Common::parse_args(argc, argv);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        std::cerr << "[ERROR] Failed to create socket"<< std::endl;
        Common::LOGGER->log("ERROR", "Failed to create socket");
        return EXIT_FAILURE;
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(Common::TARGET_PORT);

    if (inet_pton(AF_INET, Common::TARGET_IP.c_str(), &server_address.sin_addr) <= 0) {
        std::cerr << "[ERROR] Invalid target TARGET_IP" << std::endl;
        Common::LOGGER->log("ERROR", "Invalid target TARGET_IP");
        return EXIT_FAILURE;
    }

    if (connect(sockfd, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        std::cerr << "[Error] Connecting to server failed" << std::endl;
        Common::LOGGER->log("ERROR", "Connecting to server failed");
        return EXIT_FAILURE;
    }

    Common::LOGGER->log("CONNECTED", "target=" + Common::TARGET_IP + ":" + std::to_string(Common::TARGET_PORT)
             + " - timeout=" + std::to_string(Common::TIMEOUT)
             + " - max_retries=" + std::to_string(Common::MAX_RETRIES));

    std::string input;
    int32_t ack ;
    uint32_t seq = 0;
    uint32_t offset;
    uint8_t retries;

    while (Common::RUNNING && read_input(input)) {

        offset = 0;
        retries = 0;
        Common::LOGGER->log("ENTERED INPUT", "Entered message: " + input);

        while (offset < input.length()) {
            Packet packet(seq++, input.length());
            offset = packet.setPayload(input, offset);
            std::vector<uint8_t> buf = packet.serialize();

            Common::LOGGER->log("SENDING PACKET", packet.to_string());
            sendto(sockfd, buf.data(), buf.size(), 0, (sockaddr*) &server_address, sizeof(server_address));

            auto start = std::chrono::steady_clock::now();
            auto timeout = std::chrono::seconds(Common::TIMEOUT);

            while (Common::RUNNING && retries < Common::MAX_RETRIES) {
                ack = wait_for_ack(sockfd, server_address);

                if (ack >= 0) {
                    Common::LOGGER->log("RECEIVED ACK", "Received ack for seq: " + std::to_string(ack));

                    if (ack == packet.getSeq()) {
                        break;
                    }
                }

                if (std::chrono::steady_clock::now() - start > timeout) {
                    Common::LOGGER->log("ACK TIMEOUT", "Did not receive ack for seq: " + std::to_string(packet.getSeq()));
                    sendto(sockfd, buf.data(), buf.size(), 0, (sockaddr*) &server_address, sizeof(server_address));
                    start = std::chrono::steady_clock::now();
                    retries++;
                    Common::LOGGER->log("RE-SENT PACKET", "Re-sending packet -- try count: " + std::to_string(retries));
                }
            }
            
            if (retries == Common::MAX_RETRIES) {
                Common::LOGGER->log("FAILURE", "Failed to send message");
                break;
            } else {
                retries = 0;
            }

        }
    }

    if (!Common::RUNNING) {
        Common::LOGGER->log("TERMINATING CLIENT", "Interrupt signal detected, exiting client and performing clean up");
    } else if (std::cin.eof()) {
        Common::LOGGER->log("TERMINATING CLIENT", "End of file detected, exiting client and performing clean up");
    }

    close(sockfd);
    Common::LOGGER->log("CLEAN UP", "Performed clean up");

    return EXIT_SUCCESS;
}

