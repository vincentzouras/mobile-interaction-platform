#include "net/udp_receiver.h"

#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>

UDPReceiver::UDPReceiver(int port) : port(port), socket_fd(-1) {}

UDPReceiver::~UDPReceiver() { stop(); }

bool UDPReceiver::start() {
    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        std::cerr << "[UDP] Failed to create socket\n";
        return false;
    }

    // Allow reusing the address
    int opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "[UDP] Bind failed\n";
        close(socket_fd);
        return false;
    }

    running = true;
    std::cout << "[UDP] Listening on port " << port << "\n";
    return true;
}

std::vector<uint8_t> UDPReceiver::recv() {
    std::vector<uint8_t> buffer(65536);  // Max datagram size 64KB

    if (socket_fd < 0) {
        return buffer;
    }

    int n = recvfrom(socket_fd, buffer.data(), buffer.size(), 0, nullptr, nullptr);
    if (n > 0) {
        buffer.resize(n);
    } else {
        std::cerr << "[UDP] Receive failed\n";
        buffer.resize(0);
    }

    return buffer;
}

void UDPReceiver::stop() {
    if (socket_fd >= 0) {
        close(socket_fd);
        socket_fd = -1;
        running = false;
        std::cout << "[UDP] Stopped\n";
    }
}
