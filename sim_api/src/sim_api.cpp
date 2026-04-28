#include "sim_api/sim_api.h"

#include "sim/version.hpp"

extern "C" SIM_API const char* sim_version(void) {
    return sim::version().data();
}