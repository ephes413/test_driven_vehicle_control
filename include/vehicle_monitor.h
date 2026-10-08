#ifndef VEHICLE_MONITOR_H
#define VEHICLE_MONITOR_H

#include "vehicle_types.h"

DoorOpenWarningState CheckDoorOpenWhileDriving(
    int speed,
    DoorState doorFL,
    DoorState doorFR,
    DoorState doorRL,
    DoorState doorRR
);

SensorState CheckLightSensorFault(
    int illuminance
);

SensorState CheckRainSensorFault(
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

#endif