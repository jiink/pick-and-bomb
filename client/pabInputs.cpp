#include <raylib.h>
#include "client/pabInputs.h"

void InitPlayerBindings(PlayerBindings& pBindings, int playerNum) {
  switch (playerNum) {
  case 0:
    pBindings.bindings[static_cast<size_t>(BindingAction::UP)].key = KEY_W;
    pBindings.bindings[static_cast<size_t>(BindingAction::DOWN)].key = KEY_S;
    pBindings.bindings[static_cast<size_t>(BindingAction::LEFT)].key = KEY_A;
    pBindings.bindings[static_cast<size_t>(BindingAction::RIGHT)].key = KEY_D;
    pBindings.bindings[static_cast<size_t>(BindingAction::ATTACK)].key =
        KEY_SPACE;
    pBindings.bindings[static_cast<size_t>(BindingAction::WEP_SELECT)].key =
        KEY_Q;
    pBindings.bindings[static_cast<size_t>(BindingAction::MINE)].key = KEY_E;
    break;
  case 1:
    pBindings.bindings[static_cast<size_t>(BindingAction::UP)].key = KEY_I;
    pBindings.bindings[static_cast<size_t>(BindingAction::DOWN)].key = KEY_K;
    pBindings.bindings[static_cast<size_t>(BindingAction::LEFT)].key = KEY_J;
    pBindings.bindings[static_cast<size_t>(BindingAction::RIGHT)].key = KEY_L;
    pBindings.bindings[static_cast<size_t>(BindingAction::ATTACK)].key = KEY_O;
    pBindings.bindings[static_cast<size_t>(BindingAction::WEP_SELECT)].key =
        KEY_U;
    pBindings.bindings[static_cast<size_t>(BindingAction::MINE)].key = KEY_E;
    break;
  case 2:
    pBindings.bindings[static_cast<size_t>(BindingAction::UP)].key = KEY_UP;
    pBindings.bindings[static_cast<size_t>(BindingAction::DOWN)].key = KEY_DOWN;
    pBindings.bindings[static_cast<size_t>(BindingAction::LEFT)].key = KEY_LEFT;
    pBindings.bindings[static_cast<size_t>(BindingAction::RIGHT)].key =
        KEY_RIGHT;
    pBindings.bindings[static_cast<size_t>(BindingAction::ATTACK)].key =
        KEY_RIGHT_SHIFT;
    pBindings.bindings[static_cast<size_t>(BindingAction::WEP_SELECT)].key =
        KEY_RIGHT_CONTROL;
    pBindings.bindings[static_cast<size_t>(BindingAction::MINE)].key = KEY_E;
    break;
  default:
    pBindings.bindings[static_cast<size_t>(BindingAction::UP)].key = KEY_NULL;
    pBindings.bindings[static_cast<size_t>(BindingAction::DOWN)].key = KEY_NULL;
    pBindings.bindings[static_cast<size_t>(BindingAction::LEFT)].key = KEY_NULL;
    pBindings.bindings[static_cast<size_t>(BindingAction::RIGHT)].key =
        KEY_NULL;
    pBindings.bindings[static_cast<size_t>(BindingAction::ATTACK)].key =
        KEY_NULL;
    pBindings.bindings[static_cast<size_t>(BindingAction::WEP_SELECT)].key =
        KEY_NULL;
    pBindings.bindings[static_cast<size_t>(BindingAction::MINE)].key = KEY_NULL;
    break;
  }
}

void UpdatePlayerInputState(PlayerInputState& pInput,
                            const PlayerBindings& pBindings, int gamepadNum) {
  pInput.direction = {0, 0};
  if (IsKeyDown(
          pBindings.bindings[static_cast<size_t>(BindingAction::UP)].key)) {
    pInput.direction.y -= 1;
  }
  if (IsKeyDown(
          pBindings.bindings[static_cast<size_t>(BindingAction::DOWN)].key)) {
    pInput.direction.y += 1;
  }
  if (IsKeyDown(
          pBindings.bindings[static_cast<size_t>(BindingAction::LEFT)].key)) {
    pInput.direction.x -= 1;
  }
  if (IsKeyDown(
          pBindings.bindings[static_cast<size_t>(BindingAction::RIGHT)].key)) {
    pInput.direction.x += 1;
  }
  if (IsGamepadAvailable(gamepadNum)) {
    Vector2 stickRawInput =
        Vector2{GetGamepadAxisMovement(gamepadNum, GAMEPAD_AXIS_LEFT_X),
                GetGamepadAxisMovement(gamepadNum, GAMEPAD_AXIS_LEFT_Y)};
    if (Vector2Length(stickRawInput) > DEADZONE) {
      pInput.direction.x +=
          GetGamepadAxisMovement(gamepadNum, GAMEPAD_AXIS_LEFT_X);
      pInput.direction.y +=
          GetGamepadAxisMovement(gamepadNum, GAMEPAD_AXIS_LEFT_Y);
    }
  }
  pInput.direction = Vector2Normalize(pInput.direction);
  pInput.attack = IsKeyDown(
      pBindings.bindings[static_cast<size_t>(BindingAction::ATTACK)].key);
  pInput.mine = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
  if (IsGamepadAvailable(gamepadNum)) {
    pInput.attack |=
        IsGamepadButtonDown(gamepadNum, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    pInput.mine |=
        IsGamepadButtonDown(gamepadNum, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
  }
  if (IsKeyPressed(
          pBindings.bindings[static_cast<size_t>(BindingAction::ATTACK)].key)) {
    pInput.attackPressed = true;
  }
  if (IsKeyReleased(
          pBindings.bindings[static_cast<size_t>(BindingAction::ATTACK)].key)) {
    pInput.attackReleased = true;
  }
  if (IsKeyPressed(
          pBindings.bindings[static_cast<size_t>(BindingAction::WEP_SELECT)]
              .key)) {
    pInput.wepSelectPressed = true;
  }
  if (IsKeyPressed(
          pBindings.bindings[static_cast<size_t>(BindingAction::LEFT)].key)) {
    pInput.leftPressed = true;
  }
  if (IsKeyPressed(
          pBindings.bindings[static_cast<size_t>(BindingAction::RIGHT)].key)) {
    pInput.rightPressed = true;
  }
  if (IsGamepadAvailable(gamepadNum)) {
    pInput.attackPressed =
        pInput.attackPressed ||
        IsGamepadButtonPressed(gamepadNum, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    pInput.attackReleased =
        pInput.attackReleased ||
        IsGamepadButtonReleased(gamepadNum, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    pInput.wepSelectPressed =
        pInput.wepSelectPressed ||
        IsGamepadButtonPressed(gamepadNum, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
    pInput.leftPressed =
        pInput.leftPressed ||
        IsGamepadButtonPressed(gamepadNum, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
    pInput.rightPressed =
        pInput.rightPressed ||
        IsGamepadButtonPressed(gamepadNum, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
  }
}

void ClearTransientPlayerInputState(PlayerInputState& pInput,
                                    const PlayerBindings& pBindings,
                                    int gamepadNum) {
  pInput.attackPressed = false;
  pInput.attackReleased = false;
  pInput.wepSelectPressed = false;
  pInput.leftPressed = false;
  pInput.rightPressed = false;
}