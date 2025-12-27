// The game is called "Pick and Bomb".
// "PAB" is short for "Pick and Bomb".
#include <iostream>
#include <raylib.h>
#include "server/pabServer.h"
#include "common/pabStructs.h"
#include "common/pabLogging.h"
#include "server/serverHost.h"
#include "client/clientHost.h"

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#define NOGDI               // Exclude GDI (fixes Rectangle collision)
#define NOUSER              // Exclude User32 (fixes CloseWindow/ShowCursor collision)
#define ENET_IMPLEMENTATION
#include <enet/enet.h>

static bool InitENet() {
    if (enet_initialize () != 0) {
        std::cout << "An error occurred while initializing ENet." << std::endl;
        return false;
    }
    atexit(enet_deinitialize);
    return true;
}


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
            if (!InitENet()) { return 2; }
            RunServer(port);
        } 
        else if (mode == "-c") {
            if (argc < 4) {
                std::cerr << "Error: Client mode requires an IP address and a port." << std::endl;
                return 1;
            }
            std::string ip = argv[2];
            int port = std::stoi(argv[3]);
            if (!InitENet()) { return 2; }
            RunClient(ip, port);
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

