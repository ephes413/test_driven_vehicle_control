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

typedef enum {
    READ_ERROR = -1,
    READ_EOF = 0,
    READ_SUCCESS = 1
} ReadResult;

ReadResult ReadVehicleInput(FILE *inputFile, VehicleInput *input);

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