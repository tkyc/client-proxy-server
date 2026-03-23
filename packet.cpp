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

void Packet::setLogger(std::shared_ptr<Logger> logger) {
    Packet::logger = logger;
}

const std::vector<uint8_t>& Packet::getPayload() const {
    return this->payload;
}

int Packet::setPayload(std::string& input, int offset) {
    int count = 0;
    size_t i = offset;

    for (; i < input.length(); i++) {
        if (count < Packet::PAYLOAD_SIZE) {
            this->payload.push_back(input[i]);
            count++;
        } else {
            break;
        }
    }

    return i;
}

void Packet::setPayload(uint8_t* buf) {
    this->payload.assign(buf + 8, buf + 8 + Packet::PAYLOAD_SIZE);
}

const std::vector<uint8_t> Packet::serialize() const {
    Packet::logger->log("FUNCTION CALL", "Packet::serialize()");

    std::vector<uint8_t> buf(Packet::PACKET_SIZE);

    uint32_t seq = htonl(this->seq);
    uint32_t len = htonl(this->len);

    std::memcpy(buf.data() + 0, &seq, 4);
    std::memcpy(buf.data() + 4, &len, 4);
    std::memcpy(buf.data() + 8, this->payload.data(), Packet::PAYLOAD_SIZE);

    Packet::logger->log("INFO", "Packet::serialize() - seq: " + std::to_string(this->seq) + " - payload: " + this->payload_to_string());

    Packet::logger->log("FUNCTION END", "Packet::serialize()");

    return buf;
}

Packet Packet::deserialize(uint8_t* buf) {
    Packet::logger->log("FUNCTION CALL", "Packet::deserialize()");

    int netSeq;
    int netLen;

    std::memcpy(&netSeq, buf + 0, 4);
    std::memcpy(&netLen, buf + 4, 4);

    Packet packet;
    packet.setSeq(ntohl(netSeq));
    packet.setLen(ntohl(netLen));
    packet.setPayload(buf);
    
    Packet::logger->log("INFO", "Packet::derialize() - seq: " + std::to_string(packet.getSeq()) + " - payload: " + packet.payload_to_string());

    Packet::logger->log("FUNCTION END", "Packet::deserialize()");

    return packet;
}

int Packet::parse_ack(uint8_t* buf) {
    int netSeq;
    std::memcpy(&netSeq, buf, 4);
    return ntohl(netSeq);
}

bool Packet::is_valid() const {
    return !(this->seq < 0) && this->payload.size() != 0;
}

const std::string Packet::payload_to_string() const {
    std::ostringstream oss;
    oss << "[ ";
    for (auto byte : this->payload) {
        oss << "0x" << std::hex << std::setfill('0') << std::setw(2) << (int)byte << " ";
    }
    oss << "]";
    return oss.str();
}

std::string Packet::to_string() const {
    std::stringstream ss;
    ss << *this;
    return ss.str();
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

