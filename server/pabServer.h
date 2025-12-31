#pragma once
#include <cstdint>
#include <vector>
#include <optional>
#include "common/pabStructs.h"
namespace pab::server {
    void Init();
    void Tick();
    uint8_t MakeNewPlayer();
    void RemovePlayer(uint8_t playerId);
    uint8_t OnNewPlayerJoin();
    void ApplyPlayerInputsFromPacket(std::vector<uint8_t> data, uint8_t playerId);
    std::optional<OutgoingPacket> ConsumePacketToSend();
}
