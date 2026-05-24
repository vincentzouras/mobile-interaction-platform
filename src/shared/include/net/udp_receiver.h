#pragma once
#include <atomic>
#include <vector>

/**
 * @brief UDP receiver for laptop to receive camera frames from RPi
 *
 */
class UDPReceiver {
   public:
    std::atomic<bool> running{false};

    UDPReceiver(int port = 5001);
    ~UDPReceiver();

    std::vector<uint8_t> recv();

   private:
    int port;
    int socket_fd = -1;
};
