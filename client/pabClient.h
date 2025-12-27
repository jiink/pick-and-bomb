#pragma once
#include <vector>
#include <cstdint>

namespace pab::client {
    void Init();
    void Tick();
    void Draw();
    void ApplySnapshot(std::vector<uint8_t> data);
}