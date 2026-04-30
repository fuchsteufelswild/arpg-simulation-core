find_package(Catch2 3 REQUIRED)

add_executable(sim_tests
    test_smoke.cpp
    test_world.cpp
    test_modifier.cpp
    test_entity_stats.cpp
    test_evaluate.cpp
    test_recent_events.cpp
    test_conditions.cpp
)

target_link_libraries(sim_tests
    PRIVATE
        sim
        Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(sim_tests)