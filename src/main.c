#include <stdio.h>
#include "simulation_io.h"
#include "vehicle_simulation.h"

int main(void) {
    FILE *inputFile = fopen("simulation/input.csv", "r");

    if (inputFile == NULL) {
        perror("Failed to open input.csv");
        return 1;
    }

    char line[256];

    /* Skip CSV header */
    if (fgets(line, sizeof(line), inputFile) == NULL) {
        fclose(inputFile);
        return 1;
    }

    FILE *outputFile = fopen("simulation/output.csv", "w");

    if (outputFile == NULL) {
        perror("Failed to open output.csv");
        fclose(inputFile);
        return 1;
    }

    fprintf(
        outputFile,
        "timeMs,headLightState,lightSensorState,wiperState,"
        "rainSensorState,doorLockState,clusterWarning\n"
    );

    /* Initialize vehicle states */
    VehicleInput input = {0};

    VehicleReception reception = {
        .speed = 0,
        .gear = GEAR_P,
        .engineState = ENGINE_OFF,
        .lastSpeedRxTime = 0,
        .lastGearRxTime = 0,
        .lastEngineRxTime = 0,
        .speedRxState = RX_NORMAL,
        .gearRxState = RX_NORMAL,
        .engineRxState = RX_NORMAL
    };

    VehicleMonitor monitor = {0};

    VehicleControlState control = {
        .lowIlluminanceTime = 0,
        .highIlluminanceTime = 0,
        .wiperActiveTime = 0,
        .headLightState = HEADLIGHT_OFF,
        .wiperState = WIPER_STOP,
        .doorLockState = DOOR_UNLOCKED
    };

    int isFirstCycle = 1;
    ReadResult readResult;

    while ((readResult = ReadVehicleInput(inputFile, &input)) == READ_SUCCESS) {

        /* 1. Update vehicle reception */
        UpdateVehicleReception(
            &input,
            &reception
        );

        /* 2. Monitor vehicle and sensor states */
        MonitorVehicleState(
            &input,
            &reception,
            &monitor
        );

        /* 3. Update timers */
        if (isFirstCycle) {
            isFirstCycle = 0;
        } else {
            UpdateVehicleTimers(
                &input,
                &control
            );
        }

        /* 4. Control vehicle functions */
        UpdateVehicleControl(
            &input,
            &reception,
            &monitor,
            &control
        );

        /* 5. Write vehicle outputs */
        WriteVehicleOutput(
            outputFile,
            input.timeMs,
            control.headLightState,
            monitor.lightSensorState,
            control.wiperState,
            monitor.rainSensorState,
            control.doorLockState,
            monitor.clusterWarning
        );
    }

    if (readResult == READ_ERROR) {
        fprintf(stderr, "Failed to read input.csv\n");
    }

    fclose(inputFile);
    fclose(outputFile);

    return readResult == READ_ERROR ? 1 : 0;
}
