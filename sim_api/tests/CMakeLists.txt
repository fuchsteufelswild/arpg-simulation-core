find_package(Catch2 3 REQUIRED)

add_executable(sim_api_tests
    test_sim_api.cpp
)

target_link_libraries(sim_api_tests
    PRIVATE
        sim_api
        Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(sim_api_tests)

if(WIN32)
    add_custom_command(TARGET sim_api_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:sim_api>
            $<TARGET_FILE_DIR:sim_api_tests>
        COMMENT "Copying sim_api.dll next to sim_api_tests.exe"
    )
endif()