#include <raylib.h>
#include "client/pabClient.h"
#include "common/pabStructs.h"
#include "common/pabLogging.h"
#include "common/packetBuilder.h"

namespace {
    static GameState gGameState;
    Camera2D camera = Camera2D {
        .offset = Vector2 {640/2, 480/2},
        .target = Vector2 {0, 0},
        .rotation = 0,
        .zoom = 1.0f
    };

    void DrawPlayer(const Player& player) {
        DrawCircleV(player.pos, 10.0f, DARKBLUE);
    }

    void DrawPlayers(const std::vector<Player>& players) {
        for (const Player& p : players) {
            DrawPlayer(p);
        }
    }

    void DrawGameState(const GameState& gameState) {
        DrawPlayers(gameState.players);
    }
}

namespace pab::client {
    void Init() {
        
    }

    void Tick() {

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