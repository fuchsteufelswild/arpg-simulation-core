#pragma once

#include <cstdint>

namespace sim {

using TagMask = uint64_t;

namespace tags {

inline constexpr TagMask None = 0;

inline constexpr TagMask Physical = 1ULL << 0;
inline constexpr TagMask Fire = 1ULL << 1;
inline constexpr TagMask Cold = 1ULL << 2;
inline constexpr TagMask Lightning = 1ULL << 3;
inline constexpr TagMask Chaos = 1ULL << 4;

inline constexpr TagMask Spell = 1ULL << 8;
inline constexpr TagMask Attack = 1ULL << 9;
inline constexpr TagMask Projectile = 1ULL << 10;
inline constexpr TagMask Melee = 1ULL << 11;
inline constexpr TagMask AoE = 1ULL << 12;
inline constexpr TagMask Channeled = 1ULL << 13;

inline constexpr TagMask Hit = 1ULL << 16;
inline constexpr TagMask DoT = 1ULL << 17;
inline constexpr TagMask Crit = 1ULL << 18;

inline constexpr TagMask Chilled = 1ULL << 24;
inline constexpr TagMask Ignited = 1ULL << 25;
inline constexpr TagMask Poisoned = 1ULL << 26;
inline constexpr TagMask Stunned = 1ULL << 27;

}  // namespace tags

[[nodiscard]] constexpr bool tags_match(TagMask required, TagMask context) noexcept {
    return (required & context) == required;
}

}  // namespace sim