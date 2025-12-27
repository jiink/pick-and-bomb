#include "pabServer.h"
#include "common/pabStructs.h"
#include "common/pabLogging.h"
#include <vector>

namespace {
    static GameState gGameState;
    static InputState gInputState;

    static void updatePlayer(
        Player& player,
        GameState& state,
        const PlayerInputState& pInput,
        const float dt)
    {
        const float speed = 1.0f;
        player.pos = Vector2Add(player.pos, Vector2Scale(pInput.direction, speed * dt));
    }

    static PlayerInputState findPlayerInputs(const InputState& inputs, int playerNum) {
        for (PlayerInputState pInput : inputs.playerInputs) {
            if (pInput.playerIdx == playerNum) {
                return pInput;
            }
        }
        PAB_ERR("Couldn't find inputs for {}", playerNum);
        PlayerInputState defaultInputs = {0};
        return defaultInputs;
    }

    static void updateGame(
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
            PlayerInputState pInput = findPlayerInputs(inputs, pNum);
            updatePlayer(p, state, pInput, dt);
        }
    } 
}

namespace pab::server {
    void init(void) {
        PAB_INFO("Initializing game state on server");
    }

    void tick(void) {
        static int tickNum = 0;
        tickNum++;
        const float dt = 1 / (float)TICK_HZ;
        //PAB_INFO("Tick %d of %d ms", tickNum, (int)(dt * 1000));
        updateGame(gGameState, gInputState, dt);
    }
}