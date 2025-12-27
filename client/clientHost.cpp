#include "clientHost.h"
#include <enet/enet.h>
#include "common/pabLogging.h"

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

void RunClient(const std::string& ip, int port) { 
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
