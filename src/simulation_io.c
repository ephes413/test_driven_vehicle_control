#include "simulation_io.h"
#include <string.h>

static int ParseGear(const char *str, GearState *value);
static int ParseEngineState(const char *str, EngineState *value);
static int ParseDoorState(const char *str, DoorState *value);
static int ParseLightSwitch(const char *str, LightSwitch *value);
static int ParseWiperSwitch(const char *str, WiperSwitch *value);

static int ParseGear(const char *str, GearState *value) {
    if (strcmp(str, "GEAR_P") == 0) *value = GEAR_P;
    else if (strcmp(str, "GEAR_R") == 0) *value = GEAR_R;
    else if (strcmp(str, "GEAR_N") == 0) *value = GEAR_N;
    else if (strcmp(str, "GEAR_D") == 0) *value = GEAR_D;
    else if (strcmp(str, "GEAR_NOT_RECEIVED") == 0) *value = GEAR_NOT_RECEIVED;
    else return 0;

    return 1;
}

static int ParseEngineState(const char *str, EngineState *value) {
    if (strcmp(str, "ENGINE_OFF") == 0) *value = ENGINE_OFF;
    else if (strcmp(str, "ENGINE_ON") == 0) *value = ENGINE_ON;
    else if (strcmp(str, "ENGINE_NOT_RECEIVED") == 0) *value = ENGINE_NOT_RECEIVED;
    else return 0;

    return 1;
}

static int ParseDoorState(const char *str, DoorState *value) {
    if (strcmp(str, "DOOR_CLOSED") == 0) *value = DOOR_CLOSED;
    else if (strcmp(str, "DOOR_OPEN") == 0) *value = DOOR_OPEN;
    else return 0;

    return 1;
}

static int ParseLightSwitch(const char *str, LightSwitch *value) {
    if (strcmp(str, "LIGHT_AUTO") == 0) *value = LIGHT_AUTO;
    else if (strcmp(str, "LIGHT_OFF") == 0) *value = LIGHT_OFF;
    else if (strcmp(str, "LIGHT_ON") == 0) *value = LIGHT_ON;
    else return 0;

    return 1;
}

static int ParseWiperSwitch(const char *str, WiperSwitch *value) {
    if (strcmp(str, "WIPER_SWITCH_OFF") == 0) *value = WIPER_SWITCH_OFF;
    else if (strcmp(str, "WIPER_SWITCH_AUTO") == 0) *value = WIPER_SWITCH_AUTO;
    else if (strcmp(str, "WIPER_SWITCH_LOW") == 0) *value = WIPER_SWITCH_LOW;
    else if (strcmp(str, "WIPER_SWITCH_HIGH") == 0) *value = WIPER_SWITCH_HIGH;
    else return 0;

    return 1;
}

ReadResult ReadVehicleInput(FILE *inputFile, VehicleInput *input) {
    char line[256];

    while (fgets(line, sizeof(line), inputFile) != NULL) {
        VehicleInput parsedInput;

        char gear[32];
        char engineState[32];
        char doorFL[16];
        char doorFR[16];
        char doorRL[16];
        char doorRR[16];
        char lightSwitch[16];
        char wiperSwitch[32];

        int parsed = sscanf(
            line,
            "%u,%d,%31[^,],%31[^,],%d,%d,"
            "%15[^,],%15[^,],%15[^,],%15[^,],"
            "%15[^,],%31[^\r\n]",
            &parsedInput.timeMs,
            &parsedInput.speed,
            gear,
            engineState,
            &parsedInput.illuminance,
            &parsedInput.rainfall,
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

        if (!ParseGear(gear, &parsedInput.gear)) {
            fprintf(stderr, "Invalid gear at %u ms: %s\n",
                    parsedInput.timeMs, gear);
            continue;
        }

        if (!ParseEngineState(engineState, &parsedInput.engineState)) {
            fprintf(stderr, "Invalid engineState at %u ms: %s\n",
                    parsedInput.timeMs, engineState);
            continue;
        }

        if (!ParseDoorState(doorFL, &parsedInput.doorFL)) {
            fprintf(stderr, "Invalid doorFL at %u ms: %s\n",
                    parsedInput.timeMs, doorFL);
            continue;
        }

        if (!ParseDoorState(doorFR, &parsedInput.doorFR)) {
            fprintf(stderr, "Invalid doorFR at %u ms: %s\n",
                    parsedInput.timeMs, doorFR);
            continue;
        }

        if (!ParseDoorState(doorRL, &parsedInput.doorRL)) {
            fprintf(stderr, "Invalid doorRL at %u ms: %s\n",
                    parsedInput.timeMs, doorRL);
            continue;
        }

        if (!ParseDoorState(doorRR, &parsedInput.doorRR)) {
            fprintf(stderr, "Invalid doorRR at %u ms: %s\n",
                    parsedInput.timeMs, doorRR);
            continue;
        }

        if (!ParseLightSwitch(lightSwitch, &parsedInput.lightSwitch)) {
            fprintf(stderr, "Invalid lightSwitch at %u ms: %s\n",
                    parsedInput.timeMs, lightSwitch);
            continue;
        }

        if (!ParseWiperSwitch(wiperSwitch, &parsedInput.wiperSwitch)) {
            fprintf(stderr, "Invalid wiperSwitch at %u ms: %s\n",
                    parsedInput.timeMs, wiperSwitch);
            continue;
        }

        *input = parsedInput;
        return READ_SUCCESS;
    }

    return ferror(inputFile) ? READ_ERROR : READ_EOF;
}

void WriteVehicleOutput(
    FILE *outputFile,
    unsigned int timeMs,
    HeadLightState headLightState,
    SensorState lightSensorState,
    WiperState wiperState,
    SensorState rainSensorState,
    DoorLockState doorLockState,
    ClusterWarning clusterWarning
) {
    const char *headLightStr =
        headLightState == HEADLIGHT_ON ? "HEADLIGHT_ON" : "HEADLIGHT_OFF";

    const char *lightSensorStr =
        lightSensorState == SENSOR_FAULT ? "SENSOR_FAULT" : "SENSOR_NORMAL";

    const char *wiperStr;
    switch (wiperState) {
        case WIPER_STOP:         wiperStr = "WIPER_STOP"; break;
        case WIPER_INTERMITTENT: wiperStr = "WIPER_INTERMITTENT"; break;
        case WIPER_LOW:          wiperStr = "WIPER_LOW"; break;
        case WIPER_HIGH:         wiperStr = "WIPER_HIGH"; break;
        default:                 wiperStr = "UNKNOWN"; break;
    }

    const char *rainSensorStr =
        rainSensorState == SENSOR_FAULT ? "SENSOR_FAULT" : "SENSOR_NORMAL";

    const char *doorLockStr =
        doorLockState == DOOR_LOCKED ? "DOOR_LOCKED" : "DOOR_UNLOCKED";

    const char *warningStr;
    switch (clusterWarning) {
        case WARNING_NONE:              warningStr = "WARNING_NONE"; break;
        case WARNING_DOOR_OPEN:         warningStr = "WARNING_DOOR_OPEN"; break;
        case WARNING_LIGHT_SENSOR_FAULT:
            warningStr = "WARNING_LIGHT_SENSOR_FAULT"; break;
        case WARNING_RAIN_SENSOR_FAULT:
            warningStr = "WARNING_RAIN_SENSOR_FAULT"; break;
        default:                        warningStr = "UNKNOWN"; break;
    }

    fprintf(
        outputFile,
        "%u,%s,%s,%s,%s,%s,%s\n",
        timeMs,
        headLightStr,
        lightSensorStr,
        wiperStr,
        rainSensorStr,
        doorLockStr,
        warningStr
    );
}
