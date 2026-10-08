#ifndef SIMULATION_IO_H
#define SIMULATION_IO_H

#include <stdio.h>
#include "vehicle_types.h"

typedef struct {
    unsigned int timeMs;
    int speed;
    Gear gear;
    EngineState engineState;

    int illuminance;
    int rainfall;

    DoorState doorFL;
    DoorState doorFR;
    DoorState doorRL;
    DoorState doorRR;

    LightSwitch lightSwitch;
    WiperSwitch wiperSwitch;
} VehicleInput;

int ReadVehicleInput(FILE *inputFile, VehicleInput *input);

void WriteVehicleOutput(
    FILE *outputFile,
    unsigned int timeMs,
    HeadLightState headLightState,
    SensorState lightSensorState,
    WiperState wiperState,
    SensorState rainSensorState,
    DoorLockState doorLockState,
    ClusterWarning clusterWarning
);

#endif