#pragma once
#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief UDP unicast transmitter for RPi to send camera frames to laptop
 *
 */
class UDPTransmitter {
   public:
    UDPTransmitter(const std::string& host, int port = 5001);
    ~UDPTransmitter();

    bool send(const std::vector<uint8_t>& data);

    std::atomic<bool> running{false};

   private:
    std::string host;
    int port;
    int socket_fd;
};
