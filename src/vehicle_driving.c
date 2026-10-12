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