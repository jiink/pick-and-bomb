#include "serverHost.h"
#include <enet/enet.h>
#include "common/pabLogging.h"
#include "pabServer.h"
#include "common/pabStructs.h"
#include "server/pabServer.h"
#include <chrono>
#include <map>

std::map<ENetPeer*, uint8_t> gPeerToPlayerNum;

const char* FormatIp(const struct in6_addr* addr) {
    static thread_local char str[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, addr, str, sizeof(str));
    return str;
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

static void CleanUpDisconnectedClient(ENetPeer* peer) {
    if (gPeerToPlayerNum.find(peer) == gPeerToPlayerNum.end()) {
        PAB_WARN("Client disconnected, but it has no player to remove");
        return;
    }
    int playerNum = gPeerToPlayerNum[peer];
    pab::server::RemovePlayer(playerNum);
    gPeerToPlayerNum.erase(peer);
    PAB_INFO("Cleaned up player #%d", playerNum);
}

static void ProcessServerEvents(ENetHost* serverHost, uint32_t waitMs) {
    ENetEvent event;
    while (enet_host_service(serverHost, &event, waitMs) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                PAB_INFO("New client connected from %s", FormatIp(&event.peer->address.host));
                gPeerToPlayerNum[event.peer] = pab::server::MakeNewPlayer();
                PAB_INFO("Made new player, num %d", gPeerToPlayerNum[event.peer]);
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                PAB_INFO("Got packet (len %d) from client", event.packet->dataLength);
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                PAB_INFO("Client disconnected");
                CleanUpDisconnectedClient(event.peer);
                break;
            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                PAB_INFO("Client disconnected timeout");
                CleanUpDisconnectedClient(event.peer);
                break;
            case ENET_EVENT_TYPE_NONE:
                break;
        }
    }
}

static const auto gStartTime = std::chrono::steady_clock::now();
unsigned long Millis() {
    auto now = std::chrono::steady_clock::now();
    auto duration = now - gStartTime;
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

void RunServer(int port) { 
    ENetHost* serverHost = CreateServerHost(port);
    if (serverHost == NULL) { return; }
    PAB_INFO("Starting server on port: %d...", port);
    pab::server::Init();
    static int tickTimeStamp = 0;
    const int tickPeriodMs = (int)((1 / (float)TICK_HZ) * 1000);
    while (true) {
        uint32_t timeSinceLastTick = Millis() - tickTimeStamp;
        uint32_t enetWaitTimeMs = tickPeriodMs - timeSinceLastTick;
        if (enetWaitTimeMs < 0) { enetWaitTimeMs = 0; }
        ProcessServerEvents(serverHost, enetWaitTimeMs);
        if (Millis() - tickTimeStamp > tickPeriodMs) {
            tickTimeStamp = Millis();
            pab::server::Tick();
        }
    }
    enet_host_destroy(serverHost);
}
