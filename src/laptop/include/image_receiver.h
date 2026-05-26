#pragma once
#include <opencv2/opencv.hpp>
#include <unordered_map>
#include <vector>

#include "net/config.h"
#include "net/udp_receiver.h"

class ImageReceiver {
   public:
    ImageReceiver(UDPReceiver& receiver);

    bool receive_image(cv::Mat& out_frame);

   private:
    UDPReceiver& udp_rx;

    // Structure to hold incomplete frames as they arrive
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