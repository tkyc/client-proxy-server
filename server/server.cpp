#include "../common.h"

static std::queue<Packet> receive_queue;
static std::unordered_map<int, int> seen;

void send_ack(int& sockfd, sockaddr_in& client_address, int seq) {
    int netSeq = htonl(seq);
    uint8_t buf[4];
    std::memcpy(buf, &netSeq, 4);
    sendto(sockfd, buf, 4, 0, (sockaddr*) &client_address, sizeof(client_address));
}

size_t get_expected_packet_count(int msg_len) {
    return msg_len % Packet::PAYLOAD_SIZE == 0 ? msg_len / Packet::PAYLOAD_SIZE : msg_len / Packet::PAYLOAD_SIZE + 1;
}

std::string construct_message(int msg_len) {
    char chars[msg_len + 1];
    int offset = 0;

    while (!receive_queue.empty()) {
        Packet& p = receive_queue.front();

        for (int i = 0; i < Packet::PAYLOAD_SIZE && offset < msg_len; i++) {
            chars[offset++] = p.getPayload()[i];
        }

        receive_queue.pop();
    }

    chars[msg_len] = '\0';

    return std::string(chars);
}

int main(int argc, char* argv[]) {

    if (argc < Common::SERVER_ARG_COUNT) {
        throw ArgumentException("Missing arguments");
    }

    Common::setup_logger("SERVER", "server.log");
    Common::setup_signal_handler();
    Common::parse_args(argc, argv);

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        std::cerr << "[ERROR] Failed to create socket"<< std::endl;
        Common::LOGGER->log("ERROR", "Failed to create socket");
        return EXIT_FAILURE;
    }

    sockaddr_in server_address;
    sockaddr_in client_address;

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(Common::LISTEN_PORT);

    if (inet_pton(AF_INET, Common::LISTEN_IP.c_str(), &server_address.sin_addr) <= 0) {
        std::cerr << "[ERROR] Invalid IP" << std::endl;
        Common::LOGGER->log("ERROR", "Invalid IP");
        return EXIT_FAILURE;
    }

    if (bind(sockfd, (const struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        std::cerr << "[ERROR] Bind failed" << std::endl;
        Common::LOGGER->log("ERROR", "Bind failed");
        return EXIT_FAILURE;
    }

    Common::LOGGER->log("STARTED", "Listening on " + Common::LISTEN_IP + ":" + std::to_string(Common::LISTEN_PORT));

    uint8_t buf[1024];
    socklen_t client_len = sizeof(client_address);

    while (Common::RUNNING) {
        int n = recvfrom(sockfd, buf, sizeof(buf), MSG_WAITALL, (sockaddr*) &client_address, &client_len);
        Packet packet;

        if (n < 0) {
            if (errno == EINTR) {
                continue; 
            }
            Common::LOGGER->log("FATAL ERROR", std::string("received_from_client: ") + std::strerror(errno));
            break;
        } else if (n > 0) {
            packet = Packet::deserialize(buf);
        } else {
            Common::LOGGER->log("ZERO BYTE PACKET RECEIVED", "Empty packet received");
            continue;
        }

        if (packet.is_valid()) {

            if (!seen.count(packet.getSeq())) {
                seen.emplace(packet.getSeq(), 0);
                receive_queue.push(packet);
            }

            Common::LOGGER->log("PACKET RECEIVED", packet.to_string());
            send_ack(sockfd, client_address, packet.getSeq());
            Common::LOGGER->log("SENDING ACK", "Sending ack with seq: " + std::to_string(packet.getSeq()));
        }

        if (receive_queue.size() == get_expected_packet_count(packet.getLen())) {
            Common::LOGGER->log("MESSAGE CONSTRUCTED", "Received message: " + construct_message(packet.getLen()));
            Common::LOGGER->log("CLEARING RECEIVE QUEUE", "receive_queue size: " + std::to_string(receive_queue.size()));
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

