#include <atomic>
#include <iostream>
#include <string>
#include <thread>

#include "net/tcp_client.h"
#include "net/udp_receiver.h"
#include "net/udp_transmitter.h"

int main(int argc, char* argv[]) {
    std::cout << "[Main] Starting laptop client...\n";

    // Create TCP client to connect with RPi and send commands
    std::string host = argc > 1 ? argv[1] : "192.168.4.21";
    TCPClient client(host);

    // Keep connecting until RPi is online
    std::cout << "[Main] Attempting to connect to RPi...\n";
    while (true) {
        if (client.connect()) {
            break;  // Success!
        }
        std::cout << "[Main] RPi not found. Retrying in 2 seconds...\n";
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    // Create UDP receiver to get video frames from RPi
    UDPReceiver receiver;

    // Spawn UDP receiver thread
    std::thread udp_thread([&receiver]() {
        std::cout << "[UDP Thread] Started listening for video frames...\n";
        while (receiver.running) {
            auto data = receiver.recv();  // blocks at most 100ms
            if (!data.empty()) {
                std::cout << "[UDP] Received video frame of " << data.size() << " bytes\n";
            }
        }
        std::cout << "[UDP Thread] Exited cleanly.\n";
    });

    // Main loop, read user commands
    std::cout << "\n[Main] Ready for commands.\n";
    std::string command;
    while (true) {
        std::cout << "Enter command (or 'exit' to quit): ";
        std::getline(std::cin, command);

        if (command == "exit") {
            break;
        }

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

    // --- GRACEFUL SHUTDOWN ---
    std::cout << "\n[Main] Shutting down...\n";

    // 4. Signal the UDP thread to stop looping
    receiver.running = false;

    // 5. Join the thread
    if (udp_thread.joinable()) {
        udp_thread.join();
    }

    // The client and receiver objects will naturally be destroyed here
    // when main() returns, cleanly closing the sockets!
    return 0;
}