#include "net/tcp_client.h"

#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>

TCPClient::TCPClient(const std::string& host, int port) : host(host), port(port), socket_fd(-1) {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        throw std::runtime_error("[TCP] Failed to create socket");
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
        close(socket_fd);
        throw std::runtime_error("[TCP] Invalid address: " + host);
    }

    if (::connect(socket_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(socket_fd);
        throw std::runtime_error("[TCP] Connection failed");
    }

    std::cout << "[TCP] Connected to " << host << ":" << port << "\n";
}

TCPClient::~TCPClient() {
    if (socket_fd >= 0) {
        close(socket_fd);
        socket_fd = -1;
        std::cout << "[TCP] Disconnected\n";
    }
}

bool TCPClient::send(const std::vector<uint8_t>& data) {
    if (socket_fd < 0) {
        std::cerr << "[TCP] Not connected\n";
        return false;
    }

    if (::send(socket_fd, data.data(), data.size(), 0) < 0) {
        std::cerr << "[TCP] Send failed\n";
        return false;
    }

    std::cout << "[TCP] Sent: " << data.size() << " bytes\n";
    return true;
}

std::vector<uint8_t> TCPClient::recv() {
    if (socket_fd < 0) {
        std::cerr << "[TCP] Not connected\n";
        return {};
    }

    std::vector<uint8_t> buffer(256);
    int n = ::recv(socket_fd, buffer.data(), buffer.size() - 1, 0);
    if (n > 0) {
        buffer.resize(n);
        return buffer;
    }

    return {};
}
