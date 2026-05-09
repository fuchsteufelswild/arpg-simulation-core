#pragma once

#include "sim/status_instance.hpp"
#include "sim/tags.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace sim {

class StatusList {
public:
    void add(StatusInstance status);

    uint32_t remove_by_source(SourceId source);

    void remove_expired(uint64_t current_tick);

    [[nodiscard]] std::span<const StatusInstance> all() const noexcept { return statuses_; }

    [[nodiscard]] std::span<StatusInstance> all_mut() noexcept { return statuses_; }

    [[nodiscard]] TagMask combined_tags() const noexcept { return combined_tags_; }

    [[nodiscard]] uint32_t size() const noexcept { return static_cast<uint32_t>(statuses_.size()); }

    void clear() noexcept;

private:
    void recompute_tags() noexcept;

    [[nodiscard]] StatusInstance* find_existing(StatusType type) noexcept;

    void apply_replace_if_stronger(StatusInstance new_status);
    void apply_refresh(StatusInstance new_status);
    void apply_stack(StatusInstance new_status);

    std::vector<StatusInstance> statuses_;
    TagMask combined_tags_ = tags::None;
};

}  // namespace sim