# Test-Driven Vehicle Control

A vehicle control software project developed and verified using test cases derived from vehicle control requirements.

The project implements vehicle monitoring and control functions in C, with automated unit and integration testing using Python and CSV files.

## Project Structure

```text
test_driven_vehicle_control/
├── Makefile
├── README.md
├── src/
│   ├── main.c
│   ├── simulation_io.c
│   ├── vehicle_simulation.c
│   ├── vehicle_control.c
│   ├── vehicle_monitor.c
│   └── vehicle_timer.c
├── include/
│   ├── simulation_io.h
│   ├── vehicle_simulation.h
│   ├── vehicle_control.h
│   ├── vehicle_monitor.h
│   ├── vehicle_timer.h
│   └── vehicle_types.h
├── simulation/
│   ├── input.csv
│   └── output.csv
├── test/
│   ├── test_driver.py
│   ├── test_integration.py
│   ├── expected_output.csv
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

## Vehicle Simulation

The vehicle simulation reads input data from a CSV file, processes vehicle and sensor states, executes control functions, and writes the results to an output CSV file.

### Simulation Procedure

Each simulation cycle executes the following steps:

1. Read and validate vehicle inputs using `ReadVehicleInput()`.
2. Update received vehicle data and reception timeout states using `UpdateVehicleReception()`.
3. Monitor vehicle and sensor states using `MonitorVehicleState()`.
4. Update control timers using `UpdateVehicleTimers()`.
5. Execute vehicle control functions using `UpdateVehicleControl()`.
6. Write simulation results using `WriteVehicleOutput()`.

The first simulation cycle skips timer updates to preserve the initial timer values.

### Simulation Modules

| Module | Responsibility |
|---|---|
| `main.c` | Simulation initialization and execution flow |
| `simulation_io.c` | CSV input parsing, validation, and output |
| `vehicle_simulation.c` | Vehicle reception, monitoring, timer, and control coordination |
| `vehicle_control.c` | Headlight, wiper, and door lock control |
| `vehicle_monitor.c` | Vehicle state validation, sensor fault detection, and warning determination |
| `vehicle_timer.c` | Illuminance and wiper operation timer updates |

### Vehicle Reception Timeout

The simulation tracks the reception of vehicle speed, gear, and engine state independently.

Missing reception is represented by:

- Speed: `-1`
- Gear: `GEAR_NOT_RECEIVED`
- Engine: `ENGINE_NOT_RECEIVED`

When a valid value is received, the corresponding stored value and last reception timestamp are updated.

If a signal is not received for 500 ms, its reception state changes to `RX_TIMEOUT`.

The overall vehicle reception state is determined from the individual reception states.

### Simulation Input and Output

The simulation reads:

```text
simulation/input.csv
```

Input columns:

```csv
timeMs,speed,gear,engineState,illuminance,rainfall,doorFL,doorFR,doorRL,doorRR,lightSwitch,wiperSwitch
```

The simulation generates:

```text
simulation/output.csv
```

Output columns:

```csv
timeMs,headLightState,lightSensorState,wiperState,rainSensorState,doorLockState,clusterWarning
```

Invalid CSV rows are reported and skipped by the input parser.

The simulation distinguishes successful input reading, end-of-file, and file read errors using the `ReadResult` enumeration.

## Build and Execution

The project uses GCC and Make.

### Build

```bash
make
```

This compiles all C source files in `src/` and generates the `vehicle_simulation` executable.

### Run Simulation

```bash
make run
```

This builds the executable if necessary and runs the simulation.

### Clean

```bash
make clean
```

This removes the generated executable.

## Test Case Format

Unit test cases are stored in CSV format.

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
3. Compile C source files into a shared library using GCC.
4. Load the compiled library using Python `ctypes`.
5. Read test cases from the corresponding CSV files.
6. Convert enumeration constants to their actual C values.
7. Call each C function with the specified test inputs.
8. Compare actual return values with expected results.
9. Report PASS/FAIL for each test case and summarize the results.

The test driver does not reimplement the vehicle control logic in Python.

### Running Unit Tests

Run all unit tests:

```bash
make test-unit
```

Alternatively:

```bash
python3 test/test_driver.py
```

Run tests for a specific function:

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

### Unit Test Results

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

The test runner returns a nonzero exit status when tests fail.

### Adding New Unit Tests

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

## Automated Integration Testing

Integration tests are automated using `test/test_integration.py`.

Unlike unit tests, which verify individual functions, integration tests execute the complete vehicle simulation and compare its output against predefined expected results.

### Integration Test Procedure

1. Validate the structure of `simulation/input.csv` and `test/expected_output.csv`.
2. Compile all C source files into a temporary executable using GCC.
3. Remove any existing `simulation/output.csv` to prevent comparison with stale output.
4. Execute the complete vehicle simulation.
5. Read the generated `simulation/output.csv`.
6. Validate CSV column names and timestamp values.
7. Match expected and actual output rows using `timeMs`.
8. Compare each output field against its expected value.
9. Report PASS/FAIL for each timestamp and summarize the results.

The integration test compiles the application with `-Wall`, `-Wextra`, and `-Werror`.

### Integration Test Data

Integration testing uses three CSV files.

| File | Purpose |
|---|---|
| `simulation/input.csv` | Vehicle and sensor input data |
| `test/expected_output.csv` | Expected simulation results |
| `simulation/output.csv` | Actual results generated by the simulation |

Each expected output row corresponds to a simulation timestamp.

The integration test compares rows using `timeMs` rather than relying on row positions.

### Running Integration Tests

```bash
make test-integration
```

Alternatively:

```bash
python3 test/test_integration.py
```

### Integration Test Results

Each timestamp is reported as PASS or FAIL.

Example:

```text
=== Integration Test ===
[PASS] timeMs=0
[PASS] timeMs=100
[PASS] timeMs=200
[PASS] timeMs=300

Passed: 4
Failed: 0
```

When an output differs from the expected value, the test displays the timestamp, field name, expected value, and actual value.

Example:

```text
[FAIL] timeMs=500
  headLightState:
    expected = HEADLIGHT_ON
    actual   = HEADLIGHT_OFF
```

The integration test also detects:

- Missing or duplicate CSV column names.
- Invalid or duplicate timestamps.
- Missing expected or actual output rows.
- Unexpected output rows.
- Differences between input and expected row counts.
- Compilation or simulation execution failures.

The test returns a nonzero exit status when an error or mismatch is detected.

## Running All Tests

The Makefile provides a single command to execute both unit and integration tests.

```bash
make test
```

This executes:

1. `test/test_driver.py`
2. `test/test_integration.py`

Both test stages must complete successfully for the overall test command to succeed.

### Makefile Commands

| Command | Description |
|---|---|
| `make` | Build the vehicle simulation executable |
| `make run` | Build and run the vehicle simulation |
| `make test` | Run unit and integration tests |
| `make test-unit` | Run unit tests |
| `make test-integration` | Run integration tests |
| `make clean` | Remove the generated executable |

## Testing Scope

The project provides CSV-based unit tests for 11 functions across three modules, together with end-to-end integration testing of the vehicle simulation.

### Unit Testing

| Module | Functions |
|---|---:|
| `vehicle_control.c` | 3 |
| `vehicle_monitor.c` | 5 |
| `vehicle_timer.c` | 3 |
| **Total** | **11** |

### Integration Testing

Integration testing verifies the combined behavior of:

- Vehicle input parsing and validation.
- Vehicle data reception and timeout handling.
- Vehicle and sensor state monitoring.
- Timer updates.
- Headlight, wiper, and door lock control.
- Cluster warning determination.
- CSV output generation.

Tests are executed on the host computer using GCC rather than on a target microcontroller.

Passing all defined tests does not necessarily indicate complete code coverage or correctness for every possible input.

## Requirements

- Python 3
- GCC
- Make
- Linux environment

No additional Python packages are required.