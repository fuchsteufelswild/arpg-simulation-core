#include "sim/sim_rng.hpp"

namespace sim {

namespace {

constexpr uint64_t SplitMix64Increment = 0x9E3779B97F4A7C15ull;

[[nodiscard]] uint64_t splitmix64(uint64_t& state) noexcept {
    state += SplitMix64Increment;
    uint64_t z = state;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

}  // namespace

SimRng::SimRng(uint64_t seed) noexcept : state_(seed == 0 ? 1 : seed) {
}

void SimRng::reseed(uint64_t seed) noexcept {
    state_ = (seed == 0) ? 1 : seed;
}

uint64_t SimRng::next_u64() noexcept {
    return splitmix64(state_);
}

uint32_t SimRng::next_u32() noexcept {
    return static_cast<uint32_t>(next_u64() >> 32);
}

SimFloat SimRng::next_float_unit() noexcept {
    constexpr uint32_t mantissa = (1u << 24) - 1;
    const uint32_t bits = next_u32() >> 8;
    return static_cast<SimFloat>(bits) / static_cast<SimFloat>(mantissa);
}

}  // namespace sim