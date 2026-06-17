#pragma once
#include <netinet/in.h>

#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

#include "config.h"

/**
 * @brief UDP unicast transmitter for RPi to send camera frames to laptop
 *
 */
class UDPTransmitter {
   public:
    UDPTransmitter(const std::string& host = std::string(net::DEFAULT_LAPTOP_IP),
                   int port = net::DEFAULT_UDP_PORT);
    ~UDPTransmitter();

    bool send(const std::vector<uint8_t>& data);

    std::atomic<bool> running{false};

   private:
    sockaddr_in addr{};
    int socket_fd;
};
