#include "net/fragment_receiver.h"

#include <netinet/in.h>

#include <cstring>

#include "spdlog/spdlog.h"

FragmentReceiver::FragmentReceiver(UDPReceiver& receiver) : udp_rx(receiver) {}

bool FragmentReceiver::receive_payload(std::vector<uint8_t>& out_payload) {
    // Read all available packets in the UDP socket queue
    while (true) {
        std::vector<uint8_t> packet = udp_rx.recv();

        // Are there more packets to read?
        if (packet.empty()) break;

        // Skip packets that are too small to contain a valid header
        if (packet.size() < sizeof(FragmentHeader)) continue;

        // Extract the header
        FragmentHeader network_header;
        std::memcpy(&network_header, packet.data(), sizeof(FragmentHeader));

        FragmentHeader host_header;
        host_header.frame_id = ntohl(network_header.frame_id);
        host_header.fragment_id = ntohs(network_header.fragment_id);
        host_header.total_fragments = ntohs(network_header.total_fragments);
        host_header.payload_size = ntohs(network_header.payload_size);

        // Skip packets from older frames
        if (host_header.frame_id <= last_completed_frame_id && last_completed_frame_id > 0) {
            // If we see a frame ID that is much older than our last completed frame, it likely
            // means the stream has restarted
            if (last_completed_frame_id - host_header.frame_id > 100) {
                spdlog::info("[UDP] Stream reset detected. Resetting buffers.");
                last_completed_frame_id = 0;
                frame_buffers.clear();
            } else {
                continue;  // Otherwise drop late packet from an old frame
            }
        }

        // Access or create the buffer for this specific frame
        auto& fb = frame_buffers[host_header.frame_id];
        if (fb.total_fragments == 0) {
            fb.total_fragments = host_header.total_fragments;
            fb.fragments.resize(host_header.total_fragments);
        }

        // Avoid counting duplicate packets
        if (fb.fragments[host_header.fragment_id].empty()) {
            // Copy just the payload data (skipping the header)
            fb.fragments[host_header.fragment_id] =
                std::vector<uint8_t>(packet.begin() + sizeof(FragmentHeader), packet.end());
            fb.received_count++;
        }

        // Check if we have received all pieces for this frame
        if (fb.received_count == fb.total_fragments) {
            // Reconstruct the full payload
            out_payload.clear();

            // Preallocate memory to speed up concatenation
            size_t total_size = 0;
            for (const auto& frag : fb.fragments) total_size += frag.size();
            out_payload.reserve(total_size);

            for (const auto& frag : fb.fragments) {
                out_payload.insert(out_payload.end(), frag.begin(), frag.end());
            }

            // Update our tracker and clean up old data
            last_completed_frame_id = host_header.frame_id;
            cleanup_old_frames(host_header.frame_id);

            return !out_payload.empty();
        }
    }

    return false;
}

void FragmentReceiver::cleanup_old_frames(uint32_t current_frame_id) {
    // If a UDP packet was dropped on a previous frame, it will never complete.
    // We must erase those old buffers to prevent memory leaks.
    for (auto it = frame_buffers.begin(); it != frame_buffers.end();) {
        if (it->first <= current_frame_id) {
            it = frame_buffers.erase(it);
        } else {
            ++it;
        }
    }
}
