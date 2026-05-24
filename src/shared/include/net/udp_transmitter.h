#pragma once
#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

class UDPTransmitter {
   public:
    /**
     * @brief Construct a new UDPTransmitter object
     *
     * @param host Target host IP address
     * @param port Target port number
     */
    UDPTransmitter(const std::string& host, int port);
    ~UDPTransmitter();

    bool send(const std::vector<uint8_t>& data);

    std::atomic<bool> running{false};

   private:
    std::string host;
    int port;
    int socket_fd;
};
