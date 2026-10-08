#include <stdio.h>
#include "vehicle_types.h"
#include "vehicle_control.h"
#include "vehicle_monitor.h"
#include "vehicle_timer.h"
#include "simulation_io.h"

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

    while (fgets(line, sizeof(line), inputFile) != NULL) {
        /* 1. Update vehicle inputs */

        unsigned int timeMs;
        int speed;
        char gear[16];
        char engineState[16];
        int illuminance;
        int rainfall;
        char doorFL[16];
        char doorFR[16];
        char doorRL[16];
        char doorRR[16];
        char lightSwitch[16];
        char wiperSwitch[16];

        int parsed = sscanf(
            line,
            "%u,%d,%15[^,],%15[^,],%d,%d,%15[^,],%15[^,],%15[^,],%15[^,],%15[^,],%15[^\r\n]",
            &timeMs,
            &speed,
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

        Gear gearValue;
        EngineState engineStateValue;
        DoorState doorFLValue;
        DoorState doorFRValue;
        DoorState doorRLValue;
        DoorState doorRRValue;
        LightSwitch lightSwitchValue;
        WiperSwitch wiperSwitchValue;

        if (!ParseGear(gear, &gearValue) ||
            !ParseEngineState(engineState, &engineStateValue) ||
            !ParseDoorState(doorFL, &doorFLValue) ||
            !ParseDoorState(doorFR, &doorFRValue) ||
            !ParseDoorState(doorRL, &doorRLValue) ||
            !ParseDoorState(doorRR, &doorRRValue) ||
            !ParseLightSwitch(lightSwitch, &lightSwitchValue) ||
            !ParseWiperSwitch(wiperSwitch, &wiperSwitchValue)) {

            fprintf(stderr, "Invalid enum input at %u ms\n", timeMs);
            continue;
        }

        /* CSV does not currently provide reception timeout information */
        RxState vehicleRxState = RX_NORMAL;

        SpeedState speedState = CheckSpeedValidity(
            vehicleRxState,
            speed
        );

        /* 3. Monitor vehicle and sensor states */

        /* 4. Update timers */

        /* 5. Control vehicle functions */

        /* 6. Determine cluster warning */

        /* 7. Update vehicle outputs */
    }

    fclose(inputFile);

    return 0;
}