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

        // Wire format: [ seqNum (4 bytes) | message len (4 bytes) | payload (MAX_SIZE bytes) ]
        
        static inline constexpr int MAX_SIZE = 2;
        static inline constexpr int PACKET_SIZE = 8 + MAX_SIZE;
        int seq;
        int len;
        std::vector<uint8_t> payload;

    public:

        Packet() {};
        Packet(int seq, int len);

        void setSeq(int seq);
        const int& getSeq() const;

        void setLen(int len);
        const int& getLen() const;

        const std::vector<uint8_t>& getPayload() const;
        int setPayload(std::string& input, int offset);
        void setPayload(uint8_t* buf);

        const std::vector<uint8_t> serialize() const;
        static Packet deserialize(uint8_t* buf);

        friend std::ostream& operator<<(std::ostream& os, const Packet& packet);
};

