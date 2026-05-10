add_library(sim_api SHARED
    src/sim_api.cpp
    src/abi_layout_check.cpp
)

target_include_directories(sim_api
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(sim_api
    PRIVATE
        sim
)

target_compile_definitions(sim_api PRIVATE SIM_API_BUILDING_DLL)

if(SIM_DEBUG_LOGGING)
    target_compile_definitions(sim_api PRIVATE SIM_DEBUG=1)
endif()

set_target_properties(sim_api PROPERTIES
    OUTPUT_NAME "sim_api"
    PREFIX ""
    WINDOWS_EXPORT_ALL_SYMBOLS OFF
)

add_custom_command(TARGET sim_api POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:sim_api>
        "${CMAKE_SOURCE_DIR}/unity/arpg-sim-client/Assets/Plugins/x86_64/sim_api.dll"
    COMMENT "Copying sim_api.dll to Unity Plugins directory..."
)