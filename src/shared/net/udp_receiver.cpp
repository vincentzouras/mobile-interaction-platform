#include "net/udp_receiver.h"

#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>

UDPReceiver::UDPReceiver(int port) : port(port), socket_fd(-1) {
    // Create socket
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        throw std::runtime_error("[UDP] Failed to create socket");
    }

    // Set socket options

    // Allow reusing the address
    int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "[UDP] Failed to set socket options\n";
    }
    // Set timeout to avoid blocking indefinitely on accept()
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;  // 100ms
    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        std::cerr << "[UDP] Warning: Failed to set SO_RCVTIMEO on socket\n";
    }

    // Bind
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(socket_fd);
        throw std::runtime_error("[UDP] Bind failed");
    }

    running = true;
    std::cout << "[UDP] Bound to port " << port << "\n";
}

UDPReceiver::~UDPReceiver() {
    if (socket_fd >= 0) {
        close(socket_fd);
        socket_fd = -1;
    }

    running = false;
    std::cout << "[UDP] Stopped\n";
}

std::vector<uint8_t> UDPReceiver::recv() {
    if (socket_fd < 0 || !running) {
        return {};
    }

    std::vector<uint8_t> buffer(65536);  // Max datagram size 64KB
    int n = recvfrom(socket_fd, buffer.data(), buffer.size(), 0, nullptr, nullptr);  // blocks
    if (n < 0) {
        // Don't treat timeout as an error
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return {};
        }

        std::cerr << "[UDP] Receive failed\n";
        return {};
    }

    buffer.resize(n);
    return buffer;
}
