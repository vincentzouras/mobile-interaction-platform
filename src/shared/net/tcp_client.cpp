#include "net/tcp_client.h"

#include <arpa/inet.h>
#include <unistd.h>

#include <cstring>

#include "net/config.h"
#include "spdlog/spdlog.h"

TCPClient::TCPClient(const std::string& host, int port) : host(host), port(port), client_fd(-1) {
    spdlog::info("[TCP] Client initialized for target {}:{}", host, port);
    buffer.resize(net::OS_PAGE_SIZE);
}

TCPClient::~TCPClient() {
    if (client_fd >= 0) {
        close(client_fd);
        client_fd = -1;
        spdlog::info("[TCP] Disconnected");
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
        spdlog::error("[TCP] Failed to create socket");
        return false;
    }

    // Set timeout so recv() doesn't block indefinitely
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;  // 100ms
    if (setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        spdlog::warn("[TCP] Failed to set SO_RCVTIMEO");
    }

    // Build address
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
        spdlog::error("[TCP] Invalid address: {}", host);
        close(client_fd);
        client_fd = -1;
        return false;
    }

    if (::connect(client_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(client_fd);
        client_fd = -1;
        spdlog::error("[TCP] Connection failed");
        return false;
    }

    spdlog::info("[TCP] Successfully connected to {}:{}", host, port);
    return true;
}

bool TCPClient::send(const std::vector<uint8_t>& data) {
    if (client_fd < 0) {
        spdlog::warn("[TCP] Not connected");
        return false;
    }

    if (::send(client_fd, data.data(), data.size(), MSG_NOSIGNAL) < 0) {
        spdlog::error("[TCP] Send failed");
        return false;
    }

    spdlog::info("[TCP] Sent: {} bytes", data.size());
    return true;
}

std::vector<uint8_t> TCPClient::recv() {
    if (client_fd < 0) {
        spdlog::warn("[TCP] Not connected");
        return {};
    }

    int n = ::recv(client_fd, buffer.data(), buffer.size() - 1, 0);
    if (n < 0) {
        // Ignore timeout errors
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return {};
        }

        // Actual error
        spdlog::error("[TCP] Disconnected from server (Error: {})", strerror(errno));
        close(client_fd);
        client_fd = -1;
        return {};
    } else if (n == 0) {
        // Clean disconnect
        spdlog::info("[TCP] Disconnected from server (clean)");
        close(client_fd);
        client_fd = -1;
        return {};
    }

    return std::vector<uint8_t>(buffer.begin(), buffer.begin() + n);
}
