#include "net/tcp_server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

TCPServer::TCPServer(int port) : port(port), server_fd(-1), client_fd(-1) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        throw std::runtime_error("[TCP] Failed to create socket");
    }

    // Allow reusing the address
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw std::runtime_error("[TCP] Failed to set socket options");
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(server_fd);
        throw std::runtime_error("[TCP] Bind failed");
    }

    if (listen(server_fd, 1) < 0) {
        close(server_fd);
        throw std::runtime_error("[TCP] Listen failed");
    }

    running = true;
    std::cout << "[TCP] Listening on port " << port << "\n";
}

TCPServer::~TCPServer() {
    if (client_fd >= 0) {
        close(client_fd);
        client_fd = -1;
    }

    if (server_fd >= 0) {
        close(server_fd);
        server_fd = -1;
    }

    running = false;
    std::cout << "[TCP] Stopped\n";
}

std::vector<uint8_t> TCPServer::recv() {
    if (server_fd < 0) {
        return {};
    }

    // Check if no client is connected
    if (client_fd < 0) {
        client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            std::cerr << "[TCP] Accept failed\n";
            return {};
        }
        std::cout << "[TCP] Client connected\n";
    }

    std::vector<uint8_t> buffer(256);
    int n = ::recv(client_fd, buffer.data(), buffer.size() - 1, 0);
    if (n <= 0) {
        close(client_fd);
        client_fd = -1;
        std::cout << "[TCP] Client disconnected\n";
        return {};
    }

    buffer.resize(n);
    return buffer;
}

bool TCPServer::send(const std::vector<uint8_t>& data) {
    if (client_fd < 0) {
        std::cerr << "[TCP] No client connected\n";
        return false;
    }

    if (::send(client_fd, data.data(), data.size(), 0) < 0) {
        std::cerr << "[TCP] Send failed\n";
        return false;
    }

    std::cout << "[TCP] Sent: " << data.size() << " bytes\n";
    return true;
}
