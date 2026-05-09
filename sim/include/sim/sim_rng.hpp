#pragma once

#include "sim/sim_float.hpp"

#include <cstdint>

namespace sim {

class SimRng {
public:
    explicit SimRng(uint64_t seed = 1) noexcept;

    void reseed(uint64_t seed) noexcept;

    [[nodiscard]] uint64_t next_u64() noexcept;

    [[nodiscard]] uint32_t next_u32() noexcept;

    [[nodiscard]] SimFloat next_float_unit() noexcept;

    [[nodiscard]] uint64_t state() const noexcept { return state_; }

private:
    uint64_t state_;
};

}  // namespace sim