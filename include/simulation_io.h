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

int ParseGear(const char *str, Gear *value);
int ParseEngineState(const char *str, EngineState *value);
int ParseDoorState(const char *str, DoorState *value);
int ParseLightSwitch(const char *str, LightSwitch *value);
int ParseWiperSwitch(const char *str, WiperSwitch *value);

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