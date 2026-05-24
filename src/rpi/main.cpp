#include <unistd.h>

#include <iostream>
#include <thread>
#include <vector>

#include "net/tcp_server.h"
#include "net/udp_transmitter.h"

int main(int argc, char* argv[]) {
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
            // Constantly listen for commands from laptop
            while (tcp_server.running) {
                std::vector<uint8_t> command = tcp_server.recv();  // blocks
                if (!command.empty()) {
                    std::cout << "[TCP] Received: " << std::string(command.begin(), command.end())
                              << "\n";
                    tcp_server.send(std::vector<uint8_t>({'O', 'K', '\n'}));
                }
            }
        });
        std::thread udp_thread([&udp_transmitter, &dummy_data]() {
            // Constantly send most recent camera frame to laptop
            while (udp_transmitter.running) {
                udp_transmitter.send(dummy_data);
                usleep(33000);  // ~30 FPS
            }
        });

        // Main control loop
        std::cout << "[Main] Entering control loop...\n";
        while (true) {
            std::cout << "[Main] Control loop tick...\n";
            // TODO: Perception, Planning, Motion Control
            sleep(2);
        }
    } catch (const std::exception& e) {
        std::cerr << "[Main] Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}