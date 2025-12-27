#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include "common/pabLogging.h"

struct PacketBuilder {
    std::vector<uint8_t> buffer;

    // Generic write for any simple type (int, float, structs, enums)
    template <typename T>
    PacketBuilder& operator<<(const T& value) {
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

    // Generic read for simple types (int, float, etc)
    template <typename T>
    PacketReader& operator>>(T& value) {
        static_assert(std::is_trivially_copyable<T>::value, "Type must be POD");

        if (offset + sizeof(T) > buffer.size()) {
            //throw std::runtime_error("Buffer underflow: Packet too short!");
            PAB_ERR("PacketReader buffer underflow: Packet too short!")
            return *this;
        }
        std::memcpy(&value, &buffer[offset], sizeof(T));
        offset += sizeof(T);
        return *this;
    }
};
