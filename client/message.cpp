#include "message.h"

Message::Message(int seq, std::string msg) : seq(seq), msg(std::move(msg)) {}

void Message::setSeq(int seq) {
    this->seq = seq;
}

void Message::setMsg(std::string msg) {
    this->msg = std::move(msg);
}

const int& Message::getSeq() const {
    return this->seq;
}

const std::string& Message::getMsg() const {
    return this->msg;
}

const std::vector<uint8_t>& Message::getPayload() const {
    return this->payload;
}

void Message::createPayload() const {
    return;
}

