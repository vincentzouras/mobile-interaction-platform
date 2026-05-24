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
    /**
     * @brief Construct a new TCPClient object
     *
     * @param host Target host IP address
     * @param port Target port number
     */
    TCPClient(const std::string& host, int port = 5000);
    ~TCPClient();

    bool send(const std::vector<uint8_t>& data);
    std::vector<uint8_t> recv();

   private:
    std::string host;
    int port;
    int socket_fd = -1;
};