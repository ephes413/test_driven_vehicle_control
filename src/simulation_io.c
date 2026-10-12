#include "simulation_io.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define COMMAND_FILE "simulation/command.csv"
#define STATUS_FILE  "simulation/status.csv"
#define TEMP_FILE    "simulation/status.tmp"
#define RESULT_FILE  "simulation/result.csv"

static int ParseSpeed(const char *str, int *speed);
static int ParseGear(const char *str, GearState *value);
static int ParseEngineState(const char *str, EngineState *value);
static int ParseDoorState(const char *str, DoorState *value);
static int ParseLightSwitch(const char *str, LightSwitch *value);
static int ParseWiperSwitch(const char *str, WiperSwitch *value);
static int ParseEngineCommand(
    const char *str,
    EngineCommand *value
);

static int ParseGearCommand(
    const char *str,
    GearCommand *value
);

static int ParsePedalCommand(
    const char *str,
    PedalCommand *value
);

static int ParseSpeed(const char *str, int *speed) {
    if (strcmp(str, "SPEED_NOT_RECEIVED") == 0) {
        *speed = SPEED_NOT_RECEIVED;
        return 1;
    }

    char *end;
    long value;

    errno = 0;
    value = strtol(str, &end, 10);

    if (errno == ERANGE ||
        end == str ||
        *end != '\0' ||
        value < INT_MIN ||
        value > INT_MAX ||
        value == SPEED_NOT_RECEIVED) {
        return 0;
    }

    *speed = (int)value;
    return 1;
}

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

static int ParseEngineCommand(
    const char *str,
    EngineCommand *value
) {
    if (strcmp(str, "ENGINE_CMD_OFF") == 0)
        *value = ENGINE_CMD_OFF;
    else if (strcmp(str, "ENGINE_CMD_ON") == 0)
        *value = ENGINE_CMD_ON;
    else
        return 0;

    return 1;
}

static int ParseGearCommand(
    const char *str,
    GearCommand *value
) {
    if (strcmp(str, "GEAR_CMD_P") == 0)
        *value = GEAR_CMD_P;
    else if (strcmp(str, "GEAR_CMD_R") == 0)
        *value = GEAR_CMD_R;
    else if (strcmp(str, "GEAR_CMD_N") == 0)
        *value = GEAR_CMD_N;
    else if (strcmp(str, "GEAR_CMD_D") == 0)
        *value = GEAR_CMD_D;
    else
        return 0;

    return 1;
}

static int ParsePedalCommand(
    const char *str,
    PedalCommand *value
) {
    if (strcmp(str, "PEDAL_RELEASED") == 0)
        *value = PEDAL_RELEASED;
    else if (strcmp(str, "PEDAL_PRESSED") == 0)
        *value = PEDAL_PRESSED;
    else
        return 0;

    return 1;
}

ReadResult ReadVehicleCommand(
    FILE *commandFile,
    VehicleCommand *command
) {
    char line[256];

    if (fgets(line, sizeof(line), commandFile) == NULL) {
        return ferror(commandFile) ? READ_ERROR : READ_EOF;
    }

    VehicleCommand parsedCommand;

    char engineCommand[32];
    char gearCommand[32];
    char brakeCommand[32];
    char acceleratorCommand[32];

    int parsed = sscanf(
        line,
        " %u,%31[^,],%31[^,],%31[^,],%31[^\r\n]",
        &parsedCommand.timeMs,
        engineCommand,
        gearCommand,
        brakeCommand,
        acceleratorCommand
    );

    if (parsed != 5) {
        fprintf(stderr, "Invalid command CSV row: %s", line);
        return READ_ERROR;
    }

    if (!ParseEngineCommand(
        engineCommand,
        &parsedCommand.engineCommand
    )) {
        fprintf(stderr, "Invalid engineCommand: %s\n", engineCommand);
        return READ_ERROR;
    }

    if (!ParseGearCommand(
        gearCommand,
        &parsedCommand.gearCommand
    )) {
        fprintf(stderr, "Invalid gearCommand: %s\n", gearCommand);
        return READ_ERROR;
    }

    if (!ParsePedalCommand(
        brakeCommand,
        &parsedCommand.brakeCommand
    )) {
        fprintf(stderr, "Invalid brakeCommand: %s\n", brakeCommand);
        return READ_ERROR;
    }

    if (!ParsePedalCommand(
        acceleratorCommand,
        &parsedCommand.acceleratorCommand
    )) {
        fprintf(
            stderr,
            "Invalid acceleratorCommand: %s\n",
            acceleratorCommand
        );
        return READ_ERROR;
    }

    *command = parsedCommand;

    return READ_SUCCESS;
}

ReadResult ReadVehicleStatus(FILE *statusFile, VehicleStatus *status) {
    char line[256];

    while (fgets(line, sizeof(line), statusFile) != NULL) {
        VehicleStatus parsedStatus;

        char speed[32];
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
            "%u,%31[^,],%31[^,],%31[^,],%d,%d,"
            "%15[^,],%15[^,],%15[^,],%15[^,],"
            "%15[^,],%31[^\r\n]",
            &parsedStatus.timeMs,
            speed,
            gear,
            engineState,
            &parsedStatus.illuminance,
            &parsedStatus.rainfall,
            doorFL,
            doorFR,
            doorRL,
            doorRR,
            lightSwitch,
            wiperSwitch
        );

        if (parsed != 12) {
            fprintf(stderr, "Invalid CSV row: %s", line);
            return READ_ERROR;
        }

        if (!ParseSpeed(speed, &parsedStatus.speed)) {
            fprintf(stderr, "Invalid speed at %u ms: %s\n",
                parsedStatus.timeMs, speed);
            return READ_ERROR;
        }

        if (!ParseGear(gear, &parsedStatus.gear)) {
            fprintf(stderr, "Invalid gear at %u ms: %s\n",
                    parsedStatus.timeMs, gear);
            return READ_ERROR;
        }

        if (!ParseEngineState(engineState, &parsedStatus.engineState)) {
            fprintf(stderr, "Invalid engineState at %u ms: %s\n",
                    parsedStatus.timeMs, engineState);
            return READ_ERROR;
        }

        if (!ParseDoorState(doorFL, &parsedStatus.doorFL)) {
            fprintf(stderr, "Invalid doorFL at %u ms: %s\n",
                    parsedStatus.timeMs, doorFL);
            return READ_ERROR;
        }

        if (!ParseDoorState(doorFR, &parsedStatus.doorFR)) {
            fprintf(stderr, "Invalid doorFR at %u ms: %s\n",
                    parsedStatus.timeMs, doorFR);
            return READ_ERROR;
        }

        if (!ParseDoorState(doorRL, &parsedStatus.doorRL)) {
            fprintf(stderr, "Invalid doorRL at %u ms: %s\n",
                    parsedStatus.timeMs, doorRL);
            return READ_ERROR;
        }

        if (!ParseDoorState(doorRR, &parsedStatus.doorRR)) {
            fprintf(stderr, "Invalid doorRR at %u ms: %s\n",
                    parsedStatus.timeMs, doorRR);
            return READ_ERROR;
        }

        if (!ParseLightSwitch(lightSwitch, &parsedStatus.lightSwitch)) {
            fprintf(stderr, "Invalid lightSwitch at %u ms: %s\n",
                    parsedStatus.timeMs, lightSwitch);
            return READ_ERROR;
        }

        if (!ParseWiperSwitch(wiperSwitch, &parsedStatus.wiperSwitch)) {
            fprintf(stderr, "Invalid wiperSwitch at %u ms: %s\n",
                    parsedStatus.timeMs, wiperSwitch);
            return READ_ERROR;
        }

        *status = parsedStatus;
        return READ_SUCCESS;
    }

    return ferror(statusFile) ? READ_ERROR : READ_EOF;
}

void WriteVehicleStatus(
    FILE *statusFile,
    const VehicleStatus *status
) {
    const char *speedStr = NULL;
    char speedBuffer[32];

    if (status->speed == SPEED_NOT_RECEIVED) {
        speedStr = "SPEED_NOT_RECEIVED";
    } else {
        snprintf(speedBuffer, sizeof(speedBuffer), "%d", status->speed);
        speedStr = speedBuffer;
    }

    const char *gearStr;

    switch (status->gear) {
        case GEAR_P:            gearStr = "GEAR_P"; break;
        case GEAR_R:            gearStr = "GEAR_R"; break;
        case GEAR_N:            gearStr = "GEAR_N"; break;
        case GEAR_D:            gearStr = "GEAR_D"; break;
        case GEAR_NOT_RECEIVED: gearStr = "GEAR_NOT_RECEIVED"; break;
        default:                gearStr = "UNKNOWN"; break;
    }

    const char *engineStr;

    switch (status->engineState) {
        case ENGINE_OFF:          engineStr = "ENGINE_OFF"; break;
        case ENGINE_ON:           engineStr = "ENGINE_ON"; break;
        case ENGINE_NOT_RECEIVED: engineStr = "ENGINE_NOT_RECEIVED"; break;
        default:                  engineStr = "UNKNOWN"; break;
    }

    const char *doorFLStr =
        status->doorFL == DOOR_OPEN ? "DOOR_OPEN" : "DOOR_CLOSED";

    const char *doorFRStr =
        status->doorFR == DOOR_OPEN ? "DOOR_OPEN" : "DOOR_CLOSED";

    const char *doorRLStr =
        status->doorRL == DOOR_OPEN ? "DOOR_OPEN" : "DOOR_CLOSED";

    const char *doorRRStr =
        status->doorRR == DOOR_OPEN ? "DOOR_OPEN" : "DOOR_CLOSED";

    const char *lightSwitchStr;

    switch (status->lightSwitch) {
        case LIGHT_AUTO: lightSwitchStr = "LIGHT_AUTO"; break;
        case LIGHT_OFF:  lightSwitchStr = "LIGHT_OFF"; break;
        case LIGHT_ON:   lightSwitchStr = "LIGHT_ON"; break;
        default:         lightSwitchStr = "UNKNOWN"; break;
    }

    const char *wiperSwitchStr;

    switch (status->wiperSwitch) {
        case WIPER_SWITCH_OFF:
            wiperSwitchStr = "WIPER_SWITCH_OFF";
            break;
        case WIPER_SWITCH_AUTO:
            wiperSwitchStr = "WIPER_SWITCH_AUTO";
            break;
        case WIPER_SWITCH_LOW:
            wiperSwitchStr = "WIPER_SWITCH_LOW";
            break;
        case WIPER_SWITCH_HIGH:
            wiperSwitchStr = "WIPER_SWITCH_HIGH";
            break;
        default:
            wiperSwitchStr = "UNKNOWN";
            break;
    }

    fprintf(
        statusFile,
        "%u,%s,%s,%s,%d,%d,%s,%s,%s,%s,%s,%s\n",
        status->timeMs,
        speedStr,
        gearStr,
        engineStr,
        status->illuminance,
        status->rainfall,
        doorFLStr,
        doorFRStr,
        doorRLStr,
        doorRRStr,
        lightSwitchStr,
        wiperSwitchStr
    );
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

int OpenDrivingFiles(DrivingFiles *files) {
    char line[512];

    *files = (DrivingFiles){0};

    files->commandFile = fopen(COMMAND_FILE, "r");
    if (files->commandFile == NULL) {
        perror("Failed to open command.csv");
        return 1;
    }

    files->statusFile = fopen(STATUS_FILE, "r");
    if (files->statusFile == NULL) {
        perror("Failed to open status.csv");
        CloseDrivingFiles(files, 0);
        return 1;
    }

    files->tempFile = fopen(TEMP_FILE, "w");
    if (files->tempFile == NULL) {
        perror("Failed to open status.tmp");
        CloseDrivingFiles(files, 0);
        return 1;
    }

    if (fgets(line, sizeof(line), files->commandFile) == NULL ||
        fgets(line, sizeof(line), files->statusFile) == NULL) {
        fprintf(stderr, "Failed to read CSV headers\n");
        CloseDrivingFiles(files, 0);
        return 1;
    }

    if (fprintf(
        files->tempFile,
        "timeMs,speed,gear,engineState,illuminance,rainfall,"
        "doorFL,doorFR,doorRL,doorRR,lightSwitch,wiperSwitch\n"
    ) < 0) {
        CloseDrivingFiles(files, 0);
        return 1;
    }

    return 0;
}

int CloseDrivingFiles(DrivingFiles *files, int success) {
    int error = !success;

    if (files->commandFile != NULL) {
        if (fclose(files->commandFile) != 0) error = 1;
        files->commandFile = NULL;
    }

    if (files->statusFile != NULL) {
        if (fclose(files->statusFile) != 0) error = 1;
        files->statusFile = NULL;
    }

    if (files->tempFile != NULL) {
        if (fclose(files->tempFile) != 0) error = 1;
        files->tempFile = NULL;
    }

    if (error) {
        remove(TEMP_FILE);
        return 1;
    }

    if (rename(TEMP_FILE, STATUS_FILE) != 0) {
        perror("Failed to replace status.csv");
        remove(TEMP_FILE);
        return 1;
    }

    return 0;
}

int OpenControlFiles(ControlFiles *files) {
    char line[512];

    *files = (ControlFiles){0};

    files->statusFile = fopen(STATUS_FILE, "r");
    if (files->statusFile == NULL) {
        perror("Failed to open status.csv");
        return 1;
    }

    if (fgets(line, sizeof(line), files->statusFile) == NULL) {
        fprintf(stderr, "Failed to read status.csv header\n");
        CloseControlFiles(files);
        return 1;
    }

    files->resultFile = fopen(RESULT_FILE, "w");
    if (files->resultFile == NULL) {
        perror("Failed to open result.csv");
        CloseControlFiles(files);
        return 1;
    }

    if (fprintf(
        files->resultFile,
        "timeMs,headLightState,lightSensorState,wiperState,"
        "rainSensorState,doorLockState,clusterWarning\n"
    ) < 0) {
        CloseControlFiles(files);
        return 1;
    }

    return 0;
}

int CloseControlFiles(ControlFiles *files) {
    int error = 0;

    if (files->statusFile != NULL) {
        if (fclose(files->statusFile) != 0) error = 1;
        files->statusFile = NULL;
    }

    if (files->resultFile != NULL) {
        if (fclose(files->resultFile) != 0) error = 1;
        files->resultFile = NULL;
    }

    return error;
}
