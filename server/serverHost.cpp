#include "serverHost.h"
#include <enet/enet.h>
#include "common/pabLogging.h"
#include "pabServer.h"
#include "common/pabStructs.h"
#include "server/pabServer.h"
#include <chrono>
#include <map>
#include "common/packetBuilder.h"

std::map<ENetPeer*, uint8_t> gPeerToPlayerId;

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
    if (gPeerToPlayerId.find(peer) == gPeerToPlayerId.end()) {
        PAB_WARN("Client disconnected, but it has no player to remove");
        return;
    }
    int playerNum = gPeerToPlayerId[peer];
    pab::server::RemovePlayer(playerNum);
    gPeerToPlayerId.erase(peer);
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

static void ServerProcessEvents(ENetHost* serverHost, uint32_t waitMs) {
    ENetEvent event;
    // First call: allow waiting up to waitMs, then drain with 0 timeout
    int res = enet_host_service(serverHost, &event, waitMs);
    //PAB_INFO("enet_host_service => %d (event.type=%d)", res, event.type);
    while (res > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                PAB_INFO("New client connected from %s", FormatIp(&event.peer->address.host));
                gPeerToPlayerId[event.peer] = pab::server::OnNewPlayerJoin();
                PAB_INFO("Made new player, id %d", gPeerToPlayerId[event.peer]);
                PacketBuilder pb;
                pb << (uint8_t)Command::WELCOME << (uint32_t)0 << gPeerToPlayerId[event.peer];
                ENetPacket* packet = enet_packet_create(
                    pb.buffer.data(),
                    pb.buffer.size(),
                    ENET_PACKET_FLAG_RELIABLE
                );
                enet_peer_send(event.peer, 0, packet);
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE: {
                std::vector<uint8_t> packetAsVec(event.packet->data, event.packet->data + event.packet->dataLength);
                if (gPeerToPlayerId.find(event.peer) == gPeerToPlayerId.end()) {
                    PAB_ERR("Received packet from unregistered client, disregarding.");
                    enet_packet_destroy(event.packet);
                    // handle gracefully, continue draining
                    break;
                }
                uint8_t playerNum = gPeerToPlayerId[event.peer];
                ParsePacket(packetAsVec, playerNum);
                enet_packet_destroy(event.packet);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT:
            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                PAB_INFO("Client disconnected");
                CleanUpDisconnectedClient(event.peer);
                break;
            case ENET_EVENT_TYPE_NONE:
                break;
        }

        // drain next event without waiting
        res = enet_host_service(serverHost, &event, 0);
        //PAB_INFO("enet_host_service => %d (event.type=%d)", res, event.type);
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
        int timeSinceLastTick = (int)(Millis() - tickTimeStamp);
        int enetWaitTimeMs = tickPeriodMs - timeSinceLastTick;
        if (enetWaitTimeMs > 100) { enetWaitTimeMs = 100; }
        if (enetWaitTimeMs < 0) { enetWaitTimeMs = 0; }
        ServerProcessEvents(serverHost, (uint32_t)enetWaitTimeMs);
        if (Millis() - tickTimeStamp > tickPeriodMs) {
            tickTimeStamp = Millis();
            pab::server::Tick();
            while (auto packetDataOpt = pab::server::ConsumePacketToSend()) {
                uint8_t cmd = (*packetDataOpt).data()[0];
                uint32_t packetFlags = 0;
                if (cmd < (uint8_t)Command::COMMAND_COUNT) {
                    packetFlags = CommandRegistry[cmd].isReliable ? ENET_PACKET_FLAG_RELIABLE : 0;
                } else {
                    PAB_ERR("No command registry entry for command %d", cmd);
                }
                ENetPacket* packet = enet_packet_create(
                    (*packetDataOpt).data(),
                    (*packetDataOpt).size(),
                    packetFlags
                );
                enet_host_broadcast(serverHost, 0, packet);
            }
        }
    }
    enet_host_destroy(serverHost);
}
