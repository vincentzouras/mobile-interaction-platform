#include <unistd.h>

#include <atomic>
#include <csignal>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>

#include "net/fragment_receiver.h"
#include "net/tcp_client.h"
#include "net/udp_receiver.h"
#include "net/udp_transmitter.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

std::atomic<bool> g_quit{false};

void init_logging() {
    auto logger = spdlog::stdout_color_mt("mip");
    spdlog::set_default_logger(logger);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::info);
}

void signal_handler(int) { g_quit = true; }

int main() {
    init_logging();

    // Register signal handler for graceful shutdown
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    spdlog::info("[Main] Starting laptop client...");

    try {
        cv::Mat shared_frame;
        std::mutex frame_mutex;

        // Spawn UDP receiver thread (only for networking)
        std::jthread udp_thread([&shared_frame, &frame_mutex]() {
            try {
                UDPReceiver udp_rx;
                FragmentReceiver frag_rx(udp_rx);

                spdlog::info("[UDP Thread] Started listening for video frames...");

                std::vector<uint8_t> jpeg_buffer;
                cv::Mat frame;

                while (!g_quit) {
                    if (frag_rx.receive_payload(jpeg_buffer)) {
                        // Decode the reassembled JPEG payload
                        frame = cv::imdecode(jpeg_buffer, cv::IMREAD_COLOR);
                        if (frame.empty()) continue;

                        std::lock_guard<std::mutex> lock(frame_mutex);
                        frame.copyTo(shared_frame);
                    }
                }
            } catch (const std::exception& e) {
                spdlog::error("[UDP Thread] Exception: {}", e.what());
            }

            spdlog::info("[UDP Thread] Exited cleanly.");
        });

        // Create TCP client to connect with RPi and send commands
        std::jthread tcp_thread([]() {
            try {
                TCPClient client;

                spdlog::info("[TCP Thread] Attempting to connect to RPi...");
                while (!g_quit) {
                    if (client.connect()) {
                        break;
                    }
                    spdlog::info("[TCP Thread] RPi not found. Retrying in 2 seconds...");
                    std::this_thread::sleep_for(std::chrono::seconds(2));
                }

                spdlog::info("[TCP Thread] Ready for commands.");
                std::string command;
                while (!g_quit) {
                    spdlog::info("[TCP Thread] Enter command: ");

                    if (!std::getline(std::cin, command)) break;
                    if (command.empty()) continue;

                    std::vector<uint8_t> data(command.begin(), command.end());

                    if (client.send(data)) {
                        std::vector<uint8_t> response = client.recv();
                        if (!response.empty()) {
                            spdlog::info("[TCP Thread] RPi Response: {}",
                                         std::string(response.begin(), response.end()));
                        }
                    } else {
                        spdlog::warn("[TCP Thread] Connection to Pi lost!");
                        break;
                    }
                }
            } catch (const std::exception& e) {
                spdlog::error("[TCP Thread] Exception: {}", e.what());
            }
            spdlog::info("[TCP Thread] Exited cleanly.");
        });

        // Main loop and GUI
        cv::Mat display_frame;
        while (!g_quit) {
            {
                // Safely extract the latest frame
                std::lock_guard<std::mutex> lock(frame_mutex);
                if (!shared_frame.empty()) {
                    shared_frame.copyTo(display_frame);
                }
            }

            if (!display_frame.empty()) {
                cv::imshow("RPi Camera Stream", display_frame);
            }

            if (cv::waitKey(33) == 27) {  // ESC pressed
                g_quit = true;
                break;
            }
        }

        cv::destroyAllWindows();
        spdlog::info("[Main] Shutting down...");

    } catch (const std::exception& e) {
        spdlog::critical("[Main] Critical Error: {}", e.what());
        return 1;
    }

    spdlog::info("[Main] Shutdown completed cleanly, bye!");
    return 0;
}