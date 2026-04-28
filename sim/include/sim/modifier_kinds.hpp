#pragma once

#include <cstdint>

namespace sim {

enum class ModOp : uint8_t {
    Flat = 0,
    Increased,
    More,
};

enum class ModPhase : uint8_t {
    Primary = 0,
    Meta,
};

enum class ConditionId : uint16_t {
    None = 0,
    WhileAtFullLife,
    WhileAtLowLife,
    AgainstChilled,
    AgainstIgnited,
    OnCrit,
    CritRecently,
};

using SourceId = uint32_t;

[[nodiscard]] constexpr SourceId make_source_id(uint16_t category, uint16_t instance) noexcept {
    return (static_cast<uint32_t>(category) << 16) | instance;
}

namespace source_categories {
inline constexpr uint16_t None = 0;
inline constexpr uint16_t Gear = 1;
inline constexpr uint16_t Status = 2;
inline constexpr uint16_t Aura = 3;
inline constexpr uint16_t PassiveTree = 4;
inline constexpr uint16_t Ability = 5;
}  // namespace source_categories

}  // namespace sim