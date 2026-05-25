#include "net/udp_transmitter.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

UDPTransmitter::UDPTransmitter(const std::string& host, int port)
    : host(host), port(port), socket_fd(-1) {
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_fd < 0) {
        throw std::runtime_error("[UDP] Failed to create socket");
    }

    running = true;
}

UDPTransmitter::~UDPTransmitter() {
    if (socket_fd >= 0) {
        close(socket_fd);
        socket_fd = -1;
    }

    running = false;
    std::cout << "[UDP] Stopped\n";
}

bool UDPTransmitter::send(const std::vector<uint8_t>& data) {
    if (socket_fd < 0 || data.empty()) {
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

    if (sendto(socket_fd, data.data(), data.size(), 0, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "[UDP] Send failed\n";
        return false;
    }

    return true;
}
