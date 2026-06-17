#include "perception/image_sender.h"

#include <thread>

ImageSender::ImageSender(UDPTransmitter& transmitter) : udp_tx(transmitter) {}

void ImageSender::send_image(const cv::Mat& frame) {
    if (frame.empty()) return;

    // 1. Compress the frame to JPEG
    cv::imencode(".jpg", frame, jpeg_buffer, compression_params);

    // 2. Calculate fragment math
    uint16_t total_fragments = (jpeg_buffer.size() / net::MAX_FRAGMENT_DATA_SIZE) +
                               ((jpeg_buffer.size() % net::MAX_FRAGMENT_DATA_SIZE) != 0 ? 1 : 0);

    // 3. Send fragments
    for (uint16_t i = 0; i < total_fragments; ++i) {
        size_t offset = i * net::MAX_FRAGMENT_DATA_SIZE;
        size_t bytes_to_send =
            std::min((size_t)net::MAX_FRAGMENT_DATA_SIZE, jpeg_buffer.size() - offset);

        // Prepare the packet buffer
        packet_buffer.resize(sizeof(ImageFragmentHeader) + bytes_to_send);

        // Populate the header
        host_header.frame_id = current_frame_id;
        host_header.fragment_id = i;
        host_header.total_fragments = total_fragments;
        host_header.payload_size = bytes_to_send;

        // Create a network copy so endianess doesn't mess with our header fields
        ImageFragmentHeader network_header = host_header;
        network_header.frame_id = htonl(network_header.frame_id);
        network_header.fragment_id = htons(network_header.fragment_id);
        network_header.total_fragments = htons(network_header.total_fragments);
        network_header.payload_size = htons(network_header.payload_size);

        // Copy header into packet
        std::memcpy(packet_buffer.data(), &network_header, sizeof(ImageFragmentHeader));

        // Copy image data into packet right after the header
        std::memcpy(packet_buffer.data() + sizeof(ImageFragmentHeader), jpeg_buffer.data() + offset,
                    bytes_to_send);

        // Send using your existing UDPTransmitter
        udp_tx.send(packet_buffer);

        // Add a tiny microsecond delay to prevent UDP socket flooding
        // std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    current_frame_id++;
}