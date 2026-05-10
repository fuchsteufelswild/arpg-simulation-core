find_package(Catch2 3 REQUIRED)

add_executable(sim_tests
    test_smoke.cpp
    test_world.cpp
    test_modifier.cpp
    test_entity_stats.cpp
    test_evaluate.cpp
    test_recent_events.cpp
    test_conditions.cpp
    test_meta_modifiers.cpp
    test_ability_data.cpp
    test_ability_execution.cpp
    test_status.cpp
    test_spatial_grid.cpp
    test_chain_effect.cpp
    test_trigger_effect.cpp
    test_projectile_travel.cpp
    test_sim.cpp
    test_data_loading.cpp
    test_cooldown.cpp
)

target_link_libraries(sim_tests
    PRIVATE
        sim
        Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(sim_tests)