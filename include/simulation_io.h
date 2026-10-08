#ifndef SIMULATION_IO_H
#define SIMULATION_IO_H

#include "vehicle_types.h"

int ParseGear(const char *str, Gear *value);
int ParseEngineState(const char *str, EngineState *value);
int ParseDoorState(const char *str, DoorState *value);
int ParseLightSwitch(const char *str, LightSwitch *value);
int ParseWiperSwitch(const char *str, WiperSwitch *value);

#endif