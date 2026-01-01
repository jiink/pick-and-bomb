#include "pabServer.h"
#include "common/pabStructs.h"
#include "common/pabLogging.h"
#include <vector>
#include "common/packetBuilder.h"
#include <deque>
#include <optional>
#include <string>

namespace {
    GameState _gameState;
    InputState _inputState;
    uint32_t _tickNum = 0;
    uint8_t _nextPlayerId = 0;
    std::deque<OutgoingPacket> _netPacketsToSend;
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
    
    void DebugPlayfield(Playfield& pf) {
        std::string debugStr;
        for (int y = 0; y < 50; y++) {
            for (int x = 0; x < 50; x++) {
                Vector2 samplePt = Vector2 {(float)x, (float)y};
                Cell* cell = pf.GetCellAtWorldPos(samplePt);
                if (cell == nullptr) {
                    debugStr += 'X';
                } else {
                    int cellTypeAsInt = (int)(cell->type);
                    debugStr += std::to_string(cellTypeAsInt);
                }
            }
            debugStr += '\n';
        }
        PAB_INFO("%s", debugStr.c_str());
    }

    void SetupPlayfield(GameState& state) {
        Playfield newPField = Playfield(50.0f, 50.0f, 10.0f);
        newPField.AddCell({9, 7}, CellType::DIRT);
        newPField.AddCell({36, 13}, CellType::STONE);
        newPField.AddCell({11, 21}, CellType::TREASURE);
        newPField.AddCell({43, 25}, CellType::WALL);
        newPField.AddCell({27, 29}, CellType::AIR);
        newPField.AddCell({17, 37}, CellType::DIRT);
        newPField.AddCell({41, 41}, CellType::DIRT);
        state.playfield = newPField;
        PAB_INFO("Playfield set up");
        //DebugPlayfield(state.playfield);
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
        for (PlayerInputState& ogPInput : _inputState.playerInputs) {
            //PAB_INFO(">> ogPInput.playerId == playerId -> %d == %d", ogPInput.playerId, pInputs.playerId);
            if (ogPInput.playerId == pInputs.playerId) {
                ogPInput = pInputs;
                ogPInput.playerId = pInputs.playerId;
                return;
            }
        }
        PAB_ERR("Couldn't apply inputs for player id %d since inputs for that player doesn't exist", pInputs.playerId);
    }

    void NetAddPacket(std::vector<uint8_t> packet, std::optional<uint8_t> targetId = std::nullopt) {
        if (packet.size() > MAX_PACKET_SIZE) {
            PAB_ERR("Packet too big (%d B > %d B)",
                packet.size(), MAX_PACKET_SIZE);
            return;
        }
        if (_netPacketsToSend.size() > MAX_PACKET_COUNT) {
            PAB_ERR("Too many packets this tick (%d > %d)",
                _netPacketsToSend.size(), MAX_PACKET_COUNT);
            return;
        }
        _netPacketsToSend.push_back({ std::move(packet), targetId });
    }

    void NetSendWelcome(uint8_t playerId) {
        PacketBuilder pb;
        pb << (uint8_t)Command::WELCOME << uint32_t(0) << playerId;
        NetAddPacket(std::move(pb.buffer), playerId);
    }

    void NetSendSnapshot() {
        PacketBuilder pb;
        pb.buffer.reserve(512);
        pb << (uint8_t)Command::SNAPSHOT
            << _tickNum
            << (uint8_t)_gameState.players.size();
        for (const auto& [id, p] : _gameState.players) {
            pb << p.id
                << (uint8_t)p.dead
                << p.pos.x
                << p.pos.y;
        }
        NetAddPacket(std::move(pb.buffer));
    }

    void NetSendNewPlayer(uint8_t playerId, std::optional<uint8_t> targetId) {
        Player* newP = GetPlayer(_gameState.players, playerId);
        if (!newP) {
            return;
        }
        PacketBuilder pb;
        pb.buffer.reserve(sizeof(Player));
        pb << (uint8_t)Command::NEW_PLAYER
            << _tickNum
            << newP->id
            << newP->dead
            << newP->pos
            << newP->hue;
        NetAddPacket(std::move(pb.buffer), targetId);
    }

    void NetSendNewPlayfield(std::optional<uint8_t> targetId) {
        const Playfield& pf = _gameState.playfield;
        const std::vector<Cell>& cells = pf.GetAllCells();
        if (!pf.IsInitialized()) {
            PAB_ERR("Can't send uninitialized playfield");
            return;
        }
        PacketBuilder pb;
        size_t estimatedSize = 5 + 4 + (cells.size() * (sizeof(uint16_t) + sizeof(float) * 3 + 1)); 
        pb.buffer.reserve(estimatedSize);
        pb << (uint8_t)Command::NEW_PLAYFIELD
            << _tickNum
            << pf._worldWidth
            << pf._worldHeight
            << pf._bucketSize
            << (uint32_t)cells.size();
        for (const auto& cell : cells) {
            pb << cell.id
                << cell.pos
                << (uint8_t)cell.type
                << cell.health;
        }
        NetAddPacket(std::move(pb.buffer), targetId);
    }
}

namespace pab::server {
    std::optional<OutgoingPacket> ConsumePacketToSend() {
        if (_netPacketsToSend.empty()) {
            return std::nullopt;
        }
        OutgoingPacket out = {
            .data = std::move(_netPacketsToSend.front().data),
            .targetPlayerId = _netPacketsToSend.front().targetPlayerId
        };
        _netPacketsToSend.pop_front();
        return out;
    }

    void Init(void) {
        PAB_INFO("Initializing game state on server");
        SetupPlayfield(_gameState);
    }

    void Tick(void) {
        _tickNum++;
        const float dt = 1 / (float)TICK_HZ;
        //PAB_INFO("Tick %d of %d ms (%.1f s)", _tickNum, (int)(dt * 1000), (_tickNum * dt));
        UpdateGame(_gameState, _inputState, dt);
        NetSendSnapshot();
    }

    // Returns the player id of the new player
    uint8_t MakeNewPlayer() {
        uint8_t pId = _nextPlayerId;
        if ((_nextPlayerId + 1) < _nextPlayerId) {
            PAB_ERR("Player id overflowed. Get ready for trouble.");
        }
        _nextPlayerId++;
        _gameState.players[pId] = Player {
            .id = pId,
            .dead = false,
            .pos = Vector2 {0, 0},
            .hue = pId * 40.0f
        };
        PlayerInputState newInput = {
            .playerId = pId
        };
        _inputState.playerInputs.push_back(newInput);
        PAB_INFO("> Registered new inputs with player id %d", newInput.playerId);
        return pId;
    }

    void RemovePlayer(uint8_t playerId) {
        if (!_gameState.players.contains(playerId)) {
            PAB_ERR("Tried to remove nonexistant player id %d", playerId);
            return;
        }
        _gameState.players.erase(playerId);
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
        NetSendWelcome(newId);
        NetSendNewPlayer(newId, std::nullopt);
        // The new player needs to know about all existing players, not just himself
        for (const auto& [existingId, p] : _gameState.players) {
            if (existingId == newId) { continue; }
            NetSendNewPlayer(existingId, newId);
        }
        NetSendNewPlayfield(newId);
        return newId;
    }
}