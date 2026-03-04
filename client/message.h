#pragma once
#include <cstdint>
#include <exception>
#include <string>
#include <vector>

class Message {
    private:
        int seq;
        std::string msg;
        std::vector<uint8_t> payload;

    public:
        Message(int seq, std::string msg);

        void setSeq(int seq);
        void setMsg(std::string msg);

        const int& getSeq() const;
        const std::string& getMsg() const;
        const std::vector<uint8_t>& getPayload() const;

        void createPayload() const;
};

