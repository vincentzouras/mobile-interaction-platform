#include "perception/camera.h"

#include "spdlog/spdlog.h"

Camera::Camera(int device_id) {
    if (cap.open(device_id, cv::CAP_V4L2)) {
        // Use raw YUYV to save CPU from decoding MJPEG. If we need more memory, switch to MJPEG
        cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('Y', 'U', 'Y', 'V'));

        cap.set(cv::CAP_PROP_FPS, 10);

        cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 800);

        spdlog::info("[Camera] Initialized Arducam on device {}", device_id);
    } else {
        spdlog::critical("[Camera] Failed to open camera hardware!");
    }
}

Camera::~Camera() { cap.release(); }

bool Camera::grab_frame(cv::Mat& out_frame) {
    if (!cap.isOpened()) return false;
    cap >> out_frame;  // If we use MJPEG, this also decodes the frame using CPU
    return !out_frame.empty();
}