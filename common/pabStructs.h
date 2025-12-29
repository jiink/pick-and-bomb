#pragma once
// NOTE: If you're using this header in a file that needs <raylib.h>, include
// <raylib.h> before you include this file. That's because <raymath.h> will
// define Vector2 and other types if they have not already been defined by <raylib.h>.
// This is done by checking macros that <raylib.h> sets, e.g. `RL_VECTOR2_TYPE`.
// So if you are getting multiple definitions of Vector2 or other math stuff,
// just move up your `#include <raylib.h>` line.
#include <raymath.h> 
#include <vector>
#include <cstdint>

const int TICK_HZ = 30;

struct Player
{
    bool active = false;
    Vector2 pos;
};

struct GameState {
    std::vector<Player> players;
};

struct PlayerInputState {
    uint8_t playerIdx = 0;
    Vector2 direction = Vector2 {0, 0};
    bool attack = false;
    bool attackPressed = false;
    bool attackReleased = false;
    bool wepSelectPressed = false;
    bool leftPressed = false;
    bool rightPressed = false;
};

struct InputState {
    std::vector<PlayerInputState> playerInputs;
};

// ------- Networking -------------

enum class Command {
    SNAPSHOT,
    INPUTS
};

const size_t HEADER_SIZE = 5;