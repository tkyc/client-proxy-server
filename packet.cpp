#include "packet.h"

Packet::Packet(int seq, int len) : seq(seq), len(len) {}

void Packet::setSeq(int seq) {
    this->seq = seq;
}

const int& Packet::getSeq() const {
    return this->seq;
}

void Packet::setLen(int len) {
    this->len = len;
}

const int& Packet::getLen() const {
    return this->len;
}

const std::vector<uint8_t>& Packet::getPayload() const {
    return this->payload;
}

int Packet::setPayload(std::string& input, int offset) {
    int count = 0;
    int i = offset;

    std::cout<< "Packet::setPayload() - starting payload size: " << this->payload.size() << std::endl;
    std::cout<< "Packet::setPayload() - offset: " << offset << std::endl;

    for (; i < input.length(); i++) {
        if (count < MAX_SIZE) {
            std::cout<< "Packet::setPayload() - i: " << i << " - input[i]: " << input[i] << std::endl;
            std::cout<< "Packet::setPayload() - sizeof(input[i]): " << sizeof(input[i]) << std::endl;
            this->payload.push_back(input[i]);
            std::cout<< "Packet::setPayload() - working payload size: " << this->payload.size() << std::endl;
            count++;
        } else {
            break;
        }
    }

    std::cout<< "Packet::setPayload() - final payload size: " << this->payload.size() << std::endl;

    for (uint8_t byte : this->payload) {
        std::cout << static_cast<char>(byte) << " ";
    }
    std::cout << std::endl;

    return i;
}

void Packet::setPayload(uint8_t* buf) {
    this->payload.assign(buf + 8, buf + 8 + Packet::MAX_SIZE);
}

const std::vector<uint8_t> Packet::serialize() const {
    std::cout << "Packet::serialize() - START" << std::endl;
    // Wire format: [ seqNum (4 bytes) | payload (MAX_SIZE bytes) ]
    std::vector<uint8_t> buf(Packet::PACKET_SIZE);

    uint32_t seq = htonl(this->seq);
    uint32_t len = htonl(this->len);

    std::memcpy(buf.data() + 0, &seq, 4);
    std::memcpy(buf.data() + 4, &len, 4);
    std::memcpy(buf.data() + 8, this->payload.data(), Packet::MAX_SIZE);

    std::cout << "Packet::serialize() - seq: " << seq << " - " << "payload: [";
    for (uint8_t byte : this->payload) {
        std::cout << static_cast<char>(byte) << " ";
    }
    std::cout << "]" << std::endl;
    std::cout << "Packet::serialize() - buf size: " << buf.size() << std::endl;
    std::cout << "Packet::serialize() - buf: [ ";
    for (uint8_t byte : buf) {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') 
              << static_cast<int>(byte) << " ";
    }
    std::cout << "]" << std::endl;
    std::cout << "Packet::serialize() - END" << std::endl;

    return buf;
}

Packet Packet::deserialize(uint8_t* buf) {
    int netSeq;
    int netLen;

    std::memcpy(&netSeq, buf + 0, 4);
    std::memcpy(&netLen, buf + 4, 4);

    Packet packet;
    packet.setSeq(ntohl(netSeq));
    packet.setLen(ntohl(netLen));
    packet.setPayload(buf);

    return packet;
}

std::ostream& operator<<(std::ostream& os, const Packet& packet) {
    os << "SEQUENCE: " << packet.getSeq() 
        << " - MESSAGE LEN: " << packet.getLen() 
        << " - PAYLOAD SIZE: " << packet.getPayload().size() << " - PAYLOAD MSG CHARS: [";
    
    for (uint8_t byte : packet.getPayload()) {
        os << static_cast<char>(byte);
    }
    
    os << "]";
    return os;
}

