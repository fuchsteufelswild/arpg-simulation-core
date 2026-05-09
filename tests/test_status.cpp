#include "sim/effect_context.hpp"
#include "sim/sim_commands.hpp"
#include "sim/status_list.hpp"
#include "sim/status_system.hpp"
#include "sim/world.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace sim;
using Catch::Matchers::WithinAbs;

namespace {

constexpr SimFloat Tolerance = 0.0001f;

StatusInstance make_chill(SimFloat slow, uint64_t apply_tick, uint64_t duration) {
    return StatusInstance{
        .type = StatusType::Chill,
        .source = make_source_id(source_categories::Status, 1),
        .apply_tick = apply_tick,
        .expire_tick = apply_tick + duration,
        .last_tick_processed = apply_tick,
        .payload = ChillData{.slow_amount = slow},
    };
}

StatusInstance make_ignite(SimFloat dps, uint64_t apply_tick, uint64_t duration) {
    return StatusInstance{
        .type = StatusType::Ignite,
        .source = make_source_id(source_categories::Status, 2),
        .apply_tick = apply_tick,
        .expire_tick = apply_tick + duration,
        .last_tick_processed = apply_tick,
        .payload = IgniteData{.damage_per_tick = dps},
    };
}

StatusInstance make_poison(SimFloat dps, uint64_t apply_tick, uint64_t duration) {
    return StatusInstance{
        .type = StatusType::Poison,
        .source = make_source_id(source_categories::Status, 3),
        .apply_tick = apply_tick,
        .expire_tick = apply_tick + duration,
        .last_tick_processed = apply_tick,
        .payload = PoisonData{.damage_per_tick = dps},
    };
}

StatusInstance make_stun(uint64_t apply_tick, uint64_t duration) {
    return StatusInstance{
        .type = StatusType::Stun,
        .source = make_source_id(source_categories::Status, 4),
        .apply_tick = apply_tick,
        .expire_tick = apply_tick + duration,
        .last_tick_processed = apply_tick,
        .payload = StunData{},
    };
}

}  // namespace

TEST_CASE("empty status list has no tags", "[status]") {
    StatusList list;
    REQUIRE(list.combined_tags() == tags::None);
    REQUIRE(list.size() == 0);
}

TEST_CASE("adding chill contributes Chilled tag", "[status]") {
    StatusList list;
    list.add(make_chill(0.30f, 0, 100));

    REQUIRE(list.size() == 1);
    REQUIRE((list.combined_tags() & tags::Chilled) != 0);
}

TEST_CASE("adding multiple status types contributes multiple tags", "[status]") {
    StatusList list;
    list.add(make_chill(0.30f, 0, 100));
    list.add(make_ignite(10.0f, 0, 100));

    REQUIRE((list.combined_tags() & tags::Chilled) != 0);
    REQUIRE((list.combined_tags() & tags::Ignited) != 0);
}

TEST_CASE("chill replace-if-stronger keeps the stronger one", "[status][stacking]") {
    StatusList list;
    list.add(make_chill(0.20f, 0, 100));
    list.add(make_chill(0.40f, 5, 100));

    REQUIRE(list.size() == 1);
    const auto& only = list.all()[0];
    const auto& chill = std::get<ChillData>(only.payload);
    REQUIRE(chill.slow_amount == 0.40f);
}

TEST_CASE("chill replace-if-stronger keeps existing if new is weaker", "[status][stacking]") {
    StatusList list;
    list.add(make_chill(0.40f, 0, 100));
    list.add(make_chill(0.20f, 5, 200));

    REQUIRE(list.size() == 1);
    const auto& only = list.all()[0];
    const auto& chill = std::get<ChillData>(only.payload);
    REQUIRE(chill.slow_amount == 0.40f);
    REQUIRE(only.expire_tick == 205);
}

TEST_CASE("poison stacks as separate instances", "[status][stacking]") {
    StatusList list;
    list.add(make_poison(5.0f, 0, 100));
    list.add(make_poison(5.0f, 5, 100));
    list.add(make_poison(5.0f, 10, 100));

    REQUIRE(list.size() == 3);
}

TEST_CASE("stun refresh extends duration", "[status][stacking]") {
    StatusList list;
    list.add(make_stun(0, 50));
    list.add(make_stun(20, 80));

    REQUIRE(list.size() == 1);
    REQUIRE(list.all()[0].expire_tick == 100);
}

TEST_CASE("stun refresh does not shorten existing duration", "[status][stacking]") {
    StatusList list;
    list.add(make_stun(0, 200));
    list.add(make_stun(0, 50));

    REQUIRE(list.size() == 1);
    REQUIRE(list.all()[0].expire_tick == 200);
}

TEST_CASE("remove_by_source removes matching statuses", "[status]") {
    StatusList list;
    list.add(make_chill(0.30f, 0, 100));
    list.add(make_ignite(10.0f, 0, 100));

    const SourceId chill_source = make_source_id(source_categories::Status, 1);
    const uint32_t removed = list.remove_by_source(chill_source);

    REQUIRE(removed == 1);
    REQUIRE(list.size() == 1);
    REQUIRE((list.combined_tags() & tags::Chilled) == 0);
    REQUIRE((list.combined_tags() & tags::Ignited) != 0);
}

TEST_CASE("remove_expired removes only expired statuses", "[status]") {
    StatusList list;
    list.add(make_chill(0.30f, 0, 100));
    list.add(make_ignite(10.0f, 0, 50));

    list.remove_expired(60);

    REQUIRE(list.size() == 1);
    REQUIRE(list.all()[0].type == StatusType::Chill);
    REQUIRE((list.combined_tags() & tags::Ignited) == 0);
}

TEST_CASE("ignite ticks damage at interval", "[status][system]") {
    World world;
    SimCommands commands;

    auto target = world.spawn(EntityKind::Enemy);
    auto applier = world.spawn(EntityKind::Player);
    world.health(target) = Health{.current = 100.0f, .max = 100.0f};

    StatusInstance ignite = make_ignite(5.0f, 0, 200);
    ignite.applier = applier;
    world.status_list(target).add(ignite);

    update_statuses(world, commands, 30);
    REQUIRE(commands.deal_damage.size() == 1);
    REQUIRE_THAT(commands.deal_damage[0].amount, WithinAbs(5.0f, Tolerance));

    commands.clear();
    update_statuses(world, commands, 60);
    REQUIRE(commands.deal_damage.size() == 1);
}

TEST_CASE("ignite does not tick before interval", "[status][system]") {
    World world;
    SimCommands commands;

    auto target = world.spawn(EntityKind::Enemy);
    auto applier = world.spawn(EntityKind::Player);

    StatusInstance ignite = make_ignite(5.0f, 0, 200);
    ignite.applier = applier;
    world.status_list(target).add(ignite);

    update_statuses(world, commands, 15);
    REQUIRE(commands.deal_damage.empty());
}

TEST_CASE("expired status is removed during update", "[status][system]") {
    World world;
    SimCommands commands;

    auto target = world.spawn(EntityKind::Enemy);
    auto applier = world.spawn(EntityKind::Player);

    StatusInstance ignite = make_ignite(5.0f, 0, 50);
    ignite.applier = applier;
    world.status_list(target).add(ignite);

    update_statuses(world, commands, 60);
    REQUIRE(world.status_list(target).size() == 0);
}

TEST_CASE("chill tag affects against-chilled modifier", "[status][evaluate]") {
    World world;
    auto target = world.spawn(EntityKind::Enemy);
    world.status_list(target).add(make_chill(0.30f, 0, 100));

    REQUIRE((world.status_list(target).combined_tags() & tags::Chilled) != 0);
}