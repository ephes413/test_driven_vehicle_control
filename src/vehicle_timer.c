#include "vehicle_timer.h"

unsigned int UpdateLowIlluminanceTime(
    int illuminance,
    unsigned int lowIlluminanceTime
) {
    if (illuminance < 1000) return lowIlluminanceTime + 100;
    return 0;
}

unsigned int UpdateHighIlluminanceTime(
    int illuminance,
    unsigned int highIlluminanceTime
) {
    if (illuminance > 1500) return highIlluminanceTime + 100;
    return 0;
}

unsigned int UpdateWiperActiveTime(
    WiperState wiperState,
    unsigned int wiperActiveTime
) {
    if (wiperState != WIPER_STOP) return wiperActiveTime + 100;
    return 0;
}