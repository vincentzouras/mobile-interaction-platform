#pragma once
#include <cstdint>
#include <string_view>

namespace net {
inline constexpr int DEFAULT_TCP_PORT = 5000;
inline constexpr int DEFAULT_UDP_PORT = 5001;

inline constexpr std::string_view DEFAULT_RPI_IP = "192.168.4.21";
inline constexpr std::string_view DEFAULT_LAPTOP_IP = "192.168.4.241";

inline constexpr size_t MAX_UDP_PACKET_SIZE = 1200;  // Conservative size to avoid fragmentation
inline constexpr size_t OS_PAGE_SIZE = 4096;
}  // namespace net
