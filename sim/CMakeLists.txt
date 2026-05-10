add_library(sim STATIC
    src/version.cpp
    src/world.cpp
    src/entity_stats.cpp
    src/evaluate.cpp
    src/recent_events.cpp
    src/conditions.cpp
    src/ability_registry.cpp
    src/ability_system.cpp
    src/sim_commands.cpp
    src/effect_execution.cpp
    src/damage.cpp
    src/status_list.cpp
    src/status_system.cpp
    src/spatial_grid.cpp
    src/command_application.cpp
    src/sim_rng.cpp
    src/sim.cpp
    src/data/enum_lookup.cpp
    src/data/ability_loader.cpp
    src/cooldown.cpp
    src/input_log.cpp
    src/world_hash.cpp
    src/replay.cpp
)

target_include_directories(sim
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(tomlplusplus CONFIG REQUIRED)
target_link_libraries(sim PUBLIC tomlplusplus::tomlplusplus)

target_compile_features(sim PUBLIC cxx_std_23)

if(SIM_DEBUG_LOGGING)
    target_compile_definitions(sim PUBLIC SIM_DEBUG=1)
endif()