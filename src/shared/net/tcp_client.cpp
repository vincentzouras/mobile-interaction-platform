#include "net/tcp_client.h"

#include <arpa/inet.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

TCPClient::TCPClient(const std::string& host, int port) : host(host), port(port), client_fd(-1) {
    std::cout << "[TCP] Client initialized for target " << host << ":" << port << "\n";
}

TCPClient::~TCPClient() {
    if (client_fd >= 0) {
        close(client_fd);
        client_fd = -1;
        std::cout << "[TCP] Disconnected\n";
    }
}

bool TCPClient::connect() {
    // Close existing connection if any
    if (client_fd >= 0) {
        close(client_fd);
        client_fd = -1;
    }

    // Create socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        std::cerr << "[TCP] Failed to create socket\n";
        return false;
    }

    // Set timeout so recv() doesn't block indefinitely
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;  // 100ms
    if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        std::cerr << "[TCP] Warning: Failed to set SO_RCVTIMEO\n";
    }

    // Build address
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
        std::cerr << "[TCP] Invalid address: " << host << "\n";
        close(client_fd);
        client_fd = -1;
        return false;
    }

    if (::connect(client_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(client_fd);
        client_fd = -1;
        std::cerr << "[TCP] Connection failed\n";
        return false;
    }

    std::cout << "[TCP] Successfully connected to " << host << ":" << port << "\n";
    return true;
}

bool TCPClient::send(const std::vector<uint8_t>& data) {
    if (client_fd < 0) {
        std::cerr << "[TCP] Not connected\n";
        return false;
    }

    if (::send(client_fd, data.data(), data.size(), MSG_NOSIGNAL) < 0) {
        std::cerr << "[TCP] Send failed\n";
        return false;
    }

    std::cout << "[TCP] Sent: " << data.size() << " bytes\n";
    return true;
}

std::vector<uint8_t> TCPClient::recv() {
    if (client_fd < 0) {
        std::cerr << "[TCP] Not connected\n";
        return {};
    }

    std::vector<uint8_t> buffer(256);
    int n = ::recv(client_fd, buffer.data(), buffer.size() - 1, 0);
    if (n < 0) {
        // Ignore timeout errors
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return {};
        }

        // Actual error
        std::cerr << "[TCP] Disconnected from server (Error: " << strerror(errno) << ")\n";
        close(client_fd);
        client_fd = -1;
        return {};
    } else if (n == 0) {
        // Clean disconnect
        std::cerr << "[TCP] Disconnected from server (clean)\n";
        close(client_fd);
        client_fd = -1;
        return {};
    }

    buffer.resize(n);
    return buffer;
}
