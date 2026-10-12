#ifndef VEHICLE_DRIVING_H
#define VEHICLE_DRIVING_H

#include "vehicle_types.h"

EngineState ControlEngine(
    int speed,
    EngineState currentEngine,
    EngineCommand engineCommand
);

GearState ControlGear(
    int speed,
    GearState currentGear,
    GearCommand gearCommand
);

#endif