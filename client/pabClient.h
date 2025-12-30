#pragma once
#include <vector>
#include <cstdint>
#include <optional>
#include <vector>

namespace pab::client {
    void Init();
    void Tick();
    void Draw();
    void ApplySnapshot(uint32_t tick, std::vector<uint8_t> data);
    std::optional<std::vector<uint8_t>> ConsumePacketToSend();
    void SetPlayerId(uint8_t id);
}