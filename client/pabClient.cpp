#include <raylib.h>
#include "client/pabClient.h"
#include "common/pabStructs.h"
#include "common/pabLogging.h"
#include "common/packetBuilder.h"
#include "client/pabInputs.h"
#include <vector>
#include <deque>
#include <optional>
namespace {
    GameState gGameState;
    PlayerBindings gInputBindings;
    PlayerInputState gInputs;
    Camera2D camera = Camera2D {
        .offset = Vector2 {640/2, 480/2},
        .target = Vector2 {0, 0},
        .rotation = 0,
        .zoom = 50.0f
    };
    std::deque<std::vector<uint8_t>> gNetPacketsToSend;
    uint32_t gTickNum = 0;
    const size_t MAX_PACKET_SIZE = 1000;
    const size_t MAX_PACKET_COUNT = 100;

    void DrawPlayer(const Player& player) {
        DrawCircleV(player.pos, 0.5f, DARKBLUE);
    }

    void DrawPlayers(const std::vector<Player>& players) {
        for (const Player& p : players) {
            DrawPlayer(p);
        }
    }

    void DrawGameState(const GameState& gameState) {
        DrawPlayers(gameState.players);
    }

    void NetAddPacket(std::vector<uint8_t> packet) {
        if (packet.size() > MAX_PACKET_SIZE) {
            PAB_ERR("Packet too big (%d B > %d B)",
                packet.size(), MAX_PACKET_SIZE);
            return;
        }
        if (gNetPacketsToSend.size() > MAX_PACKET_COUNT) {
            PAB_ERR("Too many packets this tick (%d > %d)",
                gNetPacketsToSend.size(), MAX_PACKET_COUNT);
            return;
        }
        gNetPacketsToSend.push_back(std::move(packet));
    }

    void NetSendInputs(const PlayerInputState& inputs) {
        PacketBuilder pb;
        pb.buffer.reserve(sizeof(PlayerInputState));
        pb << (uint8_t)Command::INPUTS
            << gTickNum
            << inputs;
        NetAddPacket(pb.buffer);
    }
}

namespace pab::client {
    std::optional<std::vector<uint8_t>> ConsumePacketToSend() {
        if (gNetPacketsToSend.empty()) {
            return std::nullopt;
        }
        std::vector<uint8_t> packet = std::move(gNetPacketsToSend.front());
        gNetPacketsToSend.pop_front();
        return packet;
    }

    void Init() {
        InitPlayerBindings(gInputBindings, 0);
    }

    void Tick() {
        gTickNum++;
        UpdatePlayerInputState(gInputs, gInputBindings, 0);
        NetSendInputs(gInputs);
    }

    void Draw() {
        BeginMode2D(camera);
            ClearBackground(GRAY);
            DrawGameState(gGameState);
        EndMode2D();
        DrawText(TextFormat("great..."), 10, 10, 20, BLACK);
    }

    void ApplySnapshot(std::vector<uint8_t> data) {
        gGameState.players.clear();
        PacketReader pr(data);
        uint8_t numPlayers;
        pr >> numPlayers;
        for (size_t pI = 0; pI < numPlayers; pI++) {
            uint8_t pActiveB;
            float pX;
            float pY;
            pr >> pActiveB >> pX >> pY;
            Player p = Player {
                .active = pActiveB > 0,
                .pos = Vector2 {
                    .x = pX,
                    .y = pY
                }
            };
            gGameState.players.push_back(p);
        }
    }
}