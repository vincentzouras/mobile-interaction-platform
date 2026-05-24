#pragma once
#include <atomic>
#include <vector>

/**
 * @brief TCP server for RPi to receive commands from laptop
 *
 */
class TCPServer {
   public:
    std::atomic<bool> running{false};

    TCPServer(int port = 5000);
    ~TCPServer();

    std::vector<uint8_t> recv();
    bool send(const std::vector<uint8_t>& data);

   private:
    int server_fd;
    int client_fd;
};
