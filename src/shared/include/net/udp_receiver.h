#pragma once
#include <atomic>
#include <vector>

#include "config.h"

/**
 * @brief UDP receiver for laptop to receive camera frames from RPi
 *
 */
class UDPReceiver {
   public:
    std::atomic<bool> running{false};

    UDPReceiver(int port = net::DEFAULT_UDP_PORT);
    ~UDPReceiver();

    std::vector<uint8_t> recv();

   private:
    std::vector<uint8_t> buffer;  // buffer for receiving data so we don't allocate on every recv()
    int port;
    int socket_fd = -1;
};
