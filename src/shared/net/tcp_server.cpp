#include "net/tcp_server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>

#include "net/config.h"
#include "spdlog/spdlog.h"

TCPServer::TCPServer(int port) : server_fd(-1), client_fd(-1) {
    // Create server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        throw std::runtime_error("[TCP] Failed to create socket");
    }

    set_server_opts(server_fd);

    // Bind and listen
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (::bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        close(server_fd);
        throw std::runtime_error("[TCP] Bind failed");
    }

    if (listen(server_fd, 1) < 0) {
        close(server_fd);
        throw std::runtime_error("[TCP] Listen failed");
    }

    running = true;
    spdlog::info("[TCP] Listening on port {}", port);

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
    spdlog::info("[TCP] Stopped");
}

std::vector<uint8_t> TCPServer::recv() {
    if (server_fd < 0 || !running) {
        return {};
    }

    // Check if no client is connected
    if (client_fd < 0) {
        int fd = accept(server_fd, nullptr, nullptr);
        if (fd < 0) {
            // EINVAL: stop() called shutdown() on the listening socket
            // EINTR:  interrupted by a signal
            if (errno != EINVAL && errno != EINTR) {
                spdlog::error("[TCP] Accept failed: {}", strerror(errno));
            }
            return {};
        }

        client_fd = fd;
        spdlog::info("[TCP] Client connected");

        if (!running) {
            return {};
        }
    }

    int n = ::recv(client_fd, buffer.data(), buffer.size(), 0);
    if (n < 0) {
        // Interrupted by a signal; return so the caller re-checks running and blocks again
        if (errno == EINTR) {
            return {};
        }

        // Actual error
        close(client_fd);
        client_fd = -1;
        spdlog::error("[TCP] Client disconnected (Error: {})", strerror(errno));
        return {};
    } else if (n == 0) {
        // Peer closed the connection, or stop() called shutdown() on this socket
        close(client_fd);
        client_fd = -1;
        spdlog::info("[TCP] Client disconnected (clean)");
        return {};
    }

    return std::vector<uint8_t>(buffer.begin(), buffer.begin() + n);
}

bool TCPServer::send(const std::vector<uint8_t>& data) {
    if (client_fd < 0) {
        spdlog::warn("[TCP] No client connected");
        return false;
    }

    if (::send(client_fd, data.data(), data.size(), MSG_NOSIGNAL) < 0) {
        spdlog::error("[TCP] Send failed");
        return false;
    }

    spdlog::info("[TCP] Sent: {} bytes", data.size());
    return true;
}

void TCPServer::stop() {
    running = false;
    if (client_fd >= 0) {
        ::shutdown(client_fd, SHUT_RDWR);
    }

    if (server_fd >= 0) {
        ::shutdown(server_fd, SHUT_RDWR);
    }
}

void TCPServer::set_server_opts(int fd) {
    // Allow reusing the address
    int opt = 1;  // enable
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        spdlog::warn("[TCP] Failed to set socket option SO_REUSEADDR");
    }
}
