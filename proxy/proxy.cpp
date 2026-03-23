#include "../common.h"

int main(int argc, char* argv[]) {

    if (argc < Common::PROXY_ARG_COUNT) {
        throw ArgumentException("Missing arguments");
    }

    Common::setup_logger("PROXY", "proxy.log");
    Common::setup_signal_handler();
    Common::parse_args(argc, argv);

    int bind_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int connect_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (bind_sockfd < 0) {
        std::cerr << "[ERROR] Failed to create bind socket"<< std::endl;
        Common::LOGGER->log("ERROR", "Failed to create bind socket");
        return EXIT_FAILURE;
    }

    if (connect_sockfd < 0) {
        std::cerr << "[ERROR] Failed to create connect socket"<< std::endl;
        Common::LOGGER->log("ERROR", "Failed to create connect socket");
        return EXIT_FAILURE;
    }

    sockaddr_in listen_address;
    listen_address.sin_family = AF_INET;
    listen_address.sin_port = htons(Common::LISTEN_PORT);

    if (inet_pton(AF_INET, Common::LISTEN_IP.c_str(), &listen_address.sin_addr) <= 0) {
        std::cerr << "[ERROR] Invalid listen IP" << std::endl;
        Common::LOGGER->log("ERROR", "Invalid listen IP");
        return EXIT_FAILURE;
    }

    if (bind(bind_sockfd, (const struct sockaddr *) &listen_address, sizeof(listen_address)) < 0) {
        std::cerr << "[ERROR] Bind failed" << std::endl;
        Common::LOGGER->log("ERROR", "Bind failed");
        return EXIT_FAILURE;
    }

    Common::LOGGER->log("LISTENING FOR CLIENT", "listen=" + Common::LISTEN_IP + ":" + std::to_string(Common::LISTEN_PORT));

    sockaddr_in connect_address;
    connect_address.sin_family = AF_INET;
    connect_address.sin_port = htons(Common::TARGET_PORT);

    if (inet_pton(AF_INET, Common::TARGET_IP.c_str(), &connect_address.sin_addr) <= 0) {
        std::cerr << "[ERROR] Invalid target IP" << std::endl;
        Common::LOGGER->log("ERROR", "Invalid target IP");
        return EXIT_FAILURE;
    }

    if (connect(connect_sockfd, (struct sockaddr*) &connect_address, sizeof(connect_address)) < 0) {
        std::cerr << "[Error] Connecting to server failed" << std::endl;
        Common::LOGGER->log("ERROR", "Connecting to server failed");
        return EXIT_FAILURE;
    }

    Common::LOGGER->log("CONNECTED TO SERVER", "target=" + Common::TARGET_IP + ":" + std::to_string(Common::TARGET_PORT));

    sockaddr_in client_address;
    uint8_t client_buf[1024];
    socklen_t client_len = sizeof(client_address);
    socklen_t server_len = sizeof(connect_address);
    uint8_t server_buf[4];

    int delay;
    std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::bernoulli_distribution client_drop(Common::CLIENT_DROP / 100.0);
    std::bernoulli_distribution client_delay(Common::CLIENT_DELAY / 100.0);
    std::uniform_int_distribution<> client_delay_range(Common::CLIENT_DELAY_TIME_MIN, Common::CLIENT_DELAY_TIME_MAX);
    std::bernoulli_distribution server_drop(Common::SERVER_DROP / 100.0);
    std::bernoulli_distribution server_delay(Common::SERVER_DELAY / 100.0);
    std::uniform_int_distribution<> server_delay_range(Common::SERVER_DELAY_TIME_MIN, Common::SERVER_DELAY_TIME_MAX);

    while (Common::RUNNING) {
        int received_from_client = recvfrom(bind_sockfd, client_buf, sizeof(client_buf), MSG_WAITALL, (sockaddr*) &client_address, &client_len);
        Packet packet;

        if (received_from_client < 0) {
            if (errno == EINTR) {
                continue; 
            }
            Common::LOGGER->log("FATAL ERROR", std::string("received_from_client: ") + std::strerror(errno));
            break;
        } else if (received_from_client > 0) {
            packet = Packet::deserialize(client_buf);
            Common::LOGGER->log("RECEIVED PACKET", "Received packet from client: " + packet.to_string());
        } else {
            Common::LOGGER->log("ZERO BYTE PACKET RECEIVED", "Empty packet received");
            continue;
        }

        if (client_drop(gen)) {
            Common::LOGGER->log("DROPPING PACKET", "Dropping client packet");
            continue;

        } else {

            if (client_delay(gen)) {
                delay = client_delay_range(gen);
                Common::LOGGER->log("DELAYING PACKET", "Delaying client packet to server for: " + std::to_string(delay) + " ms");
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            }

            sendto(connect_sockfd, client_buf, sizeof(client_buf), 0, (sockaddr*) &connect_address, sizeof(connect_address));
            Common::LOGGER->log("FORWARDING PACKET", "Forwarding packet to server");
        }

        int received_from_server = recvfrom(connect_sockfd, server_buf, sizeof(server_buf), MSG_WAITALL, (sockaddr*) &connect_address, &server_len);
        std::string ack;

        if (received_from_server < 0) {
            if (errno == EINTR) {
                continue; 
            }
            Common::LOGGER->log("FATAL ERROR", std::string("received_from_server: ") + std::strerror(errno));
            break;
        } else if (received_from_server > 0) {
            ack = std::to_string(Packet::parse_ack(server_buf));
            Common::LOGGER->log("RECEIVED ACK", "Received ack from server - ack seq: " + ack);
        } else {
            Common::LOGGER->log("ZERO BYTE ACK RECEIVED", "Empty ack packet received");
            continue;
        }

        if (server_drop(gen)) {

            Common::LOGGER->log("DROPPING ACK", "Dropping server - ack seq: " + ack);

        } else {

            if (server_delay(gen)) {
                delay = server_delay_range(gen);
                Common::LOGGER->log("DELAYING ACK", "Delaying server ack seq: " + ack + " to client for: " + std::to_string(delay) + " ms");
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            }

            sendto(bind_sockfd, server_buf, sizeof(server_buf), 0, (sockaddr*) &client_address, sizeof(client_address));
            Common::LOGGER->log("FORWARDING ACK", "Forwarding ack seq: " + ack + " to client");
        }
    }

    if (!Common::RUNNING) {
        Common::LOGGER->log("TERMINATING CLIENT", "Interrupt signal detected, exiting client and performing clean up");
    } else if (std::cin.eof()) {
        Common::LOGGER->log("TERMINATING CLIENT", "End of file detected, exiting client and performing clean up");
    }

    close(bind_sockfd);
    close(connect_sockfd);
    Common::LOGGER->log("CLEAN UP", "Performed clean up");

    return EXIT_SUCCESS;
}

