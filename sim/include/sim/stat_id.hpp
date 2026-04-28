#pragma once

#include <cstdint>

namespace sim {

enum class StatId : uint16_t {
    None = 0,

    MaxLife,
    LifeRegen,

    PhysicalDamage,
    FireDamage,
    ColdDamage,
    LightningDamage,
    ChaosDamage,

    AttackSpeed,
    CastSpeed,
    MovementSpeed,

    CritChance,
    CritMultiplier,

    FireResistance,
    ColdResistance,
    LightningResistance,
    ChaosResistance,

    _count,  // NOLINT(readability-identifier-naming)
};

[[nodiscard]] constexpr uint16_t to_index(StatId id) noexcept {
    return static_cast<uint16_t>(id);
}

inline constexpr uint16_t StatCount = static_cast<uint16_t>(StatId::_count);

}  // namespace sim