#pragma once
#include <vector>
#include <cstdint>
#include <optional>
#include <vector>

namespace pab::client {
    void Init();
    void Tick();
    void Draw();
    void ApplySnapshot(std::vector<uint8_t> data);
    std::optional<std::vector<uint8_t>> ConsumePacketToSend();
}