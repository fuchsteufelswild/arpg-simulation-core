#pragma once

#include <cstdint>
#include <functional>
#include <type_traits>

namespace sim {

struct EntityHandle {
    uint32_t index = 0;
    uint16_t generation = 0;
    uint16_t _pad = 0;  // NOLINT(readability-identifier-naming)

    [[nodiscard]] constexpr bool is_null() const noexcept { return generation == 0; }

    [[nodiscard]] constexpr bool operator==(const EntityHandle&) const noexcept = default;
};

static_assert(std::is_trivially_copyable_v<EntityHandle>);
static_assert(std::is_standard_layout_v<EntityHandle>);
static_assert(sizeof(EntityHandle) == 8);

}  // namespace sim

template <>
struct std::hash<sim::EntityHandle> {
    [[nodiscard]] size_t operator()(const sim::EntityHandle& h) const noexcept {
        return (static_cast<size_t>(h.generation) << 32) | h.index;
    }
};