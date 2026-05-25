#pragma once
#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief TCP client for laptop to send commands to RPi
 *
 */
class TCPClient {
   public:
    TCPClient(const std::string& host, int port = 5000);
    ~TCPClient();

    bool connect();
    bool send(const std::vector<uint8_t>& data);
    std::vector<uint8_t> recv();

   private:
    std::string host;
    int port;
    int client_fd = -1;
};