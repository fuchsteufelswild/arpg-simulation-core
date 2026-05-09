#include "sim/status_list.hpp"

#include "sim/util/overload.hpp"

#include <algorithm>
#include <variant>

namespace sim {

namespace {

[[nodiscard]] TagMask tag_for(StatusType type) noexcept {
    switch (type) {
    case StatusType::Chill:
        return tags::Chilled;
    case StatusType::Ignite:
        return tags::Ignited;
    case StatusType::Poison:
        return tags::Poisoned;
    case StatusType::Stun:
        return tags::Stunned;
    case StatusType::None:
        return tags::None;
    }
    return tags::None;
}

enum class StackingRule : uint8_t {
    ReplaceIfStronger,
    Refresh,
    Stack,
};

[[nodiscard]] StackingRule rule_for(StatusType type) noexcept {
    switch (type) {
    case StatusType::Chill:
    case StatusType::Ignite:
        return StackingRule::ReplaceIfStronger;
    case StatusType::Poison:
        return StackingRule::Stack;
    case StatusType::Stun:
        return StackingRule::Refresh;
    case StatusType::None:
        return StackingRule::Stack;
    }
    return StackingRule::Stack;
}

[[nodiscard]] bool is_stronger(const StatusInstance& candidate,
                               const StatusInstance& existing) noexcept {
    return std::visit(util::Overload{
                          [&](const ChillData& a) {
                              const auto& b = std::get<ChillData>(existing.payload);
                              return a.slow_amount > b.slow_amount;
                          },
                          [&](const IgniteData& a) {
                              const auto& b = std::get<IgniteData>(existing.payload);
                              return a.damage_per_tick > b.damage_per_tick;
                          },
                          [](const PoisonData&) { return false; },
                          [](const StunData&) { return false; },
                      },
                      candidate.payload);
}

}  // namespace

void StatusList::add(StatusInstance status) {
    switch (rule_for(status.type)) {
    case StackingRule::ReplaceIfStronger:
        apply_replace_if_stronger(status);
        break;
    case StackingRule::Refresh:
        apply_refresh(status);
        break;
    case StackingRule::Stack:
        apply_stack(status);
        break;
    }
    recompute_tags();
}

uint32_t StatusList::remove_by_source(SourceId source) {
    const auto before = statuses_.size();
    std::erase_if(statuses_, [source](const StatusInstance& s) { return s.source == source; });
    const auto removed = static_cast<uint32_t>(before - statuses_.size());
    if (removed > 0) {
        recompute_tags();
    }
    return removed;
}

void StatusList::remove_expired(uint64_t current_tick) {
    const auto before = statuses_.size();
    std::erase_if(statuses_, [current_tick](const StatusInstance& s) {
        return current_tick >= s.expire_tick;
    });
    if (statuses_.size() != before) {
        recompute_tags();
    }
}

void StatusList::clear() noexcept {
    statuses_.clear();
    combined_tags_ = tags::None;
}

void StatusList::recompute_tags() noexcept {
    combined_tags_ = tags::None;
    for (const StatusInstance& s : statuses_) {
        combined_tags_ |= tag_for(s.type);
    }
}

StatusInstance* StatusList::find_existing(StatusType type) noexcept {
    auto it =
        std::ranges::find_if(statuses_, [type](const StatusInstance& s) { return s.type == type; });
    return it == statuses_.end() ? nullptr : &*it;
}

void StatusList::apply_replace_if_stronger(StatusInstance new_status) {
    StatusInstance* existing = find_existing(new_status.type);
    if (existing == nullptr) {
        statuses_.push_back(new_status);
        return;
    }
    if (is_stronger(new_status, *existing)) {
        *existing = new_status;
    } else if (new_status.expire_tick > existing->expire_tick) {
        existing->expire_tick = new_status.expire_tick;
    }
}

void StatusList::apply_refresh(StatusInstance new_status) {
    StatusInstance* existing = find_existing(new_status.type);
    if (existing == nullptr) {
        statuses_.push_back(new_status);
        return;
    }
    existing->expire_tick = std::max(existing->expire_tick, new_status.expire_tick);
}

void StatusList::apply_stack(StatusInstance new_status) {
    statuses_.push_back(new_status);
}

}  // namespace sim