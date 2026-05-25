#include "net/tcp_server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "net/config.h"

TCPServer::TCPServer(int port) : server_fd(-1), client_fd(-1) {
    // Create server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        throw std::runtime_error("[TCP] Failed to create socket");
    }

    // Set socket options

    // Allow reusing the address
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "[TCP] Warning: Failed to set socket option SO_REUSEADDR\n";
    }
    // Set timeout to avoid blocking indefinitely on accept()
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;  // 100ms
    if (setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        std::cerr << "[TCP] Warning: Failed to set SO_RCVTIMEO on server socket\n";
    }

    // Bind and listen
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

    buffer.resize(net::OS_PAGE_SIZE);
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
    if (server_fd < 0 || !running) {
        return {};
    }

    // Check if no client is connected
    if (client_fd < 0) {
        client_fd = accept(server_fd, nullptr, nullptr);  // blocks until client connects or timeout
        if (client_fd < 0) {
            // Ignore timeout errors, report real errors
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                std::cerr << "[TCP] Accept failed: " << strerror(errno) << "\n";
            }
            return {};
        }
        std::cout << "[TCP] Client connected\n";

        // Set receive timeout on client socket to avoid blocking indefinitely on recv()
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000;  // 100ms
        if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            std::cerr << "[TCP] Warning: Failed to set SO_RCVTIMEO on client socket\n";
        }
    }

    int n = ::recv(client_fd, buffer.data(), buffer.size() - 1, 0);
    if (n < 0) {
        // Don't close on timeout errors
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return {};
        }

        // Actual error
        close(client_fd);
        client_fd = -1;
        std::cerr << "[TCP] Client disconnected (Error: " << strerror(errno) << ")\n";
        return {};
    } else if (n == 0) {
        // Clean disconnect
        close(client_fd);
        client_fd = -1;
        std::cout << "[TCP] Client disconnected (clean)\n";
        return {};
    }

    return std::vector<uint8_t>(buffer.begin(), buffer.begin() + n);
}

bool TCPServer::send(const std::vector<uint8_t>& data) {
    if (client_fd < 0) {
        std::cerr << "[TCP] No client connected\n";
        return false;
    }

    if (::send(client_fd, data.data(), data.size(), MSG_NOSIGNAL) < 0) {
        std::cerr << "[TCP] Send failed\n";
        return false;
    }

    std::cout << "[TCP] Sent: " << data.size() << " bytes\n";
    return true;
}
