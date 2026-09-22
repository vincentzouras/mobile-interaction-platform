#pragma once
#include <cstdint>
#include <string_view>

// Packet header for UDP payload fragments since payloads exceed size allowed by UDP
#pragma pack(push, 1)  // force 1-byte alignment, no padding
struct FragmentHeader {
    uint32_t frame_id;         // Unique ID for the current message (e.g., increments by 1)
    uint16_t fragment_id;      // The index of this specific chunk (0, 1, 2, ...)
    uint16_t total_fragments;  // Total chunks expected for this message
    uint16_t payload_size;     // Size of the actual payload data in this specific packet
};
#pragma pack(pop)

namespace net {
inline constexpr int DEFAULT_TCP_PORT = 5000;
inline constexpr int DEFAULT_UDP_PORT = 5001;

// 'tailscale ip -4' fetches the stable IPv4 tailscale configured
inline constexpr std::string_view DEFAULT_RPI_IP = "100.98.21.83";
inline constexpr std::string_view DEFAULT_LAPTOP_IP = "100.71.115.57";

inline constexpr size_t OS_PAGE_SIZE = 4096;

// size of WireGuard tunnel MTU (max transmission unit)
inline constexpr size_t MAX_UDP_PACKET_PAYLOAD = 1150;
inline constexpr int MAX_FRAGMENT_DATA_SIZE = MAX_UDP_PACKET_PAYLOAD - sizeof(FragmentHeader);
}  // namespace net
