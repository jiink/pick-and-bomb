#pragma once

#include "common/pabStructs.h"

enum class BindingAction {
  UP,
  DOWN,
  LEFT,
  RIGHT,
  ATTACK,
  MINE,
  WEP_SELECT,
  NUM_ACTIONS
};

const float DEADZONE = 0.4f;

struct KeyBind {
  BindingAction action;
  int key;
};

struct PlayerBindings {
  KeyBind bindings[static_cast<size_t>(BindingAction::NUM_ACTIONS)];
};

void InitPlayerBindings(PlayerBindings& pBindings, int playerNum);
void UpdatePlayerInputState(PlayerInputState& pInput,
                            const PlayerBindings& pBindings, int gamepadNum);
void ClearTransientPlayerInputState(PlayerInputState& pInput,
                            const PlayerBindings& pBindings, int gamepadNum);