#include "vehicle_driving.h"

EngineState ControlEngine(
    int speed,
    EngineState currentEngine,
    EngineCommand engineCommand
) {
    if (speed != 0) return currentEngine;

    if (engineCommand == ENGINE_CMD_ON) return ENGINE_ON;
    return ENGINE_OFF;
}

GearState ControlGear(
    int speed,
    GearState currentGear,
    GearCommand gearCommand
) {
    if (speed != 0) return currentGear;

    switch (gearCommand) {
        case GEAR_CMD_P: return GEAR_P;
        case GEAR_CMD_R: return GEAR_R;
        case GEAR_CMD_N: return GEAR_N;
        case GEAR_CMD_D: return GEAR_D;
        default: return currentGear;
    }
}
