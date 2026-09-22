#pragma once
#include <cstdint>
#include <vector>

#include "net/config.h"
#include "net/udp_transmitter.h"

/**
 * @brief Fragments an encoded payload and sends it over UDP.
 *
 * Splits the payload into MTU-sized fragments, each prefixed with a FragmentHeader so the
 * receiver can reassemble them. This is transport only: it doesn't know or care what the
 * payload contains (e.g. JPEG bytes).
 */
class FragmentSender {
   public:
    FragmentSender(UDPTransmitter& transmitter);

    /// Fragments and sends one payload as a single message (one frame_id).
    void send_payload(const std::vector<uint8_t>& payload);

   private:
    UDPTransmitter& udp_tx;
    uint32_t current_frame_id = 0;

    FragmentHeader host_header;

    std::vector<uint8_t> packet_buffer;
};
