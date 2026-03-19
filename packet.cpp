#include "packet.h"

Packet::Packet(int seq, int len) : seq(seq), len(len) {}

void Packet::setLogger(std::shared_ptr<Logger> logger) {
    Packet::logger = logger;
}

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

    for (; i < input.length(); i++) {
        if (count < MAX_SIZE) {
            this->payload.push_back(input[i]);
            count++;
        } else {
            break;
        }
    }

    return i;
}

void Packet::setPayload(uint8_t* buf) {
    this->payload.assign(buf + 8, buf + 8 + Packet::MAX_SIZE);
}

const std::vector<uint8_t> Packet::serialize() const {
    Packet::logger->log("FUNCTION CALL", "Packet::serialize()");

    std::vector<uint8_t> buf(Packet::PACKET_SIZE);

    uint32_t seq = htonl(this->seq);
    uint32_t len = htonl(this->len);

    std::memcpy(buf.data() + 0, &seq, 4);
    std::memcpy(buf.data() + 4, &len, 4);
    std::memcpy(buf.data() + 8, this->payload.data(), Packet::MAX_SIZE);

    Packet::logger->log("INFO", "Packet::serialize() - seq: " + std::to_string(seq) + " - payload: " + this->printPayload());

    Packet::logger->log("FUNCTION END", "Packet::serialize()");

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

const std::string Packet::printPayload() const {
    std::ostringstream oss;
    oss << "[ ";
    for (auto byte : this->payload) {
        oss << "0x" << std::hex << std::setfill('0') << std::setw(2) << (int)byte << " ";
    }
    oss << "]";
    return oss.str();
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

