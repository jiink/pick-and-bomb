#pragma once
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

const float DEADZONE = 0.4f;

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    ATTACK,
    WEP_SELECT,
    NUM_ACTIONS
} BindingAction;

struct KeyBind {
    BindingAction action;
    int key;
};

struct PlayerInputState {
    uint8_t playerIdx;
    Vector2 direction;
    bool attack;
    bool attackPressed;
    bool attackReleased;
    bool wepSelectPressed;
    bool leftPressed;
    bool rightPressed;
};

struct InputState {
    std::vector<PlayerInputState> playerInputs;
};

// ------- Networking -------------

enum class Command {
    snapshot,
    bruh
};

const size_t HEADER_SIZE = 5;