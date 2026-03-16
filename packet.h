#pragma once
#include <iostream>
#include <cstdint>
#include <exception>
#include <string>
#include <cstring>
#include <vector>
#include <arpa/inet.h>
#include <iomanip>

class Packet {
    private:
        static inline constexpr int MAX_SIZE = 1;
        // Wire format: [ seqNum (4 bytes) | payload (MAX_SIZE bytes) ]
        static inline constexpr int PACKET_SIZE = 4 + MAX_SIZE;
        int seq;
        std::vector<uint8_t> payload;

    public:
        Packet() {};
        Packet(int seq);

        void setSeq(int seq);
        const int& getSeq() const;

        const std::vector<uint8_t>& getPayload() const;
        int setPayload(std::string& input, int offset);
        void setPayload(uint8_t* buf);

        const std::vector<uint8_t> serialize() const;
        static Packet deserialize(uint8_t* buf);

        friend std::ostream& operator<<(std::ostream& os, const Packet& packet);
};

