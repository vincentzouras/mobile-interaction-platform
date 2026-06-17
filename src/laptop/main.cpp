#include <unistd.h>

#include <atomic>
#include <csignal>
#include <string>
#include <thread>

#include "image_receiver.h"
#include "net/tcp_client.h"
#include "net/udp_receiver.h"
#include "net/udp_transmitter.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

std::atomic<bool> g_quit{false};
std::atomic<int> g_last_signal{0};

void init_logging() {
    auto logger = spdlog::stdout_color_mt("mip");
    spdlog::set_default_logger(logger);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::info);
    spdlog::flush_every(std::chrono::milliseconds(100));
}

void signal_handler(int signum) {
    spdlog::info("[Main] Interrupt signal ({}) received. Initiating shutdown...", signum);
    g_quit = true;
    close(STDIN_FILENO);  // Unblock std::getline when we send Ctrl+C to shutdown
}

int main() {
    init_logging();

    // Register signal handler for graceful shutdown
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    spdlog::info("[Main] Starting laptop client...");

    cv::Mat shared_frame;
    std::mutex frame_mutex;

    // Spawn UDP receiver thread (only for networking)
    std::jthread udp_thread([&shared_frame, &frame_mutex]() {
        try {
            UDPReceiver udp_rx;
            ImageReceiver img_rx(udp_rx);

            spdlog::info("[UDP Thread] Started listening for video frames...");

            cv::Mat frame;

            while (!g_quit) {
                if (img_rx.receive_image(frame)) {
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
    std::jthread cmd_thread([]() {
        try {
            TCPClient client;

            spdlog::info("[Cmd Thread] Attempting to connect to RPi...");
            while (!g_quit) {
                if (client.connect()) {
                    break;
                }
                spdlog::info("[Cmd Thread] RPi not found. Retrying in 2 seconds...");
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }

            spdlog::info("[Cmd Thread] Ready for commands.");
            std::string command;
            while (!g_quit) {
                spdlog::info("\n[Cmd Thread] Enter command: ");
                if (!std::getline(std::cin, command)) break;
                if (command.empty()) continue;

                std::vector<uint8_t> data(command.begin(), command.end());
                data.push_back('\n');

                if (client.send(data)) {
                    std::vector<uint8_t> response = client.recv();
                    if (!response.empty()) {
                        spdlog::info("[Cmd Thread] RPi Response: {}\n",
                                     std::string(response.begin(), response.end()));
                    }
                } else {
                    spdlog::warn("[Cmd Thread] Connection to Pi lost!");
                    break;
                }
            }
        } catch (const std::exception& e) {
            spdlog::error("[Cmd Thread] Exception: {}", e.what());
        }
        spdlog::info("[Cmd Thread] Exited cleanly.");
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
    spdlog::info("[Main] Shutdown completed cleanly, bye!");
    return 0;
}