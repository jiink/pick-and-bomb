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
#include "client/playfieldShader.h"
#include <string>

namespace {
    GameState _gameState;
    std::deque<Snapshot> _snapshotBuffer;
    PlayerBindings _inputBindings;
    PlayerInputState _inputs;
    Camera2D camera = Camera2D {
        .offset = Vector2 {10, 10},
        .target = Vector2 {0, 0},
        .rotation = 0,
        .zoom = 10.0f
    };
    std::deque<std::vector<uint8_t>> _netPacketsToSend;
    uint32_t _tickNum = 0;
    uint8_t _myPlayerId = 0;
    const size_t MAX_PACKET_SIZE = 1000;
    const size_t MAX_PACKET_COUNT = 100;
    const float INTERPOLATION_OFFSET_S = 0.1f;
    float _clientTimeS = 0.0f;
    bool _firstSnapshotReceived = false;
    Shader _playfieldShader;
    Texture _playfieldTex;
    Texture _cellPropertyTex;
    int _cellPropertyShaderLoc;

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

    void DrawPlayfield(const Playfield& playfield) {
        
        BeginShaderMode(_playfieldShader);
            SetShaderValueTexture(_playfieldShader, _cellPropertyShaderLoc, _cellPropertyTex);
            DrawTexturePro(_playfieldTex,
                Rectangle {0, 0, (float)_playfieldTex.width, (float)_playfieldTex.height},
                Rectangle {0, 0, playfield._worldWidth, playfield._worldHeight},
                Vector2 {0, 0}, 0.0f, WHITE);
        EndShaderMode();
        }

    void DrawGameState(const GameState& gameState) {
        DrawPlayfield(gameState.playfield);
        DrawPlayers(gameState.players);
    }

    void NetAddPacket(std::vector<uint8_t> packet) {
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
        _netPacketsToSend.push_back(std::move(packet));
    }

    void NetSendInputs(const PlayerInputState& inputs) {
        PacketBuilder pb;
        pb.buffer.reserve(sizeof(PlayerInputState));
        pb << (uint8_t)Command::INPUTS
            << _tickNum
            << inputs.playerId << inputs.direction.x << inputs.direction.y
            << inputs.attack << inputs.attackPressed << inputs.attackReleased
            << inputs.wepSelectPressed << inputs.leftPressed << inputs.rightPressed;
        NetAddPacket(std::move(pb.buffer));
    }

    void DebugPlayfield(Playfield& pf) {
        std::string debugStr;
        for (int y = 0; y < (int)pf._worldHeight; y++) {
            for (int x = 0; x < (int)pf._worldWidth; x++) {
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

    Image GenPlayfieldImage(Playfield& pf, int pixPerWorldUnit) {
        int imW = std::floorf(pf._worldWidth) * pixPerWorldUnit;
        int imH = std::floorf(pf._worldHeight) * pixPerWorldUnit;
        const int numComponents = 4;
        float *pixels = (float*)RL_CALLOC(imW * imH * numComponents, sizeof(float));
        double startTime = GetTime();
        #pragma omp parallel for
        for (int y = 0; y < imH; y++) {
            for (int x = 0; x < imW; x++) {
                Vector2 samplePos = Vector2 {
                    x / (float)pixPerWorldUnit,
                    y / (float)pixPerWorldUnit
                };
                Cell* cell = pf.GetCellAtWorldPos(samplePos);
                CellType cellType = CellType::AIR;
                uint16_t cellId = 0;
                if (cell) {
                    cellType = cell->type;
                    cellId = cell->id;
                }
                int idx = (y * imW + x) * numComponents;
                pixels[idx + 0] = (float)cellType;
                pixels[idx + 1] = (float)cellId;
                pixels[idx + 2] = 0.0f;
                pixels[idx + 3] = 1.0f;
            }
        }
        double endTime = GetTime();
        int elapsedMs = std::ceil((endTime - startTime) * 1000.0f);
        PAB_INFO("GenPlayfieldImage took %d ms to take %d samples",
            elapsedMs, imW * imH);
        Image pfImage = {
            .data = pixels,
            .width = imW,
            .height = imH,
            .mipmaps = 1,
            .format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32
        };
        return pfImage;
    }

    Image GenCellPropertyImage(Playfield& pf) {
        const int numComponents = 4;
        int imW = pf.GetAllCells().size();
        int imH = 1;
        float *pixels = (float*)RL_CALLOC(imW * imH * numComponents, sizeof(float));
        const int y = 0;
        for (int x = 0; x < imW; x++) {
            float cellNormalizedHp = pf.GetAllCells()[x].health / 100.0f;
            int idx = (y * imW + x) * numComponents;
            pixels[idx + 0] = cellNormalizedHp;
            pixels[idx + 1] = 0.0f;
            pixels[idx + 2] = 0.0f;
            pixels[idx + 3] = 1.0f;
        }
        Image cImage = {
            .data = pixels,
            .width = imW,
            .height = imH,
            .mipmaps = 1,
            .format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32
        };
        return cImage;
    }

    void UpdatePlayfieldTex(Playfield& pf) {
        Image newPfImg = GenPlayfieldImage(pf, 16);
        _playfieldTex = LoadTextureFromImage(newPfImg);
        Image cellPImg = GenCellPropertyImage(pf);
        _cellPropertyTex = LoadTextureFromImage(cellPImg);
        int _cellPropertyShaderLoc = GetShaderLocation(_playfieldShader, "cellProps");
    }
}

namespace pab::client {
    std::optional<std::vector<uint8_t>> ConsumePacketToSend() {
        if (_netPacketsToSend.empty()) {
            return std::nullopt;
        }
        std::vector<uint8_t> packet = std::move(_netPacketsToSend.front());
        _netPacketsToSend.pop_front();
        return packet;
    }

    void Init() {
        InitPlayerBindings(_inputBindings, 0);
        _playfieldShader = LoadShaderFromMemory(0, Shaders::playfield);
        int paletteLoc = GetShaderLocation(_playfieldShader, "palette");
        float paletteData[256 * 4];
        for (int i = 0; i < 256; i++) {
            int baseIndex = i * 4;
            if (i >= (int)CellType::MAX_CELL_TYPES) {
                paletteData[baseIndex + 0] = 1.0f;
                paletteData[baseIndex + 1] = 0.0f;
                paletteData[baseIndex + 2] = 1.0f;
                paletteData[baseIndex + 3] = 1.0f;
            }
            CellType ct = (CellType)i;
            uint8_t r, g, b, a;
            switch(ct) {
                case CellType::AIR:
                    r = 0;
                    g = 255;
                    b = 255;
                    a = 0;
                    break;
                case CellType::DIRT:
                    r = 127;
                    g = 51;
                    b = 0;
                    a = 255;
                    break;
                case CellType::STONE:
                    r = 112/2;
                    g = 98/2;
                    b = 89/2;
                    a = 255;
                    break;
                case CellType::TREASURE:
                    r = 255;
                    g = 255;
                    b = 0;
                    a = 255;
                    break;
                case CellType::WALL:
                    r = 65;
                    g = 24;
                    b = 112;
                    a = 255;
                    break;
                default:
                    r = 255;
                    g = 0;
                    b = 255;
                    a = 255;
                    break;
            }
            paletteData[baseIndex + 0] = r / 255.0f;
            paletteData[baseIndex + 1] = g / 255.0f;
            paletteData[baseIndex + 2] = b / 255.0f;
            paletteData[baseIndex + 3] = a / 255.0f;
        }
        SetShaderValueV(_playfieldShader, paletteLoc, paletteData, SHADER_UNIFORM_VEC4, 256);
        Image dummyImg = GenImageChecked(16, 16, 1, 1, RED, BLUE);
        _playfieldTex = LoadTextureFromImage(dummyImg);
    }

    void Tick() {
        _tickNum++;
        UpdatePlayerInputState(_inputs, _inputBindings, 0);
        NetSendInputs(_inputs);
        
        if (_cellPropertyTex.id > 0) {
            Image cellPImg = GenCellPropertyImage(_gameState.playfield);
            UpdateTexture(_cellPropertyTex, cellPImg.data);
            UnloadImage(cellPImg);
        }
    }

    void UpdateInterpolation(float dt) {
        if (_snapshotBuffer.size() < 2) { return; }
        float newestServerTimeS = _snapshotBuffer.back().tick * (1.0f / TICK_HZ);
        float targetClientTimeS = newestServerTimeS - INTERPOLATION_OFFSET_S;
        float error = targetClientTimeS - _clientTimeS;
        float timescale = 1.0f + (error * 5.0f);
        timescale = std::clamp(timescale, 0.9f, 1.1f);
        _clientTimeS += dt * timescale;
        // let it be so:
        // _snapshotBuffer[0] is the previous tick
        // _snapshotBuffer[1] is the next tick (i.e. the new/current tick to interpolate towards)
        while (_snapshotBuffer.size() > 2 && 
            _snapshotBuffer[1].tick * (1.0f / TICK_HZ) < _clientTimeS)
        {
                _snapshotBuffer.pop_front();
        }
        if (_snapshotBuffer.size() < 2) { return; }
        const Snapshot& prev = _snapshotBuffer[0];
        const Snapshot& next = _snapshotBuffer[1];
        float prevTimeS = prev.tick * (1.0f / TICK_HZ);
        float nextTimeS = next.tick * (1.0f / TICK_HZ);
        float duration = nextTimeS - prevTimeS;
        float alpha = 0.0f;
        if (duration > 0.0001f) {
            alpha = (_clientTimeS - prevTimeS) / duration;
        }
        alpha = std::clamp(alpha, 0.0f, 1.0f);
        // Don't show players that aren't in the snapshot
        for (auto& [id, player] : _gameState.players) {
            player.renderable = false;
        }
        // Update players that are in the new snapshot
        for (const SnapshotPlayer& nextP : next.players) {
            if (!_gameState.players.contains(nextP.id)) {
                PAB_ERR("Got snapshot for player %d out of nowhere", nextP.id);
                continue;
            }
            Player& visualP = _gameState.players[nextP.id];
            visualP.renderable = true;
            visualP.id = nextP.id;
            visualP.dead = nextP.dead;
            const SnapshotPlayer* prevP = FindPlayerInSnapshot(prev, nextP.id);
            if (prevP) {
                visualP.pos = Vector2Lerp(prevP->pos, nextP.pos, alpha);
            } else {
                visualP.pos = nextP.pos;
            }
            _gameState.players[visualP.id] = visualP;
        }
    }

    void Draw() {
        UpdateInterpolation(GetFrameTime());
        camera.offset = Vector2 { (float)GetScreenWidth() / 2.0f, (float)GetScreenHeight() / 2.0f };
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            camera.zoom += wheel * 1.0f; 
            if (camera.zoom < 1.0f) camera.zoom = 1.0f;
            if (camera.zoom > 50.0f) camera.zoom = 50.0f;
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
            Vector2 delta = GetMouseDelta();
            camera.target.x -= delta.x / camera.zoom;
            camera.target.y -= delta.y / camera.zoom;
        }
        BeginMode2D(camera);
            ClearBackground(GRAY);
            DrawGameState(_gameState);
        EndMode2D();
        DrawText(TextFormat("great...\n(%.2f, %.2f)", _inputs.direction.x, _inputs.direction.y), 10, 10, 20, BLACK);
        // Player* myPlayer = GetPlayer(_gameState.players, _myPlayerId);
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
        _snapshotBuffer.push_back(newSnap);
        // sort buffer and init logic
        std::sort(_snapshotBuffer.begin(), _snapshotBuffer.end(), 
            [](const Snapshot& a, const Snapshot& b) { return a.tick < b.tick; });
        if (!_firstSnapshotReceived) {
            _firstSnapshotReceived = true;
            _clientTimeS = (newSnap.tick * (1.0f / TICK_HZ) - INTERPOLATION_OFFSET_S);
        }
    }

    void NetApplyNewPlayer(std::vector<uint8_t> data) {
        PacketReader pr(data);
        Player newP = {};
        pr >> newP.id >> newP.dead >> newP.pos >> newP.hue;
        PAB_INFO("Got info for a new player (id %d)!", newP.id);
        _gameState.players[newP.id] = newP;
    }

    void NetApplyNewPlayfield(std::vector<uint8_t> data)
    {
        PacketReader pr(data);
        float worldWidth, worldHeight, bucketSize;
        pr >> worldWidth >> worldHeight >> bucketSize;
        _gameState.playfield = Playfield(worldWidth, worldHeight, bucketSize);
        uint32_t numCells;
        pr >> numCells;
        for (uint32_t i = 0; i < numCells; i++) {
            Cell c{};
            uint8_t cType;
            pr >> c.id >> c.pos >> cType >> c.health;
            c.type = (CellType)cType;
            _gameState.playfield.AddRawCell(c);
        }
        PAB_INFO("Applied new playfield");
        DebugPlayfield(_gameState.playfield);
        UpdatePlayfieldTex(_gameState.playfield);
    }

    void NetApplyDirtyCells(std::vector<uint8_t> data) {
        PacketReader pr(data);
        uint32_t numDirty;
        pr >> numDirty;
        int fails = 0;
        for (uint32_t i = 0; i < numDirty; i++) {
            uint16_t cellId;
            float cellHp;
            pr >> cellId >> cellHp;
            if(!_gameState.playfield.UpdateCell(cellId, cellHp)) {
                fails++;
            }
        }
        //PAB_INFO("applied %d dirty cells", numDirty);
        if (fails > 0) {
            PAB_ERR("Failed to update %d dirty cells", fails);
        }
    }

    void SetPlayerId(uint8_t id)
    {
        _myPlayerId = id;
        PAB_INFO(">>>> I AM PLAYER id %d", id);
    }
}