// The game is called "Pick and Bomb".
// "PAB" is short for "Pick and Bomb".
#include <iostream>
#include <raylib.h>
#include "server/pabServer.h"
#include "common/pabStructs.h"
#include <chrono>

static void runServer(int port);
static void runClient(const std::string& ip, int port);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "look," << std::endl;
        std::cerr << "To Host:   " << argv[0] << " -s <port>" << std::endl;
        std::cerr << "To Join:   " << argv[0] << " -c <ip> <port>" << std::endl;
        return 1;
    }

    std::string mode = argv[1];

    try {
        if (mode == "-s") {
            if (argc < 3) {
                std::cerr << "Error: Server mode requires a port number." << std::endl;
                return 1;
            }
            int port = std::stoi(argv[2]);
            runServer(port);
        } 
        else if (mode == "-c") {
            if (argc < 4) {
                std::cerr << "Error: Client mode requires an IP address and a port." << std::endl;
                return 1;
            }
            std::string ip = argv[2];
            int port = std::stoi(argv[3]);
            runClient(ip, port);
        } 
        else {
            std::cerr << "Unknown flag: " << mode << std::endl;
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Invalid input: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

static const auto gStartTime = std::chrono::steady_clock::now();
unsigned long millis() {
    auto now = std::chrono::steady_clock::now();
    auto duration = now - gStartTime;
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

static void runServer(int port) { 
    std::cout << "Starting server on port: " << port << "..." << std::endl;
    pab::server::init();
    while (true) {
        static int tickTimeStamp = 0;
        const int tickPeriodMs = (int)((1 / (float)TICK_HZ) * 1000);
        if (millis() - tickTimeStamp > tickPeriodMs) {
            tickTimeStamp = millis();
            pab::server::tick();
        }
    }
}

static void runClient(const std::string& ip, int port) { 
    std::cout << "Connecting to server at " << ip << ":" << port << "..." << std::endl;
}
