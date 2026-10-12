#include "vehicle_driving.h"

EngineState ControlEngine(
    int speed,
    EngineState currentEngine,
    EngineCommand engineCommand
) {
    if (speed != 0) return currentEngine;
    if (engineCommand == ENGINE_CMD_OFF) return ENGINE_OFF;
    if (engineCommand == ENGINE_CMD_ON) return ENGINE_ON;
    return currentEngine;
}

GearState ControlGear(
    int speed,
    GearState currentGear,
    GearCommand gearCommand
) {
    if (speed != 0) return currentGear;
    if (gearCommand == GEAR_CMD_P) return GEAR_P;
    if (gearCommand == GEAR_CMD_R) return GEAR_R;
    if (gearCommand == GEAR_CMD_N) return GEAR_N;
    if (gearCommand == GEAR_CMD_D) return GEAR_D;
    return currentGear;
}

int UpdateVehicleSpeed(
    EngineState engineState,
    GearState gearState,
    PedalCommand brakeCommand,
    PedalCommand acceleratorCommand,
    int currentSpeed
) {
    if (engineState == ENGINE_OFF) return 0;
    if (gearState == GEAR_P) return 0;
    if (gearState == GEAR_N) return 0;
    if (gearState == GEAR_D) {
        if (brakeCommand == PEDAL_PRESSED) currentSpeed -= 5;
        else {
            if (acceleratorCommand == PEDAL_PRESSED) currentSpeed += 5;
            if (acceleratorCommand == PEDAL_RELEASED) currentSpeed -= 1;
        }
        if (currentSpeed < 0) currentSpeed = 0;
        if (currentSpeed > 250) currentSpeed = 250;
    }
    if (gearState == GEAR_R) {
        if (brakeCommand == PEDAL_PRESSED) currentSpeed += 5;
        else {
            if (acceleratorCommand == PEDAL_PRESSED) currentSpeed -= 5;
            if (acceleratorCommand == PEDAL_RELEASED) currentSpeed += 1;
        }
        if (currentSpeed > 0) currentSpeed = 0;
        if (currentSpeed < -20) currentSpeed = -20;
    }
    return currentSpeed;
}
