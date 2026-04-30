#include "sim/conditions.hpp"
#include "sim/world.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace sim;

TEST_CASE("None condition is always true", "[conditions]") {
    World world;
    EvalContext ctx;
    REQUIRE(evaluate_condition(ConditionId::None, 0.0f, world, ctx));
}

TEST_CASE("WhileAtFullLife requires attacker at full life", "[conditions]") {
    World world;
    auto attacker = world.spawn(EntityKind::Player);
    world.health(attacker) = Health{.current = 100.0f, .max = 100.0f};

    EvalContext ctx{.attacker = attacker};

    SECTION("at full life") {
        REQUIRE(evaluate_condition(ConditionId::WhileAtFullLife, 0.0f, world, ctx));
    }

    SECTION("at 99% life") {
        world.health(attacker).current = 99.0f;
        REQUIRE_FALSE(evaluate_condition(ConditionId::WhileAtFullLife, 0.0f, world, ctx));
    }
}

TEST_CASE("WhileAtFullLife with null attacker is false", "[conditions]") {
    World world;
    EvalContext ctx;
    REQUIRE_FALSE(evaluate_condition(ConditionId::WhileAtFullLife, 0.0f, world, ctx));
}

TEST_CASE("WhileAtLowLife threshold is parameterized", "[conditions]") {
    World world;
    auto attacker = world.spawn(EntityKind::Player);
    world.health(attacker) = Health{.current = 30.0f, .max = 100.0f};

    EvalContext ctx{.attacker = attacker};

    REQUIRE(evaluate_condition(ConditionId::WhileAtLowLife, 0.50f, world, ctx));
    REQUIRE(evaluate_condition(ConditionId::WhileAtLowLife, 0.30f, world, ctx));
    REQUIRE_FALSE(evaluate_condition(ConditionId::WhileAtLowLife, 0.25f, world, ctx));
}

TEST_CASE("AgainstChilled checks target_tags", "[conditions]") {
    World world;
    EvalContext ctx{.target_tags = tags::Chilled};

    REQUIRE(evaluate_condition(ConditionId::AgainstChilled, 0.0f, world, ctx));
}

TEST_CASE("AgainstChilled is false without the tag", "[conditions]") {
    World world;
    EvalContext ctx{.target_tags = tags::Ignited};

    REQUIRE_FALSE(evaluate_condition(ConditionId::AgainstChilled, 0.0f, world, ctx));
}

TEST_CASE("CritRecently checks attacker's recent events", "[conditions]") {
    World world;
    auto attacker = world.spawn(EntityKind::Player);
    world.recent_events(attacker).push(EventType::CritHit, 100);

    SECTION("within window") {
        EvalContext ctx{.attacker = attacker, .current_tick = 130};
        REQUIRE(evaluate_condition(ConditionId::CritRecently, 4.0f, world, ctx));
    }

    SECTION("outside window") {
        EvalContext ctx{.attacker = attacker, .current_tick = 250};
        REQUIRE_FALSE(evaluate_condition(ConditionId::CritRecently, 4.0f, world, ctx));
    }
}

TEST_CASE("OnCrit returns false (will be implemented when Hit data is available)", "[conditions]") {
    World world;
    EvalContext ctx;
    REQUIRE_FALSE(evaluate_condition(ConditionId::OnCrit, 0.0f, world, ctx));
}