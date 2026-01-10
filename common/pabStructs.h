#pragma once
// NOTE: If you're using this header in a file that needs <raylib.h>, include
// <raylib.h> before you include this file. That's because <raymath.h> will
// define Vector2 and other types if they have not already been defined by
// <raylib.h>. This is done by checking macros that <raylib.h> sets, e.g.
// `RL_VECTOR2_TYPE`. So if you are getting multiple definitions of Vector2 or
// other math stuff, just move up your `#include <raylib.h>` line.
#include <raymath.h>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <array>
#include <optional>
#include "common/playfield.h"
#include <string_view>

const int TICK_HZ = 30;

struct Player {
  uint8_t id = 0;
  bool dead = false;
  Vector2 pos = Vector2{0, 0};
  float hue = 0.0f;
  Vector2 velocity = Vector2{0, 0};
  float defSpeed = 50.0f; // How fast you walk by default
  float defFriction = 0.001f;
  float friction = 0.001f;
  // client side
  bool renderable = false;
};

enum class WeaponType : uint8_t {
  BOMB = 0,
  MINE,
  SHARP_BOMB,
  ROLLER,
  GRENADE,
  NUKE,
  WEP_COUNT
};

struct Bomb {
  uint8_t id;
  uint8_t ownerId;
  WeaponType type;
  Vector2 pos;
  Vector2 velocity;
  float fuseTimer;
  float height;
  float heightVel;
  bool isStuck;
};

struct GameState {
  Playfield playfield;
  std::unordered_map<uint8_t, Player> players;
  std::unordered_map<uint8_t, Bomb> bombs;
};

struct PlayerInputState {
  uint8_t playerId = 0;
  Vector2 direction = Vector2{0, 0};
  // todo: make this a bitfield or something
  bool attack = false;
  bool attackPressed = false;
  bool attackReleased = false;
  bool wepSelectPressed = false;
  bool leftPressed = false;
  bool rightPressed = false;
  bool mine = false;
};

struct InputState {
  std::vector<PlayerInputState> playerInputs;
};



struct WeaponProperties {
  std::string_view name;
  float startingFuse;
  float damage;
  float radius;
  float friction;
  float bounciness;
};

constexpr std::array<WeaponProperties,
                     static_cast<size_t>(WeaponType::WEP_COUNT)>
    WeaponPropRegistry = {{
        [static_cast<size_t>(WeaponType::BOMB)] = {.name = "Bomb",
                              .startingFuse = 2.0f,
                              .damage = 150,
                              .radius = 6,
                              .friction = 0.7f,
                              .bounciness = 0.0f},
        [static_cast<size_t>(WeaponType::MINE)] = {.name = "Mine",
                              .startingFuse = 2.0f,
                              .damage = 150,
                              .radius = 6,
                              .friction = 0.7f,
                              .bounciness = 0.0f},
        [static_cast<size_t>(WeaponType::SHARP_BOMB)] = {.name = "Sharp Bomb",
                                    .startingFuse = 2.0f,
                                    .damage = 150,
                                    .radius = 6,
                                    .friction = 0.7f,
                                    .bounciness = 0.0f},
        [static_cast<size_t>(WeaponType::ROLLER)] = {.name = "Roller",
                                .startingFuse = 2.0f,
                                .damage = 150,
                                .radius = 6,
                                .friction = 0.7f,
                                .bounciness = 0.0f},
        [static_cast<size_t>(WeaponType::GRENADE)] = {.name = "Grenade",
                                 .startingFuse = 2.0f,
                                 .damage = 150,
                                 .radius = 6,
                                 .friction = 0.7f,
                                 .bounciness = 0.0f},
        [static_cast<size_t>(WeaponType::NUKE)] = {.name = "Nuke",
                              .startingFuse = 2.0f,
                              .damage = 150,
                              .radius = 6,
                              .friction = 0.7f,
                              .bounciness = 0.0f},
    }};

// ------- Networking -------------

enum class Command {
  SNAPSHOT,
  INPUTS,
  WELCOME,
  NEW_PLAYER,
  NEW_PLAYFIELD,
  DIRTY_CELLS,
  COMMAND_COUNT
};

struct CommandConfig {
  bool isReliable;
};

constexpr std::array<CommandConfig, static_cast<size_t>(Command::COMMAND_COUNT)>
    CommandRegistry = {{
        [static_cast<size_t>(Command::SNAPSHOT)] = {.isReliable = false},
        [static_cast<size_t>(Command::INPUTS)] = {.isReliable = false},
        [static_cast<size_t>(Command::WELCOME)] = {.isReliable = true},
        [static_cast<size_t>(Command::NEW_PLAYER)] = {.isReliable = true},
        [static_cast<size_t>(Command::NEW_PLAYFIELD)] = {.isReliable = true},
        [static_cast<size_t>(Command::DIRTY_CELLS)] = {.isReliable = false},
    }};

const size_t HEADER_SIZE = 5;

// Subset of Player
struct SnapshotPlayer {
  uint8_t id = 0;
  bool dead = true;
  Vector2 pos = Vector2{0, 0};
};

struct Snapshot {
  uint32_t tick = 0;
  std::vector<SnapshotPlayer> players;
  std::vector<Bomb> bombs;
};

struct OutgoingPacket {
  std::vector<uint8_t> data;
  std::optional<uint8_t> targetPlayerId;
};
