#pragma once

#include "sim/modifier.hpp"
#include "sim/sim_float.hpp"
#include "sim/stat_id.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace sim {

struct StatBucket {
    StatId stat = StatId::None;
    std::vector<Modifier> modifiers;

    mutable SimFloat cached_value = 0.0f;
    mutable bool cache_dirty = true;
};

class EntityStats {
public:
    void add_modifier(const Modifier& mod);

    uint32_t remove_modifiers_by_source(SourceId source);

    [[nodiscard]] std::span<const Modifier> primary_modifiers(StatId stat) const noexcept;

    [[nodiscard]] std::span<const Modifier> meta_modifiers() const noexcept {
        return meta_modifiers_;
    }

    [[nodiscard]] bool has_stat(StatId stat) const noexcept;

    [[nodiscard]] uint32_t bucket_count() const noexcept {
        return static_cast<uint32_t>(primary_buckets_.size());
    }

    void clear() noexcept;

private:
    [[nodiscard]] StatBucket* find_bucket(StatId stat) noexcept;
    [[nodiscard]] const StatBucket* find_bucket(StatId stat) const noexcept;
    StatBucket& find_or_create_bucket(StatId stat);

    std::vector<StatBucket> primary_buckets_;
    std::vector<Modifier> meta_modifiers_;
};

}  // namespace sim