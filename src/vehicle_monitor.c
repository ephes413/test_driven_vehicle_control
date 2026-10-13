#include "vehicle_monitor.h"

#define RX_TIMEOUT_MS 500U

SensorState CheckLightSensorFault(
    RxState lightRxState,
    int illuminance
) {
    if (lightRxState == RX_TIMEOUT) return SENSOR_FAULT;
    if (illuminance < 0) return SENSOR_FAULT;
    if (illuminance > 60000) return SENSOR_FAULT;
    return SENSOR_NORMAL;
}

SensorState CheckRainSensorFault(
    RxState rainRxState,
    int rainAmount
) {
    if (rainRxState == RX_TIMEOUT) return SENSOR_FAULT;
    if (rainAmount < 0) return SENSOR_FAULT;
    if (rainAmount > 100) return SENSOR_FAULT;
    return SENSOR_NORMAL;
}

DoorOpenWarningState CheckDoorOpenWhileDriving(
    SpeedState speedState,
    int speed,
    DoorState doorFL,
    DoorState doorFR,
    DoorState doorRL,
    DoorState doorRR
) {
    if (speedState == SPEED_INVALID) return DOOR_OPEN_WARNING_INACTIVE;
    if (speed == 0) return DOOR_OPEN_WARNING_INACTIVE;
    if (doorFL == DOOR_OPEN) return DOOR_OPEN_WARNING_ACTIVE;
    if (doorFR == DOOR_OPEN) return DOOR_OPEN_WARNING_ACTIVE;
    if (doorRL == DOOR_OPEN) return DOOR_OPEN_WARNING_ACTIVE;
    if (doorRR == DOOR_OPEN) return DOOR_OPEN_WARNING_ACTIVE;
    return DOOR_OPEN_WARNING_INACTIVE;
}

RxState CheckVehicleStateRx(
    RxState speedRxState,
    RxState gearRxState,
    RxState engineRxState
) {
    if (speedRxState == RX_TIMEOUT) return RX_TIMEOUT;
    if (gearRxState == RX_TIMEOUT) return RX_TIMEOUT;
    if (engineRxState == RX_TIMEOUT) return RX_TIMEOUT;
    return RX_NORMAL;
}

ClusterWarning DetermineClusterWarning(
    DoorOpenWarningState doorOpenWarningState,
    SensorState lightSensorState,
    SensorState rainSensorState
) {
    if (doorOpenWarningState == DOOR_OPEN_WARNING_ACTIVE) return WARNING_DOOR_OPEN;
    if (lightSensorState == SENSOR_FAULT) return WARNING_LIGHT_SENSOR_FAULT;
    if (rainSensorState == SENSOR_FAULT) return WARNING_RAIN_SENSOR_FAULT;
    return WARNING_NONE;
}

SpeedState CheckSpeedValidity(
    RxState vehicleRxState,
    int speed
) {
    if (vehicleRxState == RX_TIMEOUT) return SPEED_INVALID;
    if (speed < -20) return SPEED_INVALID;
    if (speed > 250) return SPEED_INVALID;
    return SPEED_VALID;
}

RxState CheckReceptionTimeout(
    unsigned int currentTime,
    unsigned int lastRxTime
) {
    if (currentTime - lastRxTime >= RX_TIMEOUT_MS)
        return RX_TIMEOUT;

    return RX_NORMAL;
}
