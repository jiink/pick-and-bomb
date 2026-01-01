#include <raylib.h>
#include "client/pabClient.h"
#include "common/pabStructs.h"
#include "common/pabLogging.h"
#include "common/packetBuilder.h"
#include "client/pabInputs.h"
#include <vector>
#include <deque>
#include <optional>
#include <algorithm>
#include "pabClient.h"

namespace {
    GameState gGameState;
    std::deque<Snapshot> gSnapshotBuffer;
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
    uint8_t gMyPlayerId = 0;
    const size_t MAX_PACKET_SIZE = 1000;
    const size_t MAX_PACKET_COUNT = 100;
    const float INTERPOLATION_OFFSET_S = 0.1f;
    float gClientTimeS = 0.0f;
    bool gFirstSnapshotReceived = false;

    const SnapshotPlayer* FindPlayerInSnapshot(const Snapshot& snap, uint8_t id) {
        for (const auto& p : snap.players) {
            if (p.id == id) { return &p; }
        }
        return nullptr;
    }

    Player* GetPlayer(std::unordered_map<uint8_t, Player>& players, int id) {
        auto it = players.find(id);
        if (it != players.end()) {
            return &it->second;
        }
        PAB_WARN("Player id %d not found", id);
        return nullptr;
    }

    void DrawPlayer(const Player& player) {
        if (!player.renderable) { return; }
        DrawCircleV(player.pos, 0.5f, ColorFromHSV(player.hue, 1.0f, 1.0f));
    }

    void DrawPlayers(const std::unordered_map<uint8_t, Player>& players) {
        for (auto& [id, p] : players) {
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
            << inputs.playerId << inputs.direction.x << inputs.direction.y
            << inputs.attack << inputs.attackPressed << inputs.attackReleased
            << inputs.wepSelectPressed << inputs.leftPressed << inputs.rightPressed;
        NetAddPacket(std::move(pb.buffer));
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

    void UpdateInterpolation(float dt) {
        if (gSnapshotBuffer.size() < 2) { return; }
        float newestServerTimeS = gSnapshotBuffer.back().tick * (1.0f / TICK_HZ);
        float targetClientTimeS = newestServerTimeS - INTERPOLATION_OFFSET_S;
        float error = targetClientTimeS - gClientTimeS;
        float timescale = 1.0f + (error * 5.0f);
        timescale = std::clamp(timescale, 0.9f, 1.1f);
        gClientTimeS += dt * timescale;
        // let it be so:
        // gSnapshotBuffer[0] is the previous tick
        // gSnapshotBuffer[1] is the next tick (i.e. the new/current tick to interpolate towards)
        while (gSnapshotBuffer.size() > 2 && 
            gSnapshotBuffer[1].tick * (1.0f / TICK_HZ) < gClientTimeS)
        {
                gSnapshotBuffer.pop_front();
        }
        if (gSnapshotBuffer.size() < 2) { return; }
        const Snapshot& prev = gSnapshotBuffer[0];
        const Snapshot& next = gSnapshotBuffer[1];
        float prevTimeS = prev.tick * (1.0f / TICK_HZ);
        float nextTimeS = next.tick * (1.0f / TICK_HZ);
        float duration = nextTimeS - prevTimeS;
        float alpha = 0.0f;
        if (duration > 0.0001f) {
            alpha = (gClientTimeS - prevTimeS) / duration;
        }
        alpha = std::clamp(alpha, 0.0f, 1.0f);
        // Don't show players that aren't in the snapshot
        for (auto& [id, player] : gGameState.players) {
            player.renderable = false;
        }
        // Update players that are in the new snapshot
        for (const SnapshotPlayer& nextP : next.players) {
            if (!gGameState.players.contains(nextP.id)) {
                PAB_ERR("Got snapshot for player %d out of nowhere", nextP.id);
                continue;
            }
            Player& visualP = gGameState.players[nextP.id];
            visualP.renderable = true;
            visualP.id = nextP.id;
            visualP.dead = nextP.dead;
            const SnapshotPlayer* prevP = FindPlayerInSnapshot(prev, nextP.id);
            if (prevP) {
                visualP.pos = Vector2Lerp(prevP->pos, nextP.pos, alpha);
            } else {
                visualP.pos = nextP.pos;
            }
            gGameState.players[visualP.id] = visualP;
        }
    }

    void Draw() {
        UpdateInterpolation(GetFrameTime());
        BeginMode2D(camera);
            ClearBackground(GRAY);
            DrawGameState(gGameState);
        EndMode2D();
        DrawText(TextFormat("great...\n(%.2f, %.2f)", gInputs.direction.x, gInputs.direction.y), 10, 10, 20, BLACK);
        // Player* myPlayer = GetPlayer(gGameState.players, gMyPlayerId);
        // if (myPlayer) {
        // }
    }

    void NetApplySnapshot(uint32_t serverTick, std::vector<uint8_t> data) {
        PacketReader pr(data);
        uint8_t numPlayers;
        pr >> numPlayers;
        Snapshot newSnap;
        newSnap.tick = serverTick;
        newSnap.players.reserve(numPlayers);
        for (size_t i = 0; i < numPlayers; i++) {
            SnapshotPlayer sp;
            pr >> sp.id >> sp.dead >> sp.pos.x >> sp.pos.y;
            newSnap.players.push_back(sp);
        }
        gSnapshotBuffer.push_back(newSnap);
        // sort buffer and init logic
        std::sort(gSnapshotBuffer.begin(), gSnapshotBuffer.end(), 
            [](const Snapshot& a, const Snapshot& b) { return a.tick < b.tick; });
        if (!gFirstSnapshotReceived) {
            gFirstSnapshotReceived = true;
            gClientTimeS = (newSnap.tick * (1.0f / TICK_HZ) - INTERPOLATION_OFFSET_S);
        }
    }

    void NetApplyNewPlayer(std::vector<uint8_t> data) {
        PacketReader pr(data);
        Player newP = {};
        pr >> newP.id >> newP.dead >> newP.pos >> newP.hue;
        PAB_INFO("Got info for a new player (id %d)!", newP.id);
        gGameState.players[newP.id] = newP;
    }

    void NetApplyNewPlayfield(std::vector<uint8_t> data)
    {
        PacketReader pr(data);
        Playfield pf{};
        uint32_t numCells;
        pr >> numCells;
        for (uint32_t i = 0; i < numCells; i++) {
            Cell c{};
            uint8_t cType;
            pr >> c.id >> c.pos >> cType >> c.health;
            c.type = (CellType)cType;
            // hmm....
        }
    }

    void SetPlayerId(uint8_t id)
    {
        gMyPlayerId = id;
        PAB_INFO(">>>> I AM PLAYER id %d", id);
    }
}