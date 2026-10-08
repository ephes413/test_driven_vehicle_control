#ifndef VEHICLE_CONTROL_H
#define VEHICLE_CONTROL_H

#include "vehicle_types.h"

HeadLightState ControlHeadLight(
    RxState vehicleRxState,
    EngineState engineState,
    SensorState lightSensorState,
    LightSwitch lightSwitch,
    unsigned int wiperActiveTime,
    unsigned int lowIlluminanceTime,
    unsigned int highIlluminanceTime,
    HeadLightState currentHeadLight
);

WiperState ControlWiper(
    RxState vehicleRxState,
    EngineState engineState,
    WiperSwitch wiperSwitch,
    SensorState rainSensorState,
    int rainAmount,
    SpeedState speedState,
    int speed
);

DoorLockState ControlDoorLock(
    RxState vehicleRxState,
    EngineState engineState,
    Gear gear,
    int speed,
    DoorLockState currentDoorLock
);

#endif