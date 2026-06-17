#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>

/**
 * @brief Camera class for capturing frames from the Arducam on the RPi
 */
class Camera {
   public:
    Camera(int device_id = 0);
    ~Camera();

    bool grab_frame(cv::Mat& out_frame);

   private:
    cv::VideoCapture cap;
};