# Test-Driven Vehicle Control

A vehicle control software project developed and verified using test cases derived from vehicle control requirements.

## Project Structure

```text
test_driven_vehicle_control/
├── README.md
├── src/
│   └── vehicle_control.c
├── include/
│   └── vehicle_control.h
├── test/
│   ├── test_driver.py
│   └── testcases/
│       ├── ControlHeadLight.csv
│       ├── ControlWiper.csv
│       ├── ControlDoorLock.csv
│       ├── CheckDoorOpenWhileDriving.csv
│       ├── CheckLightSensorFault.csv
│       ├── CheckRainSensorFault.csv
│       ├── DetermineClusterWarning.csv
│       └── CheckVehicleStateRx.csv
└── docs/
    └── requirements.md
```

## Test Case Format

Test cases are stored in CSV format.

- The first column contains the test case ID.
- Columns between the test case ID and the last column represent function parameters.
- Each row represents one test case.
- Each parameter column contains the argument passed to the function.
- An empty cell means "Don't Care".
- The last column is the function name and contains the expected return value.

Example:

```csv
testCaseId,vehicleRxState,engineState,lightSensorState,lightSwitch,wiperActiveTime,lowIlluminanceTime,highIlluminanceTime,currentHeadLight,ControlHeadLight
HL_001,RX_TIMEOUT,,,,,,,,HEADLIGHT_OFF
HL_002,RX_NORMAL,ENGINE_OFF,,,,,,,HEADLIGHT_OFF
HL_003,RX_NORMAL,ENGINE_ON,SENSOR_FAULT,,,,,,HEADLIGHT_ON
```

The example above represents test cases for:

```c
HeadLightState ControlHeadLight(
    RxState vehicleRxState,
    EngineState engineState,
    SensorState lightSensorState,
    LightSwitch lightSwitch,
    unsigned int wiperActiveTime,
    unsigned int lowIlluminanceTime,
    unsigned int highIlluminanceTime,
    HeadLightState currentHeadLight
);
```

## Test Case ID

Test case IDs follow the format:

```text
<PREFIX>_<NUMBER>
```

| Prefix | Function |
|---|---|
| `HL` | `ControlHeadLight()` |
| `WP` | `ControlWiper()` |
| `DL` | `ControlDoorLock()` |
| `DOW` | `CheckDoorOpenWhileDriving()` |
| `LSF` | `CheckLightSensorFault()` |
| `RSF` | `CheckRainSensorFault()` |
| `CW` | `DetermineClusterWarning()` |
| `VRX` | `CheckVehicleStateRx()` |

Examples:

```text
HL_001
HL_002
WP_001
LSF_003
```