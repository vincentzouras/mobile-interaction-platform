#include <atomic>
#include <iostream>
#include <string>
#include <thread>

#include "net/tcp_client.h"
#include "net/udp_receiver.h"
#include "net/udp_transmitter.h"

int main(int argc, char* argv[]) {
    std::cout << "[Main] Starting laptop client...\n";

    // TCP client to connect to Pi
    std::string host = argc > 1 ? argv[1] : "127.0.0.1";
    TCPClient client(host, 5000);

    // UDP receiver for video stream
    UDPReceiver receiver(5001);
    if (!receiver.start()) {
        std::cerr << "[Main] Failed to start UDP receiver\n";
        return 1;
    }

    // Spawn UDP receive thread
    std::thread udp_thread([&receiver]() {
        while (receiver.running) {
            auto data = receiver.recv();
            if (!data.empty()) {
                std::cout << "[UDP] Received data of " << data.size() << " bytes\n";
            }
        }
    });

    // Main loop, read user commands
    std::cout << "[Main] Ready for commands\n";
    std::string command;
    while (true) {
        std::cout << "Enter command (or 'exit' to exit): ";
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
                std::cout << "Response: " << std::string(response.begin(), response.end()) << "\n";
            }
        }
    }

    client.disconnect();
    receiver.stop();
    udp_thread.join();

    std::cout << "[Main] Shutting down...\n";
    return 0;
}
