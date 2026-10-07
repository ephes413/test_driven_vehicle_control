#ifndef VEHICLE_TYPES_H
#define VEHICLE_TYPES_H

/* ==================== 입력 ==================== */

/* 기어 */
typedef enum {
    GEAR_P,
    GEAR_R,
    GEAR_N,
    GEAR_D
} Gear;

/* 시동 */
typedef enum {
    ENGINE_OFF,
    ENGINE_ON
} EngineState;

/* 도어 */
typedef enum {
    DOOR_CLOSED,
    DOOR_OPEN
} DoorState;

/* 조명 스위치 */
typedef enum {
    LIGHT_AUTO,
    LIGHT_OFF,
    LIGHT_ON
} LightSwitch;

/* 와이퍼 스위치 */
typedef enum {
    WIPER_SWITCH_OFF,
    WIPER_SWITCH_AUTO,
    WIPER_SWITCH_LOW,
    WIPER_SWITCH_HIGH
} WiperSwitch;

typedef enum {
    RX_NORMAL,
    RX_TIMEOUT
} RxState;

/* ==================== 출력 ==================== */

/* 전조등 */
typedef enum {
    HEADLIGHT_OFF,
    HEADLIGHT_ON
} HeadLightState;

/* 센서 상태 */
typedef enum {
    SENSOR_NORMAL,
    SENSOR_FAULT
} SensorState;

/* 와이퍼 동작 */
typedef enum {
    WIPER_STOP,
    WIPER_INTERMITTENT,
    WIPER_LOW,
    WIPER_HIGH
} WiperState;

/* 도어 잠금 */
typedef enum {
    DOOR_UNLOCKED,
    DOOR_LOCKED
} DoorLockState;

/* 주행 중 도어 열림 상태 */
typedef enum {
    DOOR_OPEN_WARNING_INACTIVE,
    DOOR_OPEN_WARNING_ACTIVE
} DoorOpenWarningState;

/* 계기판 경고 */
typedef enum {
    WARNING_NONE,
    WARNING_DOOR_OPEN,
    WARNING_LIGHT_SENSOR_FAULT,
    WARNING_RAIN_SENSOR_FAULT
} ClusterWarning;

#endif