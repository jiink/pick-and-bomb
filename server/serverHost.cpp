#include "serverHost.h"
#include <enet/enet.h>
#include "common/pabLogging.h"
#include "pabServer.h"
#include "common/pabStructs.h"
#include "server/pabServer.h"
#include <chrono>
#include <map>
#include "common/packetBuilder.h"

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

static void ParsePacket(std::vector<uint8_t>& data, uint8_t playerNum) {
    if (data.size() < 1) {
        PAB_WARN("Empty packet");
        return;
    }
    uint8_t cmdId;
    uint32_t tick;
    PacketReader pr(data);
    pr >> cmdId >> tick;
    // now remove the header so the next functions don't have to deal with it
    data.erase(data.begin(), data.begin() + HEADER_SIZE);
    switch ((Command)cmdId) {
        case Command::INPUTS:
            pab::server::ApplyPlayerInputsFromPacket(data, playerNum);
            break;
        default:
            PAB_WARN("Unhandled command %d", cmdId);
            break;
    }
}

static void ProcessEventsFromServer(ENetHost* serverHost, uint32_t waitMs) {
    ENetEvent event;
    while (enet_host_service(serverHost, &event, waitMs) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                PAB_INFO("New client connected from %s", FormatIp(&event.peer->address.host));
                gPeerToPlayerNum[event.peer] = pab::server::MakeNewPlayer();
                PAB_INFO("Made new player, num %d", gPeerToPlayerNum[event.peer]);
                break;
            case ENET_EVENT_TYPE_RECEIVE: {
                //PAB_INFO("Got packet (len %d) from client", event.packet->dataLength);
                std::vector<uint8_t> packetAsVec(event.packet->data, event.packet->data + event.packet->dataLength);
                if (gPeerToPlayerNum.find(event.peer) == gPeerToPlayerNum.end()) {
                    PAB_ERR("Received packet from unregistered client, disregarding.");
                    enet_packet_destroy(event.packet);
                    return;
                }
                uint8_t playerNum = gPeerToPlayerNum[event.peer];
                ParsePacket(packetAsVec, playerNum);
                enet_packet_destroy(event.packet);
                break;
            }
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
static unsigned long Millis() {
    auto now = std::chrono::steady_clock::now();
    auto duration = now - gStartTime;
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

void RunServer(int port) { 
    ENetHost* serverHost = CreateServerHost(port);
    if (serverHost == NULL) { return; }
    PAB_INFO("Starting server on port: %d...", port);
    pab::server::Init();
    while (true) {
        static int tickTimeStamp = 0;
        const int tickPeriodMs = (int)((1 / (float)TICK_HZ) * 1000);
        uint32_t timeSinceLastTick = Millis() - tickTimeStamp;
        uint32_t enetWaitTimeMs = tickPeriodMs - timeSinceLastTick;
        if (enetWaitTimeMs < 0) { enetWaitTimeMs = 0; }
        ProcessEventsFromServer(serverHost, enetWaitTimeMs);
        if (Millis() - tickTimeStamp > tickPeriodMs) {
            tickTimeStamp = Millis();
            pab::server::Tick();
            // todo get a list of all packets the server wants to send?
            auto snapshot = pab::server::MakeSnapshot();
            if (!snapshot.empty()) {
                ENetPacket* packet = enet_packet_create(
                    snapshot.data(),
                    snapshot.size(),
                    0
                );
                enet_host_broadcast(serverHost, 0, packet);
            }
        }
    }
    enet_host_destroy(serverHost);
}
