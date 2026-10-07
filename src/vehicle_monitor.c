#include "vehicle_monitor.h"

SensorState CheckLightSensorFault(int illuminance) {
    if (illuminance < 0) return SENSOR_FAULT;
    if (illuminance > 60000) SENSOR_FAULT;
    return SENSOR_NORMAL;
}

SensorState CheckRainSensorFault(int rainAmount) {
    if (rainAmount < 0) return SENSOR_FAULT;
    if (rainAmount > 100) return SENSOR_FAULT;
    return SENSOR_NORMAL;
}

DoorOpenWarningState CheckDoorOpenWhileDriving(
    int speed,
    DoorState doorFL,
    DoorState doorFR,
    DoorState doorRL,
    DoorState doorRR
) {
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