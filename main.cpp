// The game is called "Pick and Bomb".
// "PAB" is short for "Pick and Bomb".
#include <iostream>
#include <raylib.h>
#include "server/pabServer.h"
#include "common/pabStructs.h"
#include "common/pabLogging.h"
#include <chrono>

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#define NOGDI               // Exclude GDI (fixes Rectangle collision)
#define NOUSER              // Exclude User32 (fixes CloseWindow/ShowCursor collision)
#define ENET_IMPLEMENTATION
#include <enet/enet.h>

static void RunServer(int port);
static void RunClient(const std::string& ip, int port);

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
            RunServer(port);
        } 
        else if (mode == "-c") {
            if (argc < 4) {
                std::cerr << "Error: Client mode requires an IP address and a port." << std::endl;
                return 1;
            }
            std::string ip = argv[2];
            int port = std::stoi(argv[3]);
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

static const auto gStartTime = std::chrono::steady_clock::now();
unsigned long Millis() {
    auto now = std::chrono::steady_clock::now();
    auto duration = now - gStartTime;
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

// const char* FormatIp(enet_uint32 host) {
//     static thread_local char buffer[16];
//     sprintf(buffer, "%u.%u.%u.%u", host & 0xFF, (host >> 8) & 0xFF, (host >> 16) & 0xFF, (host >> 24) & 0xFF);
//     return buffer;
// }

const char* FormatIp(const struct in6_addr* addr) {
    static thread_local char str[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, addr, str, sizeof(str));
    return str;
}

static bool InitENet() {
    if (enet_initialize () != 0) {
        std::cout << "An error occurred while initializing ENet." << std::endl;
        return false;
    }
    atexit(enet_deinitialize);
    return true;
}

static ENetHost* CreateServerHost(int port) {
    ENetAddress address = {0};
    address.host = ENET_HOST_ANY;
    address.port = port;
    ENetHost* host = enet_host_create(&address, 32, 1, 0, 0);
    if (host == NULL) {
        PAB_ERR("failed to create ENet server host");
    }
    return host;
}

static void ProcessServerEvents(ENetHost* serverHost, uint32_t waitMs) {
    ENetEvent event;
    while (enet_host_service(serverHost, &event, waitMs) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                PAB_INFO("New client connected from %s", FormatIp(&event.peer->address.host));
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                PAB_INFO("Got packet (len %d) from client", event.packet->dataLength);
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                PAB_INFO("Client disconnected");
                break;
            case ENET_EVENT_TYPE_NONE:
                break;
            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                PAB_INFO("Client disconnected timeout");
                break;
        }
    }
}

static void RunServer(int port) { 
    if (!InitENet()) { return; }
    ENetHost* serverHost = CreateServerHost(port);
    if (serverHost == NULL) { return; }
    std::cout << "Starting server on port: " << port << "..." << std::endl;
    pab::server::init();
    static int tickTimeStamp = 0;
    const int tickPeriodMs = (int)((1 / (float)TICK_HZ) * 1000);
    while (true) {
        uint32_t timeSinceLastTick = Millis() - tickTimeStamp;
        uint32_t enetWaitTimeMs = tickPeriodMs - timeSinceLastTick;
        if (enetWaitTimeMs < 0) { enetWaitTimeMs = 0; }
        ProcessServerEvents(serverHost, enetWaitTimeMs);
        if (Millis() - tickTimeStamp > tickPeriodMs) {
            tickTimeStamp = Millis();
            pab::server::tick();
        }
    }
    enet_host_destroy(serverHost);
}

static ENetHost* CreateClientHost() {
    ENetHost* host = enet_host_create(NULL, 1, 1, 0, 0);
    if (host == NULL) {
        PAB_ERR("failed to create ENet client host");
    }
    return host;
}

static void ProcessClientEvents(ENetHost* host, bool* running) {
    const uint32_t enetWaitTimeMs = 20;
    ENetEvent event;
    while (enet_host_service(host, &event, enetWaitTimeMs) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                PAB_INFO("Got packet (len %d) from server", event.packet->dataLength);
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                PAB_INFO("Server disconnected", event.packet->dataLength);
                *running = false;
                break;
            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                PAB_INFO("Server disconnected timeout");
                break;
            case ENET_EVENT_TYPE_NONE:
                break;
            default:
                PAB_WARN("(Client) Unhandled ENet event %d", event.type);
                break;
        }
    }
}

static ENetPeer* ConnectToServer(ENetHost* clientHost, const std::string& ip, int port) {
    ENetAddress address = {0};
    enet_address_set_host(&address, ip.c_str());
    address.port = port;
    const uint32_t connectionWaitTimeMs = 5000;
    std::cout << "Connecting to server at " << ip << ":" << port << " with " << connectionWaitTimeMs << " ms timeout..." << std::endl;
    ENetPeer* peer = enet_host_connect(clientHost, &address, 1, 0);
    if (peer == NULL) {
        PAB_ERR("No peer here");
    }
    ENetEvent event;
    if (enet_host_service(clientHost, &event, connectionWaitTimeMs) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        PAB_INFO("Connected");
        return peer;
    }
    enet_peer_reset(peer);
    PAB_ERR("Couldn't connect to the server");
    return NULL;
}

static void RunClient(const std::string& ip, int port) { 
    if (!InitENet()) { return; }
    ENetHost* clientHost = CreateClientHost();
    if (clientHost == NULL) { return; }
    ENetPeer* serverPeer = ConnectToServer(clientHost, ip, port);
    if (serverPeer == NULL) {
        enet_host_destroy(clientHost);
        return;
    }
    bool running = true;
    while (running) {
        ProcessClientEvents(clientHost, &running);
    }
    enet_host_destroy(clientHost);
}
