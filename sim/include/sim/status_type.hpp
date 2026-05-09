#pragma once

#include <cstdint>

namespace sim {

enum class StatusType : uint8_t {
    None = 0,
    Chill,
    Ignite,
    Poison,
    Stun,
};

}