add_library(sim_api SHARED
    src/sim_api.cpp
)

target_include_directories(sim_api
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(sim_api
    PRIVATE
        sim
)

target_compile_definitions(sim_api PRIVATE SIM_API_BUILDING_DLL)

set_target_properties(sim_api PROPERTIES
    OUTPUT_NAME "sim_api"
    PREFIX ""
    WINDOWS_EXPORT_ALL_SYMBOLS OFF
)