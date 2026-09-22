#pragma once
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "net/config.h"
#include "net/udp_receiver.h"

/**
 * @brief Reassembles fragmented payloads received over UDP.
 *
 * This is transport only: it returns the raw payload bytes and doesn't decode them.
 *
 * Delivery policy is latest-complete-message-wins: fragments from older messages are dropped,
 * and messages that can never complete (lost fragment) are discarded once a newer message
 * completes, so a live stream never falls behind.
 */
class FragmentReceiver {
   public:
    FragmentReceiver(UDPReceiver& receiver);

    /// Drains the socket and returns true once one complete payload has been reassembled.
    bool receive_payload(std::vector<uint8_t>& out_payload);

   private:
    UDPReceiver& udp_rx;

    // Structure to hold incomplete messages as they arrive
    struct FrameBuffer {
        std::vector<std::vector<uint8_t>> fragments;  // only data no header
        uint16_t received_count = 0;
        uint16_t total_fragments = 0;
    };

    std::unordered_map<uint32_t, FrameBuffer> frame_buffers;
    uint32_t last_completed_frame_id = 0;

    // Cleans up old, incomplete frames so they don't leak memory
    void cleanup_old_frames(uint32_t current_frame_id);
};
