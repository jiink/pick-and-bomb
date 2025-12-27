#pragma once
#include <cstdint>
#include <vector>
namespace pab::server {
    void Init();
    void Tick();
    uint8_t MakeNewPlayer();
    void RemovePlayer(uint8_t playerNum);
    std::vector<uint8_t> MakeSnapshot();
}
