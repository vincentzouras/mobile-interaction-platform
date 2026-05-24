#include <unistd.h>

#include <csignal>
#include <iostream>
#include <thread>
#include <vector>

#include "net/tcp_server.h"
#include "net/udp_transmitter.h"

std::atomic<bool> g_quit{false};

void signal_handler(int signum) {
    std::cout << "\n[Main] Interrupt signal (" << signum << ") received. Initiating shutdown...\n";
    g_quit = true;
}

int main(int argc, char* argv[]) {
    // Register signal handler for graceful shutdown
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::cout << "[Main] Starting RPi controller...\n";

    try {
        // Initialize TCP server and UDP unicast
        TCPServer tcp_server(5000);  // throws
        std::string host = argc > 1 ? argv[1] : "127.0.0.1";
        UDPTransmitter udp_transmitter(host, 5001);  // throws

        // Dummy frame buffer (will be replaced with actual camera frames)
        std::vector<uint8_t> dummy_data(1024, 0xFF);

        // Spawn communication threads
        std::thread tcp_thread([&tcp_server]() {
            std::cout << "[TCP Thread] Started.\n";
            // Constantly listen for commands from laptop
            while (tcp_server.running) {
                std::vector<uint8_t> command = tcp_server.recv();  // blocks
                if (!command.empty()) {
                    std::cout << "[TCP Thread] Received: "
                              << std::string(command.begin(), command.end()) << "\n";
                    tcp_server.send(std::vector<uint8_t>({'O', 'K', '\n'}));
                }
            }
            std::cout << "[TCP Thread] Stopped.\n";
        });
        std::thread udp_thread([&udp_transmitter, &dummy_data]() {
            std::cout << "[UDP Thread] Started.\n";
            // Constantly send most recent camera frame to laptop
            while (udp_transmitter.running) {
                udp_transmitter.send(dummy_data);
                std::this_thread::sleep_for(std::chrono::milliseconds(33));  // ~30 FPS
            }
            std::cout << "[UDP Thread] Stopped.\n";
        });

        // Main control loop
        std::cout << "[Main] Entering main control loop (Ctrl+C to stop)...\n";
        while (!g_quit) {
            std::cout << "[Main] Control loop tick...\n";
            // TODO: Perception, Planning, Motion Control
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        // Graceful shutdown

        std::cout << "[Main] Breaking out of control loop. Signaling threads...\n";

        // Tell threads to stop looping
        tcp_server.running = false;
        udp_transmitter.running = false;

        // Wait for threads to finish (guaranteed to exit within 100ms due to socket timeouts)
        if (tcp_thread.joinable()) {
            tcp_thread.join();
        }
        if (udp_thread.joinable()) {
            udp_thread.join();
        }
        std::cout << "[Main] All threads joined. Exiting...\n";
    } catch (const std::exception& e) {
        std::cerr << "[Main] Critical Error: " << e.what() << "\n";
        return 1;
    }

    std::cout << "[Main] Shutdown completed cleanly, bye!\n";
    return 0;
}