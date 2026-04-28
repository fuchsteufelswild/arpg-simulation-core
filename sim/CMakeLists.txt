add_library(sim STATIC
    src/version.cpp
    src/world.cpp
)

target_include_directories(sim
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(sim PUBLIC cxx_std_23)

if(SIM_DEBUG_LOGGING)
    target_compile_definitions(sim PUBLIC SIM_DEBUG=1)
endif()