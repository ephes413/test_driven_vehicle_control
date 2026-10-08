#ifndef SIMULATION_IO_H
#define SIMULATION_IO_H

#include <stdio.h>
#include "vehicle_types.h"

int ParseGear(const char *str, Gear *value);
int ParseEngineState(const char *str, EngineState *value);
int ParseDoorState(const char *str, DoorState *value);
int ParseLightSwitch(const char *str, LightSwitch *value);
int ParseWiperSwitch(const char *str, WiperSwitch *value);

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