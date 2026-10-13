#ifndef SIMULATION_IO_H
#define SIMULATION_IO_H

#include <stdio.h>
#include "vehicle_types.h"

typedef struct {
    unsigned int timeMs;
    
    int speed;
    GearState gear;
    EngineState engineState;
    int illuminance;
    int rainfall;

    DoorState doorFL;
    DoorState doorFR;
    DoorState doorRL;
    DoorState doorRR;

    LightSwitch lightSwitch;
    WiperSwitch wiperSwitch;
} VehicleStatus;

typedef struct {
    unsigned int timeMs;

    EngineCommand engineCommand;
    GearCommand gearCommand;
    PedalCommand brakeCommand;
    PedalCommand acceleratorCommand;
} VehicleCommand;

typedef enum {
    READ_ERROR = -1,
    READ_EOF = 0,
    READ_SUCCESS = 1
} ReadResult;

typedef struct {
    FILE *commandFile;
    FILE *statusFile;
    FILE *tempFile;
} DrivingFiles;

typedef struct {
    FILE *statusFile;
    FILE *resultFile;
} ControlFiles;

/* CSV input */
ReadResult ReadVehicleCommand(
    FILE *commandFile,
    VehicleCommand *command
);

ReadResult ReadVehicleStatus(
    FILE *statusFile,
    VehicleStatus *status
);

/* CSV output */
void WriteVehicleStatus(
    FILE *statusFile,
    const VehicleStatus *status
);

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

int OpenDrivingFiles(DrivingFiles *files);
int CloseDrivingFiles(DrivingFiles *files, int success);

int OpenControlFiles(ControlFiles *files);
int CloseControlFiles(ControlFiles *files);

#endif