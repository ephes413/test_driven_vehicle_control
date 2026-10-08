# Test-Driven Vehicle Control

A vehicle control software project developed and verified using test cases derived from vehicle control requirements.

## Project Structure

```text
test_driven_vehicle_control/
├── README.md
├── src/
│   ├── vehicle_control.c
│   ├── vehicle_monitor.c
│   └── vehicle_timer.c
├── include/
│   ├── vehicle_control.h
│   ├── vehicle_monitor.h
│   ├── vehicle_timer.h
│   └── vehicle_types.h
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
│       ├── CheckVehicleStateRx.csv
│       ├── UpdateLowIlluminanceTime.csv
│       ├── UpdateHighIlluminanceTime.csv
│       └── UpdateWiperActiveTime.csv
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
- Each CSV filename must match the corresponding function name.

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

Empty parameter cells are passed as zero by the test driver. These values are intended as placeholders for parameters that do not affect the expected result.

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
| `LIT` | `UpdateLowIlluminanceTime()` |
| `HIT` | `UpdateHighIlluminanceTime()` |
| `WAT` | `UpdateWiperActiveTime()` |

Examples:

```text
HL_001
HL_002
WP_001
LSF_003
LIT_001
HIT_001
WAT_001
```

## Automated Unit Testing

Unit tests are automated using `test/test_driver.py`.

The test driver uses Python's `ctypes` module to execute actual C functions compiled into a shared library.

### Test Procedure

1. Discover function declarations from header files in `include/`.
2. Extract function names, parameter types, and return types.
3. Compile C source files in `src/` into a shared library using GCC.
4. Load the compiled library using Python `ctypes`.
5. Read test cases from the corresponding CSV files.
6. Convert enumeration constants to their actual C values.
7. Call each C function with the specified test inputs.
8. Compare actual return values with expected results.
9. Report PASS/FAIL for each test case and summarize the results.

The test driver does not reimplement the vehicle control logic in Python.

### Requirements

- Python 3
- GCC
- Linux environment

No additional Python packages are required.

### Running Tests

Run the following commands from the project root directory.

**Run all tests:**

```bash
python3 test/test_driver.py
```

Alternatively:

```bash
python3 test/test_driver.py all
```

**Run tests for a specific function:**

```bash
python3 test/test_driver.py ControlWiper
```

Other examples:

```bash
python3 test/test_driver.py ControlDoorLock
python3 test/test_driver.py CheckLightSensorFault
python3 test/test_driver.py UpdateWiperActiveTime
```

The test driver automatically locates the corresponding CSV file using the specified function name.

### Test Results

Each test case is reported as PASS or FAIL.

Example:

```text
=== ControlWiper ===
[PASS] WP_001
[PASS] WP_002
[PASS] WP_003
...
[PASS] WP_024
ControlWiper: Passed=24, Failed=0

=== Total Results ===
Passed: 24, Failed: 0
```

When a test fails, the expected and actual return values are displayed.

Example:

```text
[FAIL] LSF_006: expected=SENSOR_FAULT (1), actual=0
```

The test runner returns a nonzero exit status when tests fail, allowing integration with automated build and CI workflows.

### Adding New Tests

To add test cases for an existing function:

1. Open the corresponding CSV file in `test/testcases/`.
2. Add new rows with test inputs and expected return values.
3. Run the test driver.

To test a new function:

1. Declare the function in a header file under `include/`.
2. Implement the function in a source file under `src/`.
3. Create a CSV file named after the function in `test/testcases/`.
4. Run the test driver.

The test driver automatically discovers supported function declarations without requiring manual registration.

The current implementation supports the project's simple function declarations and scalar parameter types, including integers and enumerations.

## Testing Scope

The project currently provides CSV-based unit tests for 11 functions across three modules.

| Module | Functions |
|---|---:|
| `vehicle_control.c` | 3 |
| `vehicle_monitor.c` | 5 |
| `vehicle_timer.c` | 3 |
| **Total** | **11** |

Tests are executed on the host computer using GCC rather than on a target microcontroller.

Passing all defined test cases does not necessarily indicate complete code coverage or correctness for every possible input.