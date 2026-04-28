#pragma once

#include <stdint.h>

#ifdef SIM_API_BUILDING_DLL
    #define SIM_API __declspec(dllexport)
#else
    #define SIM_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

SIM_API const char* sim_version(void);

#ifdef __cplusplus
}
#endif