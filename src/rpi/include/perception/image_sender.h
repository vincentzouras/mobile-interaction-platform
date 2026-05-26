#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

#include "net/config.h"
#include "net/udp_transmitter.h"

/**
 * @brief ImageSender class for encoding and sending image frames over UDP, including
 * fragmentation for large frames.
 */
class ImageSender {
   public:
    ImageSender(UDPTransmitter& transmitter);

    void send_image(const cv::Mat& frame);

   private:
    UDPTransmitter& udp_tx;
    uint32_t current_frame_id = 0;

    ImageFragmentHeader host_header;

    std::vector<uint8_t> jpeg_buffer;
    std::vector<uint8_t> packet_buffer;
    std::vector<int> compression_params = {cv::IMWRITE_JPEG_QUALITY, 80};  // 80% quality
};