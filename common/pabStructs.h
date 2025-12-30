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
#include <unordered_map>

const int TICK_HZ = 10;

struct Player
{
    uint8_t id = 0;
    bool dead = false;
    Vector2 pos = Vector2 {0, 0};
};

struct GameState {
    std::unordered_map<uint8_t, Player> players;
};

struct PlayerInputState {
    uint8_t playerId = 0;
    Vector2 direction = Vector2 {0, 0};
    // todo: make this a bitfield or something
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
    INPUTS,
    WELCOME
};

const size_t HEADER_SIZE = 5;

// Subset of Player
struct SnapshotPlayer {
    uint8_t id = 0;
    bool dead = true;
    Vector2 pos = Vector2 {0, 0};
};

struct Snapshot {
    uint32_t tick = 0;
    std::vector<SnapshotPlayer> players;
};
