#include "vehicle_control.h"

DoorLockState ControlDoorLock(
    RxState vehicleRxState,
    EngineState engineState,
    GearState gear,
    SpeedState speedState,
    int speed,
    DoorLockState currentDoorLock
) {
    if (vehicleRxState == RX_TIMEOUT) return currentDoorLock;
    if (engineState == ENGINE_OFF) return DOOR_UNLOCKED;
    if (gear == GEAR_P) return DOOR_UNLOCKED;
    if (speedState == SPEED_VALID && speed >= 15) return DOOR_LOCKED;
    return currentDoorLock;
}

WiperState ControlWiper(
    RxState vehicleRxState,
    EngineState engineState,
    WiperSwitch wiperSwitch,
    SensorState rainSensorState,
    int rainAmount,
    SpeedState speedState,
    int speed
) {
    if (vehicleRxState == RX_TIMEOUT) return WIPER_STOP;
    if (engineState == ENGINE_OFF) return WIPER_STOP;
    if (wiperSwitch == WIPER_SWITCH_OFF) return WIPER_STOP;
    if (wiperSwitch == WIPER_SWITCH_LOW) return WIPER_LOW;
    if (wiperSwitch == WIPER_SWITCH_HIGH) return WIPER_HIGH;
    if (rainSensorState == SENSOR_FAULT) return WIPER_INTERMITTENT;
    if (speedState == SPEED_VALID && speed == 0) {
        if (rainAmount >= 70) return WIPER_LOW;
        if (rainAmount >= 40) return WIPER_INTERMITTENT;
        return WIPER_STOP;
    }
    if (rainAmount >= 70) return WIPER_HIGH;
    if (rainAmount >= 40) return WIPER_LOW;
    if (rainAmount >= 10) return WIPER_INTERMITTENT;
    return WIPER_STOP;
}

HeadLightState ControlHeadLight(
    RxState vehicleRxState,
    EngineState engineState,
    SensorState lightSensorState,
    LightSwitch lightSwitch,
    unsigned int wiperActiveTime,
    unsigned int lowIlluminanceTime,
    unsigned int highIlluminanceTime,
    HeadLightState currentHeadLight
) {
    if (vehicleRxState == RX_TIMEOUT) return HEADLIGHT_OFF;
    if (engineState == ENGINE_OFF) return HEADLIGHT_OFF;
    if (lightSensorState == SENSOR_FAULT) return HEADLIGHT_ON;
    if (lightSwitch == LIGHT_OFF) return HEADLIGHT_OFF;
    if (lightSwitch == LIGHT_ON) return HEADLIGHT_ON;
    if (wiperActiveTime >= 10000) return HEADLIGHT_ON;
    if (lowIlluminanceTime >= 1000) return HEADLIGHT_ON;
    if (highIlluminanceTime >= 3000) return HEADLIGHT_OFF;
    return currentHeadLight;
}