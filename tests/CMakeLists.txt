find_package(Catch2 3 REQUIRED)

add_executable(sim_tests
    test_smoke.cpp
)

target_link_libraries(sim_tests
    PRIVATE
        sim
        Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(sim_tests)