#pragma once

#include <cstdint>

namespace sim {

class Sim;

[[nodiscard]] uint64_t hash_world_state(const Sim& sim) noexcept;

}  // namespace sim