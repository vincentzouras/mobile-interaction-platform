#pragma once
#include <cstdint>
#include <string_view>

// Packet header for UDP image fragments since they exceed payload size allowed by UDP
#pragma pack(push, 1)  // force 1-byte alignment, no padding
struct ImageFragmentHeader {
    uint32_t frame_id;         // Unique ID for the current frame (e.g., increments by 1)
    uint16_t fragment_id;      // The index of this specific chunk (0, 1, 2, ...)
    uint16_t total_fragments;  // Total chunks expected for this frame
    uint16_t payload_size;     // Size of the actual image data in this specific packet
};
#pragma pack(pop)

namespace net {
inline constexpr int DEFAULT_TCP_PORT = 5000;
inline constexpr int DEFAULT_UDP_PORT = 5001;

inline constexpr std::string_view DEFAULT_RPI_IP = "192.168.4.22";
inline constexpr std::string_view DEFAULT_LAPTOP_IP = "192.168.4.241";

inline constexpr size_t OS_PAGE_SIZE = 4096;

inline constexpr size_t MAX_UDP_PACKET_PAYLOAD = 1400;  // Conservative size to avoid fragmentation
inline constexpr int MAX_FRAGMENT_DATA_SIZE = MAX_UDP_PACKET_PAYLOAD - sizeof(ImageFragmentHeader);
}  // namespace net
