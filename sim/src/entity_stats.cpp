#include "sim/entity_stats.hpp"

#include <algorithm>

namespace sim {

void EntityStats::add_modifier(const Modifier& mod) {
    if (mod.phase == ModPhase::Meta) {
        meta_modifiers_.push_back(mod);
        return;
    }

    StatBucket& bucket = find_or_create_bucket(mod.stat);
    bucket.modifiers.push_back(mod);
    bucket.cache_dirty = true;
}

uint32_t EntityStats::remove_modifiers_by_source(SourceId source) {
    uint32_t removed{};

    for (auto& bucket : primary_buckets_) {
        const auto before = bucket.modifiers.size();
        std::erase_if(bucket.modifiers, [source](const Modifier& m) { return m.source == source; });
        const auto after = bucket.modifiers.size();
        if (after != before) {
            bucket.cache_dirty = true;
            removed += static_cast<uint32_t>(before - after);
        }
    }

    const auto meta_before = meta_modifiers_.size();
    std::erase_if(meta_modifiers_, [source](const Modifier& m) { return m.source == source; });
    removed += static_cast<uint32_t>(meta_before - meta_modifiers_.size());

    return removed;
}

std::span<const Modifier> EntityStats::primary_modifiers(StatId stat) const noexcept {
    const StatBucket* bucket = find_bucket(stat);
    if (bucket == nullptr) {
        return {};
    }
    return bucket->modifiers;
}

bool EntityStats::has_stat(StatId stat) const noexcept {
    return find_bucket(stat) != nullptr;
}

void EntityStats::clear() noexcept {
    primary_buckets_.clear();
    meta_modifiers_.clear();
}

StatBucket* EntityStats::find_bucket(StatId stat) noexcept {
    auto it = std::ranges::find_if(primary_buckets_,
                                   [stat](const StatBucket& b) { return b.stat == stat; });
    return it == primary_buckets_.end() ? nullptr : &*it;
}

const StatBucket* EntityStats::find_bucket(StatId stat) const noexcept {
    auto it = std::ranges::find_if(primary_buckets_,
                                   [stat](const StatBucket& b) { return b.stat == stat; });
    return it == primary_buckets_.end() ? nullptr : &*it;
}

StatBucket& EntityStats::find_or_create_bucket(StatId stat) {
    auto it = std::ranges::lower_bound(
        primary_buckets_, stat, {}, [](const StatBucket& b) { return b.stat; });

    if (it != primary_buckets_.end() && it->stat == stat) {
        return *it;
    }

    auto inserted = primary_buckets_.insert(it, StatBucket{.stat = stat, .modifiers = {}});
    return *inserted;
}

}  // namespace sim