#include "pabServer.h"
#include "common/pabStructs.h"
#include "common/pabLogging.h"
#include <vector>
#include "common/packetBuilder.h"
#include <deque>
#include <optional>

namespace {
    GameState gGameState;
    InputState gInputState;
    uint32_t gTickNum = 0;
    uint8_t gNextPlayerId = 0;
    std::deque<std::vector<uint8_t>> gNetPacketsToSend;
    const size_t MAX_PACKET_SIZE = 1000;
    const size_t MAX_PACKET_COUNT = 100;

    Player* GetPlayer(std::unordered_map<uint8_t, Player>& players, int id) {
        auto it = players.find(id);
        if (it != players.end()) {
            return &it->second;
        }
        PAB_WARN("Player id %d not found", id);
        return nullptr;
    }

    void UpdatePlayer(
        Player& player,
        GameState& state,
        const PlayerInputState& pInput,
        const float dt)
    {
        const float speed = 2.0f;
        player.pos = Vector2Add(player.pos, Vector2Scale(pInput.direction, speed * dt));
    }

    PlayerInputState GetPlayerInputs(const InputState& inputs, int playerId) {
        for (PlayerInputState pInput : inputs.playerInputs) {
            if (pInput.playerId == playerId) {
                return pInput;
            }
        }
        PAB_ERR("Couldn't find inputs for player id %d", playerId);
        PlayerInputState defaultInputs = {0};
        return defaultInputs;
    }

    void UpdateGame(
        GameState& state, 
        InputState& inputs, 
        const float dt)
    {
        for (auto& [id, p] : state.players) {
            if (p.dead) {
                continue;
            }
            PlayerInputState pInput = GetPlayerInputs(inputs, id);
            UpdatePlayer(p, state, pInput, dt);
        }
    } 

    void ApplyPlayerInputs(const PlayerInputState& pInputs) {
        for (PlayerInputState& ogPInput : gInputState.playerInputs) {
            //PAB_INFO(">> ogPInput.playerId == playerId -> %d == %d", ogPInput.playerId, pInputs.playerId);
            if (ogPInput.playerId == pInputs.playerId) {
                ogPInput = pInputs;
                ogPInput.playerId = pInputs.playerId;
                return;
            }
        }
        PAB_ERR("Couldn't apply inputs for player id %d since inputs for that player doesn't exist", pInputs.playerId);
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

    void NetSendSnapshot() {
        PacketBuilder pb;
        pb.buffer.reserve(512);
        pb << (uint8_t)Command::SNAPSHOT
            << gTickNum
            << (uint8_t)gGameState.players.size();
        for (const auto& [id, p] : gGameState.players) {
            pb << p.id
                << (uint8_t)p.dead
                << p.pos.x
                << p.pos.y;
        }
        NetAddPacket(std::move(pb.buffer));
    }
}

namespace pab::server {
    std::optional<std::vector<uint8_t>> ConsumePacketToSend() {
        if (gNetPacketsToSend.empty()) {
            return std::nullopt;
        }
        std::vector<uint8_t> packet = std::move(gNetPacketsToSend.front());
        gNetPacketsToSend.pop_front();
        return packet;
    }

    void Init(void) {
        PAB_INFO("Initializing game state on server");
    }

    void Tick(void) {
        gTickNum++;
        const float dt = 1 / (float)TICK_HZ;
        //PAB_INFO("Tick %d of %d ms (%.1f s)", gTickNum, (int)(dt * 1000), (gTickNum * dt));
        UpdateGame(gGameState, gInputState, dt);
        NetSendSnapshot();
    }

    // Returns the player id of the new player
    uint8_t MakeNewPlayer() {
        uint8_t pId = gNextPlayerId;
        if ((gNextPlayerId + 1) < gNextPlayerId) {
            PAB_ERR("Player id overflowed. Get ready for trouble.");
        }
        gNextPlayerId++;
        gGameState.players[pId] = Player {
            .id = pId,
            .dead = false,
            .pos = Vector2 {0, 0},
            .hue = pId * 40.0f
        };
        PlayerInputState newInput = {
            .playerId = pId
        };
        gInputState.playerInputs.push_back(newInput);
        PAB_INFO("> Registered new inputs with player id %d", newInput.playerId);
        return pId;
    }

    void NetSendNewPlayer(uint8_t playerId) {
        Player* newP = GetPlayer(gGameState.players, playerId);
        if (!newP) {
            return;
        }
        PacketBuilder pb;
        pb.buffer.reserve(sizeof(Player));
        pb << (uint8_t)Command::NEW_PLAYER
            << gTickNum
            << newP->id
            << newP->dead
            << newP->pos
            << newP->hue;
        NetAddPacket(std::move(pb.buffer));
    }

    void RemovePlayer(uint8_t playerId) {
        if (!gGameState.players.contains(playerId)) {
            PAB_ERR("Tried to remove nonexistant player id %d", playerId);
            return;
        }
        gGameState.players.erase(playerId);
    }    

    void ApplyPlayerInputsFromPacket(std::vector<uint8_t> data, uint8_t playerId)
    {
        //PAB_INFO("Got inputs from player id %d", playerId);
        PlayerInputState pInputs = {};
        PacketReader pr(data);
        pr >> pInputs.playerId >> pInputs.direction.x >> pInputs.direction.y
            >> pInputs.attack >> pInputs.attackPressed >> pInputs.attackReleased
            >> pInputs.wepSelectPressed >> pInputs.leftPressed >> pInputs.rightPressed;
        pInputs.playerId = playerId;
        ApplyPlayerInputs(pInputs);
    }

    // Returns id of the new player
    uint8_t OnNewPlayerJoin() {
        uint8_t newId = MakeNewPlayer();
        NetSendNewPlayer(newId);
        return newId;
    }
}