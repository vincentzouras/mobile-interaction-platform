#pragma once
#include <atomic>
#include <vector>

#include "config.h"

/**
 * @brief TCP server for RPi to receive commands from laptop
 *
 */
class TCPServer {
   public:
    std::atomic<bool> running{false};

    TCPServer(int port = net::DEFAULT_TCP_PORT);
    ~TCPServer();

    std::vector<uint8_t> recv();
    bool send(const std::vector<uint8_t>& data);

   private:
    std::vector<uint8_t> buffer;
    int server_fd;
    int client_fd;
};
