#include "pabServer.h"
#include "common/pabStructs.h"
#include "common/pabLogging.h"
#include <vector>

namespace {
    static GameState gGameState;
    static InputState gInputState;

    static void UpdatePlayer(
        Player& player,
        GameState& state,
        const PlayerInputState& pInput,
        const float dt)
    {
        const float speed = 1.0f;
        player.pos = Vector2Add(player.pos, Vector2Scale(pInput.direction, speed * dt));
    }

    static PlayerInputState FindPlayerInputs(const InputState& inputs, int playerNum) {
        for (PlayerInputState pInput : inputs.playerInputs) {
            if (pInput.playerIdx == playerNum) {
                return pInput;
            }
        }
        PAB_ERR("Couldn't find inputs for p%d", playerNum);
        PlayerInputState defaultInputs = {0};
        return defaultInputs;
    }

    static void UpdateGame(
        GameState& state, 
        InputState& inputs, 
        const float dt)
    {
        for (int pNum = 0; pNum < (int)state.players.size(); pNum++)
        {
            Player& p = state.players[pNum];
            if (!p.active) {
                continue;
            }
            PlayerInputState pInput = FindPlayerInputs(inputs, pNum);
            UpdatePlayer(p, state, pInput, dt);
        }
    } 
}

namespace pab::server {
    void Init(void) {
        PAB_INFO("Initializing game state on server");
    }

    void Tick(void) {
        static int tickNum = 0;
        tickNum++;
        const float dt = 1 / (float)TICK_HZ;
        //PAB_INFO("Tick %d of %d ms", tickNum, (int)(dt * 1000));
        UpdateGame(gGameState, gInputState, dt);
    }

    // Returns the player index of the new player
    uint8_t MakeNewPlayer() {
        Player newPlayer = {
            .active = true,
            .pos = Vector2 {0, 0}
        };
        gGameState.players.push_back(newPlayer);
        uint8_t newPlayerIdx = gGameState.players.size() - 1;
        PlayerInputState newInput = {
            .playerIdx = newPlayerIdx
        };
        gInputState.playerInputs.push_back(newInput);
        return newPlayerIdx;
    }

    void RemovePlayer(uint8_t playerNum) {
        if (playerNum >= gGameState.players.size()) {
            PAB_ERR("Tried to remove nonexistant player #%d", playerNum);
            return;
        }
        //gGameState.players.erase(gGameState.players.begin() + playerNum);
        // set it to false instead of erasing to not ruin the indexes for everyone 
        // who joined after this person
        gGameState.players[playerNum].active = false;
        auto& inputs = gInputState.playerInputs;
        for (auto it = inputs.begin(); it != inputs.end(); ++it) {
            if (it->playerIdx == playerNum) {
                inputs.erase(it);
                break; 
            }
        }
    }
}