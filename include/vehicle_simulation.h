#ifndef VEHICLE_SIMULATION_H
#define VEHICLE_SIMULATION_H

#include "simulation_io.h"
#include "vehicle_types.h"

typedef struct {
    unsigned int lastSpeedRxTime;
    unsigned int lastGearRxTime;
    unsigned int lastEngineRxTime;

    int speed;
    GearState gear;
    EngineState engineState;

    RxState speedRxState;
    RxState gearRxState;
    RxState engineRxState;
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

void UpdateVehicleReception(
    const VehicleInput *input,
    VehicleReception *reception
);

void MonitorVehicleState(
    const VehicleInput *input,
    const VehicleReception *reception,
    VehicleMonitor *monitor
);

void UpdateVehicleTimers(
    const VehicleInput *input,
    VehicleControlState *control
);

void UpdateVehicleControl(
    const VehicleInput *input,
    const VehicleReception *reception,
    const VehicleMonitor *monitor,
    VehicleControlState *control
);

#endif