#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include "common/pabLogging.h"

struct PacketBuilder {
  std::vector<uint8_t> buffer;

  PacketBuilder& operator<<(bool value) {
    uint8_t byteVal = value ? 1 : 0;
    return *this
           << byteVal; // Recursively calls the generic template with uint8_t
  }

  // Generic write for any simple type (int, float, structs, enums)
  template <typename T> PacketBuilder& operator<<(const T& value) {
    // Ensure we don't try to memcpy complex types like std::string
    static_assert(std::is_trivially_copyable<T>::value, "Type must be POD");

    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&value);
    buffer.insert(buffer.end(), ptr, ptr + sizeof(T));
    return *this;
  }

  // Explicit overload for raw vectors if needed
  void writeBytes(const void* data, size_t size) {
    const uint8_t* ptr = static_cast<const uint8_t*>(data);
    buffer.insert(buffer.end(), ptr, ptr + size);
  }
};

struct PacketReader {
  const std::vector<uint8_t>& buffer;
  size_t offset = 0;

  PacketReader(const std::vector<uint8_t>& buf) : buffer(buf) {}

  PacketReader& operator>>(bool& value) {
    uint8_t byteVal;
    *this >> byteVal; // Recursively calls generic template
    value = (byteVal > 0);
    return *this;
  }

  // Generic read for simple types (int, float, etc)
  template <typename T> PacketReader& operator>>(T& value) {
    static_assert(std::is_trivially_copyable<T>::value, "Type must be POD");

    if (offset + sizeof(T) > buffer.size()) {
      PAB_ERR("PacketReader buffer underflow: Packet too short!");
      std::memset(&value, 0, sizeof(T));
      return *this;
    }
    std::memcpy(&value, &buffer[offset], sizeof(T));
    offset += sizeof(T);
    return *this;
  }
};
