#include "sim/world_hash.hpp"

#include "sim/sim.hpp"
#include "sim/world.hpp"

#include <cstring>

namespace sim {

namespace {

[[nodiscard]] uint64_t fnv_init() noexcept {
    return 0xCBF29CE484222325ull;
}

[[nodiscard]] uint64_t fnv_step(uint64_t hash, uint64_t value) noexcept {
    constexpr uint64_t prime = 0x100000001B3ull;
    hash ^= value;
    hash *= prime;
    return hash;
}

[[nodiscard]] uint64_t hash_float(float f) noexcept {
    uint32_t bits = 0;
    std::memcpy(&bits, &f, sizeof(bits));
    return static_cast<uint64_t>(bits);
}

}  // namespace

uint64_t hash_world_state(const Sim& sim) noexcept {
    uint64_t hash = fnv_init();
    hash = fnv_step(hash, sim.current_tick());
    hash = fnv_step(hash, sim.rng().state());

    const World& world = sim.world();
    const uint32_t capacity = world.capacity();
    hash = fnv_step(hash, capacity);
    hash = fnv_step(hash, world.alive_count());

    for (uint32_t i = 0; i < capacity; ++i) {
        if (!world.is_slot_alive(i)) {
            continue;
        }
        const EntityHandle handle = world.handle_at(i);
        hash = fnv_step(hash, i);
        hash = fnv_step(hash, handle.generation);

        const Transform& t = world.transform(handle);
        hash = fnv_step(hash, hash_float(t.x));
        hash = fnv_step(hash, hash_float(t.y));
        hash = fnv_step(hash, hash_float(t.facing_radians));

        const Health& h = world.health(handle);
        hash = fnv_step(hash, hash_float(h.current));
        hash = fnv_step(hash, hash_float(h.max));

        hash = fnv_step(hash, world.status_list(handle).combined_tags());
        hash = fnv_step(hash, static_cast<uint64_t>(world.kind(handle)));
        hash = fnv_step(hash, world.cooldowns(handle).size());
    }

    hash = fnv_step(hash, sim.abilities().total_active());

    return hash;
}

}  // namespace sim