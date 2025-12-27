#pragma once
#include <cstdint>

namespace pab::server {
    void Init();
    void Tick();
    uint8_t MakeNewPlayer();
    void RemovePlayer(uint8_t playerNum);
}
