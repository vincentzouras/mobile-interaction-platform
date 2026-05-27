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
    g_quit = true;
    g_last_signal.store(signum, std::memory_order_relaxed);
    close(STDIN_FILENO);  // Unblock std::getline when we send Ctrl+C to shutdown
}

int main(int argc, char* argv[]) {
    init_logging();

    // Register signal handler for graceful shutdown
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    spdlog::info("[Main] Starting laptop client...");

    try {
        // Create UDP receiver to get video frames from RPi
        UDPReceiver udp_rx;
        ImageReceiver img_rx(udp_rx);

        // Spawn UDP receiver thread
        std::jthread udp_thread([&udp_rx, &img_rx]() {
            spdlog::info("[UDP Thread] Started listening for video frames...");
            cv::Mat frame;
            while (udp_rx.running) {
                if (img_rx.receive_image(frame)) {
                    cv::imshow("RPi Camera Stream", frame);
                }
                // Press ESC to exit
                if (cv::waitKey(1) == 27) {
                    g_quit = true;
                    break;
                }
            }
            spdlog::info("[UDP Thread] Exited cleanly.");
        });

        // Create TCP client to connect with RPi and send commands
        std::string host = argc > 1 ? argv[1] : "192.168.4.21";
        TCPClient client(host);

        while (!g_quit) {
            // STATE 1
            // Keep connecting until RPi is online
            spdlog::info("[Main] Attempting to connect to RPi...");
            while (!g_quit) {
                if (client.connect()) {
                    break;  // Success!
                }
                spdlog::info("[Main] RPi not found. Retrying in 2 seconds...");
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }

            // STATE 2
            // Main loop, read user commands
            spdlog::info("[Main] Ready for commands.");
            std::string command;
            while (!g_quit) {
                std::cout << "Command: ";
                if (!std::getline(std::cin, command)) break;

                // Convert string to bytes
                std::vector<uint8_t> data(command.begin(), command.end());
                data.push_back('\n');

                if (client.send(data)) {
                    std::vector<uint8_t> response = client.recv();
                    if (!response.empty()) {
                        spdlog::info("RPi Response: {}",
                                     std::string(response.begin(), response.end()));
                    }
                } else {
                    // If send fails, the Pi disconnected. We should probably exit or try to
                    // reconnect.
                    spdlog::warn("[Main] Connection to Pi lost!");
                    break;
                }
            }
        }

        // Graceful shutdown

        // Signal the UDP thread to stop looping
        udp_rx.running = false;

        // Wait for UDP thread to finish
        // if (udp_thread.joinable()) {
        //     udp_thread.join();
        // }
        if (g_last_signal.load(std::memory_order_relaxed) != 0) {
            spdlog::info("[Main] Interrupt signal ({}) received. Initiating shutdown...",
                         g_last_signal.load(std::memory_order_relaxed));
        }
        spdlog::info("[Main] Joining UDP thread...");
    } catch (const std::exception& e) {
        spdlog::critical("[Main] Critical Error: {}", e.what());
        return 1;
    }

    spdlog::info("[Main] Shutdown completed cleanly, bye!");
    return 0;
}