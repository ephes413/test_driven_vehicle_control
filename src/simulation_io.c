#include "simulation_io.h"
#include <string.h>

int ParseGear(const char *str, Gear *value) {
    if (strcmp(str, "P") == 0) *value = GEAR_P;
    else if (strcmp(str, "R") == 0) *value = GEAR_R;
    else if (strcmp(str, "N") == 0) *value = GEAR_N;
    else if (strcmp(str, "D") == 0) *value = GEAR_D;
    else return 0;

    return 1;
}

int ParseEngineState(const char *str, EngineState *value) {
    if (strcmp(str, "ENGINE_OFF") == 0) *value = ENGINE_OFF;
    else if (strcmp(str, "ENGINE_ON") == 0) *value = ENGINE_ON;
    else return 0;

    return 1;
}

int ParseDoorState(const char *str, DoorState *value) {
    if (strcmp(str, "DOOR_CLOSED") == 0) *value = DOOR_CLOSED;
    else if (strcmp(str, "DOOR_OPEN") == 0) *value = DOOR_OPEN;
    else return 0;

    return 1;
}

int ParseLightSwitch(const char *str, LightSwitch *value) {
    if (strcmp(str, "LIGHT_AUTO") == 0) *value = LIGHT_AUTO;
    else if (strcmp(str, "LIGHT_OFF") == 0) *value = LIGHT_OFF;
    else if (strcmp(str, "LIGHT_ON") == 0) *value = LIGHT_ON;
    else return 0;

    return 1;
}

int ParseWiperSwitch(const char *str, WiperSwitch *value) {
    if (strcmp(str, "WIPER_SWITCH_OFF") == 0) *value = WIPER_SWITCH_OFF;
    else if (strcmp(str, "WIPER_SWITCH_AUTO") == 0) *value = WIPER_SWITCH_AUTO;
    else if (strcmp(str, "WIPER_SWITCH_LOW") == 0) *value = WIPER_SWITCH_LOW;
    else if (strcmp(str, "WIPER_SWITCH_HIGH") == 0) *value = WIPER_SWITCH_HIGH;
    else return 0;

    return 1;
}