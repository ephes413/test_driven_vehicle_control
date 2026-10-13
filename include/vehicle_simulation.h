#ifndef VEHICLE_SIMULATION_H
#define VEHICLE_SIMULATION_H

#include "simulation_io.h"
#include "vehicle_types.h"

typedef struct {
    unsigned int lastSpeedRxTime;
    unsigned int lastGearRxTime;
    unsigned int lastEngineRxTime;
    unsigned int lastLightRxTime;
    unsigned int lastRainRxTime;

    int speed;
    GearState gear;
    EngineState engineState;
    int illuminance;
    int rainfall;

    RxState speedRxState;
    RxState gearRxState;
    RxState engineRxState;
    RxState lightRxState;
    RxState rainRxState;
} VehicleReception;

typedef struct {
    RxState vehicleRxState;
    SpeedState speedState;

    SensorState lightSensorState;
    SensorState rainSensorState;

    DoorOpenWarningState doorOpenWarningState;
    ClusterWarning clusterWarning;
} VehicleMonitor;

typedef struct {
    unsigned int lowIlluminanceTime;
    unsigned int highIlluminanceTime;
    unsigned int wiperActiveTime;

    HeadLightState headLightState;
    WiperState wiperState;
    DoorLockState doorLockState;
} VehicleControlState;

int RunDrivingSimulation(void);
int RunControlSimulation(void);

#endif