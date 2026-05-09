#pragma once

#include "sim/entity_handle.hpp"
#include "sim/sim_float.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace sim {

class World;

struct CellCoord {
    int32_t x = 0;
    int32_t y = 0;

    [[nodiscard]] constexpr bool operator==(const CellCoord&) const noexcept = default;
};

struct CellCoordHash {
    [[nodiscard]] std::size_t operator()(const CellCoord& c) const noexcept {
        const auto ux = static_cast<uint32_t>(c.x);
        const auto uy = static_cast<uint32_t>(c.y);
        return (static_cast<std::size_t>(ux) << 32) | uy;
    }
};

struct SpatialEntry {
    EntityHandle handle;
    SimFloat x = 0.0f;
    SimFloat y = 0.0f;
};

class SpatialGrid {
public:
    static constexpr SimFloat DefaultCellSize = 8.0f;

    explicit SpatialGrid(SimFloat cell_size = DefaultCellSize) noexcept;

    void rebuild(const World& world);

    [[nodiscard]] std::vector<EntityHandle> query_within(SimFloat origin_x,
                                                         SimFloat origin_y,
                                                         SimFloat radius) const;

    [[nodiscard]] std::size_t entry_count() const noexcept { return entries_.size(); }

    void clear() noexcept;

private:
    [[nodiscard]] CellCoord cell_for(SimFloat x, SimFloat y) const noexcept;

    SimFloat cell_size_;
    std::vector<SpatialEntry> entries_;
    std::unordered_map<CellCoord, std::vector<uint32_t>, CellCoordHash> cells_;
};

}