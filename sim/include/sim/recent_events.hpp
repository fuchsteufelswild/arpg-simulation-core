#pragma once

#include <array>
#include <cstdint>

namespace sim {

enum class EventType : uint8_t {
    None = 0,
    CritHit,
    TookDamage,
    Killed,
};

struct RecentEvent {
    EventType type = EventType::None;
    uint64_t tick = 0;
};

class RecentEvents {
public:
    static constexpr std::size_t Capacity = 16;

    void push(EventType type, uint64_t tick) noexcept;

    [[nodiscard]] bool
    any_within(EventType type, uint64_t current_tick, uint64_t window_ticks) const noexcept;

    void clear() noexcept;

private:
    std::array<RecentEvent, Capacity> ring_{};
    std::size_t head_ = 0;
};

}  // namespace sim