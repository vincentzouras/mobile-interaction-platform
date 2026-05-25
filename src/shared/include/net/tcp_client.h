#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "config.h"

/**
 * @brief TCP client for laptop to send commands to RPi
 *
 */
class TCPClient {
   public:
    TCPClient(const std::string& host = std::string(net::DEFAULT_RPI_IP),
              int port = net::DEFAULT_TCP_PORT);
    ~TCPClient();

    bool connect();
    bool send(const std::vector<uint8_t>& data);
    std::vector<uint8_t> recv();

   private:
    std::vector<uint8_t> buffer;
    std::string host;
    int port;
    int client_fd = -1;
};