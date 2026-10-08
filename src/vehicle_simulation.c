#include "vehicle_simulation.h"

#define RX_TIMEOUT_MS 500U
#define SPEED_NOT_RECEIVED (-1)

void UpdateVehicleReception(
    const VehicleInput *input,
    VehicleReception *reception
) {
    unsigned int timeMs = input->timeMs;

    /* Update speed reception */
    if (input->speed != SPEED_NOT_RECEIVED) {
        reception->speed = input->speed;
        reception->lastSpeedRxTime = timeMs;
    }

    /* Update gear reception */
    if (input->gear != GEAR_NOT_RECEIVED) {
        reception->gear = input->gear;
        reception->lastGearRxTime = timeMs;
    }

    /* Update engine reception */
    if (input->engineState != ENGINE_NOT_RECEIVED) {
        reception->engineState = input->engineState;
        reception->lastEngineRxTime = timeMs;
    }

    /* Check reception timeout */
    reception->speedRxState =
        (timeMs - reception->lastSpeedRxTime >= RX_TIMEOUT_MS)
        ? RX_TIMEOUT : RX_NORMAL;

    reception->gearRxState =
        (timeMs - reception->lastGearRxTime >= RX_TIMEOUT_MS)
        ? RX_TIMEOUT : RX_NORMAL;

    reception->engineRxState =
        (timeMs - reception->lastEngineRxTime >= RX_TIMEOUT_MS)
        ? RX_TIMEOUT : RX_NORMAL;
}