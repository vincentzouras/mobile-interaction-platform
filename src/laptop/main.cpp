#include <atomic>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>

#include "net/tcp_client.h"
#include "net/udp_receiver.h"
#include "net/udp_transmitter.h"

std::atomic<bool> g_quit{false};

void signal_handler(int signum) {
    std::cout << "\n[Main] Interrupt signal (" << signum << ") received. Initiating shutdown...\n";
    g_quit = true;
    close(STDIN_FILENO);  // Unblock std::getline when we send Ctrl+C to shutdown
}

int main(int argc, char* argv[]) {
    // Register signal handler for graceful shutdown
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::cout << "[Main] Starting laptop client...\n";

    try {
        // Create UDP receiver to get video frames from RPi
        UDPReceiver receiver;

        // Spawn UDP receiver thread
        std::jthread udp_thread([&receiver]() {
            std::cout << "[UDP Thread] Started listening for video frames...\n";
            while (receiver.running) {
                auto data = receiver.recv();  // blocks at most 100ms
                if (!data.empty()) {
                    std::cout << "[UDP] Received video frame of " << data.size() << " bytes\n";
                }
            }
            std::cout << "[UDP Thread] Exited cleanly.\n";
        });

        // Create TCP client to connect with RPi and send commands
        std::string host = argc > 1 ? argv[1] : "192.168.4.21";
        TCPClient client(host);

        // Keep connecting until RPi is online
        std::cout << "[Main] Attempting to connect to RPi...\n";
        while (!g_quit) {
            if (client.connect()) {
                break;  // Success!
            }
            std::cout << "[Main] RPi not found. Retrying in 2 seconds...\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }

        // Main loop, read user commands
        std::cout << "\n[Main] Ready for commands.\n";
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
                    std::cout << "RPi Response: " << std::string(response.begin(), response.end())
                              << "\n";
                }
            } else {
                // If send fails, the Pi disconnected. We should probably exit or try to reconnect.
                std::cout << "[Main] Connection to Pi lost!\n";
                break;
            }
        }

        // Graceful shutdown

        // Signal the UDP thread to stop looping
        receiver.running = false;

        // Wait for UDP thread to finish
        // if (udp_thread.joinable()) {
        //     udp_thread.join();
        // }

        std::cout << "[Main] Joining UDP thread...\n";
    } catch (const std::exception& e) {
        std::cerr << "[Main] Critical Error: " << e.what() << "\n";
        return 1;
    }

    std::cout << "[Main] Shutdown completed cleanly, bye!\n";
    return 0;
}