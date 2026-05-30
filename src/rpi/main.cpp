#include <unistd.h>

#include <csignal>
#include <thread>
#include <vector>

#include "net/tcp_server.h"
#include "net/udp_transmitter.h"
#include "perception/camera.h"
#include "perception/image_sender.h"
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
}

void signal_handler(int signum) {
    g_quit = true;
    g_last_signal.store(signum, std::memory_order_relaxed);
}

int main() {
    init_logging();

    // Register signal handler for graceful shutdown
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    spdlog::info("[Main] Starting RPi controller...");

    try {
        // Initialize TCP server and UDP unicast
        TCPServer tcp_server;   // throws
        UDPTransmitter udp_tx;  // throws

        // Spawn communication threads
        std::jthread tcp_thread([&tcp_server]() {
            spdlog::info("[TCP Thread] Started.");
            // Constantly listen for commands from laptop
            while (tcp_server.running) {
                std::vector<uint8_t> command = tcp_server.recv();  // blocks at most 100ms
                if (!command.empty()) {
                    spdlog::info("[TCP Thread] Received: {}",
                                 std::string(command.begin(), command.end()));
                    tcp_server.send(std::vector<uint8_t>({'O', 'K', '\n'}));
                }
            }
            spdlog::info("[TCP Thread] Stopped.");
        });
        std::jthread udp_thread([&udp_tx]() {
            spdlog::info("[UDP Thread] Started.");

            Camera camera;
            cv::Mat frame;

            ImageSender img_sender(udp_tx);

            // Constantly send most recent camera frame to laptop
            while (udp_tx.running) {
                auto start = std::chrono::steady_clock::now();
                if (camera.grab_frame(frame)) {
                    auto end = std::chrono::steady_clock::now();
                    auto elapsed =
                        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                    spdlog::debug("[UDP Thread] grab_frame took: {} ms", elapsed);
                    start = std::chrono::steady_clock::now();

                    img_sender.send_image(frame);  // SEND IMAGE

                    end = std::chrono::steady_clock::now();
                    elapsed =
                        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                    spdlog::debug("[UDP Thread] send_image took: {} ms", elapsed);
                } else {
                    spdlog::warn("[UDP Thread] Failed to grab frame from camera.");
                }

                // std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            spdlog::info("[UDP Thread] Stopped.");
        });

        // Main control loop
        spdlog::info("[Main] Entering main control loop...");
        int dot_state = 0;  // dots heartbeat
        while (!g_quit) {
            // dots heartbeat
            dot_state = (dot_state % 3) + 1;  // 1..3
            std::string dots(dot_state, '.');
            spdlog::info("[Main] Control loop tick{}", dots);

            // TODO: Perception, Planning, Motion Control

            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        // Graceful shutdown

        // Tell threads to stop looping
        tcp_server.running = false;
        udp_tx.running = false;

        // Wait for threads to finish (guaranteed to exit within 100ms due to socket timeouts)
        // if (tcp_thread.joinable()) {
        //     tcp_thread.join();
        // }
        // if (udp_thread.joinable()) {
        //     udp_thread.join();
        // }
        if (g_last_signal.load(std::memory_order_relaxed) != 0) {
            spdlog::info("[Main] Interrupt signal ({}) received. Initiating shutdown...",
                         g_last_signal.load(std::memory_order_relaxed));
        }
        spdlog::info("[Main] Joining threads...");
    } catch (const std::exception& e) {
        spdlog::critical("[Main] Critical Error: {}", e.what());
        return 1;
    }

    spdlog::info("[Main] Shutdown completed cleanly, bye!");
    return 0;
}