#ifndef VEHICLE_TIMER_H
#define VEHICLE_TIMER_H

#include "vehicle_types.h"

unsigned int UpdateLowIlluminanceTime(
    int illuminance,
    unsigned int lowIlluminanceTime
);

unsigned int UpdateHighIlluminanceTime(
    int illuminance,
    unsigned int highIlluminanceTime
);

unsigned int UpdateWiperActiveTime(
    WiperState wiperState,
    unsigned int wiperActiveTime
);

#endif