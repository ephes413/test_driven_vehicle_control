#include <stdio.h>
#include "vehicle_types.h"
#include "vehicle_control.h"
#include "vehicle_monitor.h"
#include "vehicle_timer.h"
#include "simulation_io.h"

#define RX_TIMEOUT_MS 500U
#define SPEED_NOT_RECEIVED (-1)

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

    unsigned int lowIlluminanceTime = 0;
    unsigned int highIlluminanceTime = 0;
    unsigned int wiperActiveTime = 0;

    HeadLightState headLightState = HEADLIGHT_OFF;
    WiperState wiperState = WIPER_STOP;
    DoorLockState doorLockState = DOOR_UNLOCKED;

    unsigned int lastSpeedRxTime = 0;
    unsigned int lastGearRxTime = 0;
    unsigned int lastEngineRxTime = 0;

    int speed = 0;
    Gear gearValue = GEAR_P;
    EngineState engineStateValue = ENGINE_OFF;

    int isFirstCycle = 1;

    while (fgets(line, sizeof(line), inputFile) != NULL) {
        /* 1. Update vehicle inputs */

        unsigned int timeMs;
        int inputSpeed;
        char gear[32];
        char engineState[32];
        int illuminance;
        int rainfall;
        char doorFL[16];
        char doorFR[16];
        char doorRL[16];
        char doorRR[16];
        char lightSwitch[16];
        char wiperSwitch[32];

        int parsed = sscanf(
            line,
            "%u,%d,%31[^,],%31[^,],%d,%d,%15[^,],%15[^,],%15[^,],%15[^,],%15[^,],%31[^\r\n]",
            &timeMs,
            &inputSpeed,
            gear,
            engineState,
            &illuminance,
            &rainfall,
            doorFL,
            doorFR,
            doorRL,
            doorRR,
            lightSwitch,
            wiperSwitch
        );

        if (parsed != 12) {
            fprintf(stderr, "Invalid CSV row: %s", line);
            continue;
        }

        /* 2. Validate inputs */

        Gear inputGear;
        EngineState inputEngine;
        DoorState doorFLValue;
        DoorState doorFRValue;
        DoorState doorRLValue;
        DoorState doorRRValue;
        LightSwitch lightSwitchValue;
        WiperSwitch wiperSwitchValue;

        if (!ParseGear(gear, &inputGear)) {
            fprintf(stderr, "Invalid gear at %u ms: %s\n", timeMs, gear);
            continue;
        }

        if (!ParseEngineState(engineState, &inputEngine)) {
            fprintf(stderr, "Invalid engineState at %u ms: %s\n", timeMs, engineState);
            continue;
        }

        if (!ParseDoorState(doorFL, &doorFLValue)) {
            fprintf(stderr, "Invalid doorFL at %u ms: %s\n", timeMs, doorFL);
            continue;
        }

        if (!ParseDoorState(doorFR, &doorFRValue)) {
            fprintf(stderr, "Invalid doorFR at %u ms: %s\n", timeMs, doorFR);
            continue;
        }

        if (!ParseDoorState(doorRL, &doorRLValue)) {
            fprintf(stderr, "Invalid doorRL at %u ms: %s\n", timeMs, doorRL);
            continue;
        }

        if (!ParseDoorState(doorRR, &doorRRValue)) {
            fprintf(stderr, "Invalid doorRR at %u ms: %s\n", timeMs, doorRR);
            continue;
        }

        if (!ParseLightSwitch(lightSwitch, &lightSwitchValue)) {
            fprintf(stderr, "Invalid lightSwitch at %u ms: %s\n", timeMs, lightSwitch);
            continue;
        }

        if (!ParseWiperSwitch(wiperSwitch, &wiperSwitchValue)) {
            fprintf(stderr, "Invalid wiperSwitch at %u ms: %s\n", timeMs, wiperSwitch);
            continue;
        }

        /* Update speed reception */
        if (inputSpeed != SPEED_NOT_RECEIVED) {
            speed = inputSpeed;
            lastSpeedRxTime = timeMs;
        }

        /* Update gear reception */
        if (inputGear != GEAR_NOT_RECEIVED) {
            gearValue = inputGear;
            lastGearRxTime = timeMs;
        }

        /* Update engine reception */
        if (inputEngine != ENGINE_NOT_RECEIVED) {
            engineStateValue = inputEngine;
            lastEngineRxTime = timeMs;
        }

        /* Check reception timeout */
        RxState speedRxState =
            (timeMs - lastSpeedRxTime >= RX_TIMEOUT_MS)
            ? RX_TIMEOUT : RX_NORMAL;

        RxState gearRxState =
            (timeMs - lastGearRxTime >= RX_TIMEOUT_MS)
            ? RX_TIMEOUT : RX_NORMAL;

        RxState engineRxState =
            (timeMs - lastEngineRxTime >= RX_TIMEOUT_MS)
            ? RX_TIMEOUT : RX_NORMAL;

        /* 3. Monitor vehicle and sensor states */

        RxState vehicleRxState = CheckVehicleStateRx(
            speedRxState,
            gearRxState,
            engineRxState
        );

        SpeedState speedState = CheckSpeedValidity(
            vehicleRxState,
            speed
        );

        SensorState lightSensorState = CheckLightSensorFault(
            illuminance
        );

        SensorState rainSensorState = CheckRainSensorFault(
            rainfall
        );

        DoorOpenWarningState doorOpenWarningState = CheckDoorOpenWhileDriving(
            speedState,
            speed,
            doorFLValue,
            doorFRValue,
            doorRLValue,
            doorRRValue
        );

        ClusterWarning clusterWarning = DetermineClusterWarning(
            doorOpenWarningState,
            lightSensorState,
            rainSensorState
        );

        /* 4. Update timers */

        if (isFirstCycle) {
            isFirstCycle = 0;
        } else {
            lowIlluminanceTime = UpdateLowIlluminanceTime(
                illuminance,
                lowIlluminanceTime
            );

            highIlluminanceTime = UpdateHighIlluminanceTime(
                illuminance,
                highIlluminanceTime
            );

            wiperActiveTime = UpdateWiperActiveTime(
                wiperState,
                wiperActiveTime
            );
        }

        /* 5. Control vehicle functions */

        headLightState = ControlHeadLight(
            vehicleRxState,
            engineStateValue,
            lightSensorState,
            lightSwitchValue,
            wiperActiveTime,
            lowIlluminanceTime,
            highIlluminanceTime,
            headLightState
        );

        wiperState = ControlWiper(
            vehicleRxState,
            engineStateValue,
            wiperSwitchValue,
            rainSensorState,
            rainfall,
            speedState,
            speed
        );

        doorLockState = ControlDoorLock(
            vehicleRxState,
            engineStateValue,
            gearValue,
            speedState,
            speed,
            doorLockState
        );

        /* 6. Update vehicle outputs */

        WriteVehicleOutput(
            outputFile,
            timeMs,
            headLightState,
            lightSensorState,
            wiperState,
            rainSensorState,
            doorLockState,
            clusterWarning
        );
    }

    fclose(inputFile);
    fclose(outputFile);

    return 0;
}