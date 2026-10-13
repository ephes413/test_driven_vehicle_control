#ifndef VEHICLE_MONITOR_H
#define VEHICLE_MONITOR_H

#include "vehicle_types.h"

DoorOpenWarningState CheckDoorOpenWhileDriving(
    SpeedState speedState,
    int speed,
    DoorState doorFL,
    DoorState doorFR,
    DoorState doorRL,
    DoorState doorRR
);

SensorState CheckLightSensorFault(
    RxState lightRxState,
    int illuminance
);

SensorState CheckRainSensorFault(
    RxState rainRxState,
    int rainAmount
);

ClusterWarning DetermineClusterWarning(
    DoorOpenWarningState doorOpenWarningState,
    SensorState lightSensorState,
    SensorState rainSensorState
);

RxState CheckVehicleStateRx(
    RxState speedRxState,
    RxState gearRxState,
    RxState engineRxState
);

SpeedState CheckSpeedValidity(
    RxState vehicleRxState,
    int speed
);

RxState CheckReceptionTimeout(
    unsigned int currentTime,
    unsigned int lastRxTime
);

#endif