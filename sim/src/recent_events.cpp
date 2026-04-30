#include "sim/recent_events.hpp"

#include <algorithm>

namespace sim {

void RecentEvents::push(EventType type, uint64_t tick) noexcept {
    ring_[head_] = RecentEvent{// NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
                               .type = type,
                               .tick = tick};
    head_ = (head_ + 1) % Capacity;
}

bool RecentEvents::any_within(EventType type,
                              uint64_t current_tick,
                              uint64_t window_ticks) const noexcept {
    const uint64_t threshold_tick =
        (current_tick > window_ticks) ? (current_tick - window_ticks) : 0;

    return std::ranges::any_of(ring_, [&](const RecentEvent& event) {
        return event.type == type && event.tick >= threshold_tick && event.tick <= current_tick;
    });
}

void RecentEvents::clear() noexcept {
    ring_.fill(RecentEvent{});
    head_ = 0;
}

}  // namespace sim