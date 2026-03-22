#pragma once
#include "logger.h"
#include <memory>
#include <iostream>
#include <cstdint>
#include <exception>
#include <string>
#include <cstring>
#include <vector>
#include <arpa/inet.h>
#include <iomanip>

// Wire format: [ sequence number (4 bytes) | message len (4 bytes) | payload (MAX_SIZE bytes) ]
class Packet {
    private:
        static inline std::shared_ptr<Logger> logger = nullptr;
        int seq;
        int len;
        std::vector<uint8_t> payload;

    public:
        static inline constexpr int MAX_SIZE = 1;
        static inline constexpr int PACKET_SIZE = 8 + MAX_SIZE;

        Packet() {};

        Packet(int seq, int len);

        void setSeq(int seq);

        const int& getSeq() const;

        void setLen(int len);

        const int& getLen() const;

        static void setLogger(std::shared_ptr<Logger> logger);

        const std::vector<uint8_t>& getPayload() const;

        int setPayload(std::string& input, int offset);

        void setPayload(uint8_t* buf);

        const std::vector<uint8_t> serialize() const;

        static Packet deserialize(uint8_t* buf);

        static int parse_ack(uint8_t* buf);

        const bool is_valid() const;

        const std::string payload_to_string() const;

        std::string to_string() const;

        friend std::ostream& operator<<(std::ostream& os, const Packet& packet);
};

