#include "clientHost.h"
#include <raylib.h>
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#define NOGDI               // Exclude GDI (fixes Rectangle collision)
#define NOUSER              // Exclude User32 (fixes CloseWindow/ShowCursor collision)
#include <enet/enet.h>
#include "common/pabLogging.h"
#include "common/pabStructs.h"
#include "client/pabClient.h"
#include "common/packetBuilder.h"
#include <chrono>

static ENetHost* CreateClientHost() {
    ENetHost* host = enet_host_create(NULL, 1, 1, 0, 0);
    if (host == NULL) {
        PAB_ERR("failed to create ENet client host");
    }
    return host;
}

static void ParsePacket(std::vector<uint8_t>& data) {
    if (data.size() < 1) {
        PAB_WARN("Empty packet");
        return;
    }
    uint8_t cmdId;
    uint32_t tick;
    PacketReader pr(data);
    pr >> cmdId >> tick;
    // now remove the header so the next functions don't have to deal with it.
    data.erase(data.begin(), data.begin() + HEADER_SIZE); // After this you may NOT keep using PacketReader pr.
    switch ((Command)cmdId) {
        case Command::SNAPSHOT:
            //PAB_INFO("> Got a snapshot (#%d, %d B)", tick, data.size());
            pab::client::NetApplySnapshot(tick, data);
            break;
        case Command::WELCOME: {
            PacketReader pr2(data);
            uint8_t playerId;
            pr2 >> playerId;
            pab::client::SetPlayerId(playerId);
            break;
        }
        case Command::NEW_PLAYER:
            pab::client::NetApplyNewPlayer(data);
            break;
        default:
            PAB_WARN("Unhandled command %d", cmdId);
            break;
    }
}

static void ClientProcessEvents(ENetHost* host, bool* running) {
    const uint32_t enetWaitTimeMs = 0;
    ENetEvent event;
    while (enet_host_service(host, &event, enetWaitTimeMs) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE: {
                //PAB_INFO("Got packet (len %d) from server", event.packet->dataLength);
                std::vector<uint8_t> packetAsVec(event.packet->data, event.packet->data + event.packet->dataLength);
                ParsePacket(packetAsVec);
                enet_packet_destroy(event.packet);
                break;
            }
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
    PAB_INFO("Connecting to server at %s:%d with %d ms timeout...", ip.c_str(), port, connectionWaitTimeMs);
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

static const auto gStartTime = std::chrono::steady_clock::now();
static unsigned long Millis() {
    auto now = std::chrono::steady_clock::now();
    auto duration = now - gStartTime;
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}


void RunClient(const std::string& ip, int port) { 
    ENetHost* clientHost = CreateClientHost();
    if (clientHost == NULL) { return; }
    ENetPeer* serverPeer = ConnectToServer(clientHost, ip, port);
    if (serverPeer == NULL) {
        enet_host_destroy(clientHost);
        return;
    }
    const int SCR_W = 640;
    const int SCR_H = 480;
    InitWindow(SCR_W, SCR_H, "Pick and Bomb");
    SetTargetFPS(60);
    pab::client::Init();
    bool running = true;
    while (!WindowShouldClose() && running) {
        static int tickTimeStamp = 0;
        const int tickPeriodMs = (int)((1 / (float)TICK_HZ) * 1000);
        ClientProcessEvents(clientHost, &running);
        uint32_t timeSinceLastTick = Millis() - tickTimeStamp;
        uint32_t enetWaitTimeMs = tickPeriodMs - timeSinceLastTick;
        if (enetWaitTimeMs < 0) { enetWaitTimeMs = 0; }
        if (Millis() - tickTimeStamp > tickPeriodMs) {
            tickTimeStamp = Millis();
            pab::client::Tick();
            while (auto packetDataOpt = pab::client::ConsumePacketToSend()) {
                ENetPacket* packet = enet_packet_create(
                    (*packetDataOpt).data(),
                    (*packetDataOpt).size(),
                    0
                );
                enet_peer_send(serverPeer, 0, packet);
            }
        }
        BeginDrawing();
            // ClearBackground(DARKGREEN);
            // DrawText(TextFormat("Hello %f", (float)GetTime()), 10, 10, 20, WHITE);
            pab::client::Draw();
        EndDrawing();
    }
    if (running) {
        enet_peer_disconnect(serverPeer, 0);
        enet_host_flush(clientHost);
    }
    enet_host_destroy(clientHost);
}
