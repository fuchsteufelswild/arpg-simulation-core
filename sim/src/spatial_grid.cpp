#include "sim/spatial_grid.hpp"

#include "sim/world.hpp"

#include <algorithm>
#include <cmath>

namespace sim {

namespace {

[[nodiscard]] SimFloat
distance_squared(SimFloat ax, SimFloat ay, SimFloat bx, SimFloat by) noexcept {
    const SimFloat dx = ax - bx;
    const SimFloat dy = ay - by;
    return (dx * dx) + (dy * dy);
}

}  // namespace

SpatialGrid::SpatialGrid(SimFloat cell_size) noexcept : cell_size_(cell_size) {
}

CellCoord SpatialGrid::cell_for(SimFloat x, SimFloat y) const noexcept {
    return CellCoord{
        .x = static_cast<int32_t>(std::floor(x / cell_size_)),
        .y = static_cast<int32_t>(std::floor(y / cell_size_)),
    };
}

void SpatialGrid::rebuild(const World& world) {
    entries_.clear();
    cells_.clear();

    const uint32_t capacity = world.capacity();
    entries_.reserve(world.alive_count());

    for (uint32_t i = 0; i < capacity; ++i) {
        if (!world.is_slot_alive(i)) {
            continue;
        }

        const EntityHandle handle = world.handle_at(i);
        if (!world.is_alive(handle)) {
            continue;
        }

        const Transform& transform = world.transform(handle);
        const auto entry_index = static_cast<uint32_t>(entries_.size());
        entries_.push_back(SpatialEntry{
            .handle = handle,
            .x = transform.x,
            .y = transform.y,
        });

        const CellCoord cell = cell_for(transform.x, transform.y);
        cells_[cell].push_back(entry_index);
    }
}

std::vector<EntityHandle>
SpatialGrid::query_within(SimFloat origin_x, SimFloat origin_y, SimFloat radius) const {
    std::vector<EntityHandle> results;
    if (radius <= 0.0f) {
        return results;
    }

    const SimFloat radius_squared = radius * radius;
    const CellCoord min_cell = cell_for(origin_x - radius, origin_y - radius);
    const CellCoord max_cell = cell_for(origin_x + radius, origin_y + radius);

    struct Candidate {
        EntityHandle handle;
        SimFloat distance_sq;
    };
    std::vector<Candidate> candidates;

    for (int32_t cy = min_cell.y; cy <= max_cell.y; ++cy) {
        for (int32_t cx = min_cell.x; cx <= max_cell.x; ++cx) {
            const auto it = cells_.find(CellCoord{.x = cx, .y = cy});
            if (it == cells_.end()) {
                continue;
            }
            for (uint32_t entry_index : it->second) {
                const SpatialEntry& entry = entries_[entry_index];
                const SimFloat dist_sq = distance_squared(origin_x, origin_y, entry.x, entry.y);
                if (dist_sq <= radius_squared) {
                    candidates.push_back(Candidate{
                        .handle = entry.handle,
                        .distance_sq = dist_sq,
                    });
                }
            }
        }
    }

    std::ranges::sort(candidates, [](const Candidate& a, const Candidate& b) {
        if (a.distance_sq != b.distance_sq) {
            return a.distance_sq < b.distance_sq;
        }
        if (a.handle.index != b.handle.index) {
            return a.handle.index < b.handle.index;
        }
        return a.handle.generation < b.handle.generation;
    });

    results.reserve(candidates.size());
    for (const Candidate& c : candidates) {
        results.push_back(c.handle);
    }
    return results;
}

void SpatialGrid::clear() noexcept {
    entries_.clear();
    cells_.clear();
}

}  // namespace sim