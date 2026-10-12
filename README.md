# Test-Driven Vehicle Control

A C-based vehicle control simulation developed and verified using requirements-derived test cases. Python test tools execute CSV-driven unit tests, end-to-end integration tests, requirement-linked tests, and traceability-based change impact analysis.

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
├── requirements/
│   └── requirements.csv
├── traceability/
│   └── traceability.csv
├── test/
│   ├── test_unit.py
│   ├── test_integration.py
│   ├── test_requirement.py
│   ├── analyze_impact.py
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

The simulation reads vehicle and sensor inputs from `simulation/input.csv`, processes control logic, and writes results to `simulation/output.csv`.

### Execution Flow

Each cycle follows these steps:

1. `ReadVehicleInput()` — read and validate input.
2. `UpdateVehicleReception()` — update received signals and timeout states.
3. `MonitorVehicleState()` — evaluate vehicle and sensor states.
4. `UpdateVehicleTimers()` — update elapsed-time counters.
5. `UpdateVehicleControl()` — compute control outputs.
6. `WriteVehicleOutput()` — write simulation results.

The first cycle skips timer updates to preserve initial timer values.

### Modules

| Module | Responsibility |
| --- | --- |
| `main.c` | Initialization and execution flow |
| `simulation_io.c` | CSV input validation and output |
| `vehicle_simulation.c` | Reception, monitoring, timer, and control coordination |
| `vehicle_control.c` | Headlight, wiper, and door-lock control |
| `vehicle_monitor.c` | Signal validation, sensor fault detection, and warnings |
| `vehicle_timer.c` | Illuminance and wiper timers |

### Vehicle Reception Timeout

Vehicle speed, gear, and engine state are tracked independently. Missing values are represented by:

- Speed: `-1`
- Gear: `GEAR_NOT_RECEIVED`
- Engine: `ENGINE_NOT_RECEIVED`

Receiving a valid value updates its stored state and last reception timestamp. If no value arrives for 500 ms, the corresponding reception state becomes `RX_TIMEOUT`. The overall reception state is determined from these individual states.

### CSV Input and Output

Input file: `simulation/input.csv`

```csv
timeMs,speed,gear,engineState,illuminance,rainfall,doorFL,doorFR,doorRL,doorRR,lightSwitch,wiperSwitch
```

Output file: `simulation/output.csv`

```csv
timeMs,headLightState,lightSensorState,wiperState,rainSensorState,doorLockState,clusterWarning
```

Invalid input rows are reported and skipped. `ReadResult` distinguishes successful reads, end-of-file, and read errors.

## Build and Run

Requires Linux, GCC, Make, and Python 3. No third-party Python packages are required.

```bash
make          # Build vehicle_simulation
make run      # Build if needed, then run the simulation
make clean    # Remove the executable
```

The build compiles all `src/*.c` files with `-Wall -Wextra -std=c11 -Iinclude`.

## CSV-Driven Unit Testing

`test/test_unit.py` compiles C functions into a shared library, calls them through Python `ctypes`, and checks the results against CSV test cases. It does not reproduce the control algorithms in Python.

### Test Case Format

Each `test/testcases/<FunctionName>.csv` contains:

1. `testCaseId` in the first column.
2. Function parameters in declaration order.
3. The expected return value in the last column, named after the function.

Example: `test/testcases/ControlHeadLight.csv`

```csv
testCaseId,vehicleRxState,engineState,lightSensorState,lightSwitch,wiperActiveTime,lowIlluminanceTime,highIlluminanceTime,currentHeadLight,ControlHeadLight
HL_001,RX_TIMEOUT,,,,,,,,HEADLIGHT_OFF
HL_002,RX_NORMAL,ENGINE_OFF,,,,,,,HEADLIGHT_OFF
HL_003,RX_NORMAL,ENGINE_ON,SENSOR_FAULT,,,,,,HEADLIGHT_ON
```

Blank parameter cells mean **Don't Care** in the test specification. The runner currently passes zero for such cells; this is a placeholder, not exhaustive testing of every possible value.

### Test Case IDs

Test IDs use the format `<PREFIX>_<NUMBER>`.

| Prefix | Function |
| --- | --- |
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

### Running Unit Tests

```bash
make test-unit
python3 test/test_unit.py ControlWiper
python3 test/test_unit.py ControlDoorLock
python3 test/test_unit.py CheckLightSensorFault
```

The runner discovers supported declarations in `include/`, finds corresponding CSV files, resolves C enumeration values, and reports each case as PASS or FAIL. A failing test produces a nonzero exit status.

Example output:

```text
=== ControlWiper ===
[PASS] WP_001
[PASS] WP_002
...
ControlWiper: Passed=24, Failed=0

=== Total Results ===
Passed: 24, Failed: 0
```

To add tests, declare and implement a C function, create its matching CSV file in `test/testcases/`, and run the unit-test tool. Only functions with corresponding test CSV files are selected for testing.

## Requirement Traceability

`requirements/requirements.csv` records requirement IDs, categories, descriptions, priority, verification, and precedence. Requirements use the original IDs `L1`–`L6`, `W1`–`W5`, and `D1`–`D5`.

`traceability/traceability.csv` connects requirements to their source files, C functions, test CSV files, and test case IDs:

```csv
requirementId,sourceFile,function,testFile,testCaseId
```

One requirement can map to multiple cases, and one case can support multiple requirements. Traceability identifies relevant evidence but does not, by itself, establish complete verification coverage.

### Requirement-Based Testing

`test/test_requirement.py` runs the test cases linked to requirements through the traceability matrix.

```bash
make test-requirement       # All mapped requirements
make test-requirement L1    # Requirement L1
make test-requirement W3    # Requirement W3
make test-requirement D3    # Requirement D3
```

Results are summarized by requirement. Tests linked to a requirement are verification evidence, but passing them does not prove every scenario or timing constraint has been covered.

### Change Impact Analysis

`test/analyze_impact.py` reports the implementation and test artifacts associated with a requirement.

```bash
make analyze-impact D3
make analyze-impact L2
make analyze-impact W5
```

The output lists linked source files, functions, test files, and case IDs, with missing-file or missing-case indications where applicable. It is a list of **potential change impacts**, not an automatic source-code diff or a definitive list of required modifications.

## Automated Integration Testing

`test/test_integration.py` compiles and runs the full vehicle simulation, then compares the generated `simulation/output.csv` against `test/expected_output.csv` by `timeMs`.

The integration runner:

1. Validates input and expected-output CSV structures.
2. Compiles the C application using GCC (including `-Werror`).
3. Removes stale simulation output.
4. Runs the simulation.
5. Validates output columns and timestamps.
6. Compares expected and actual values at each timestamp.
7. Reports PASS/FAIL and exits nonzero on failure.

| File | Purpose |
| --- | --- |
| `simulation/input.csv` | Simulation inputs |
| `test/expected_output.csv` | Expected outputs |
| `simulation/output.csv` | Actual generated outputs |

```bash
make test-integration
# Alternatively:
python3 test/test_integration.py
```

The test detects mismatched values, missing or duplicate columns and timestamps, unexpected rows, and execution failures.

## Running All Tests

```bash
make test
```

This runs unit tests and integration tests. Requirement-based testing is a separate command and is **not** included in `make test`.

### Makefile Commands

| Command | Description |
| --- | --- |
| `make` | Build the simulator |
| `make run` | Build and run the simulator |
| `make test` | Run unit and integration tests |
| `make test-unit` | Run all CSV-driven unit tests |
| `make test-integration` | Run end-to-end integration tests |
| `make test-requirement` | Run tests mapped to all requirements |
| `make test-requirement L1` | Run tests mapped to requirement L1 |
| `make analyze-impact D3` | List artifacts potentially affected by requirement D3 |
| `make clean` | Delete the compiled simulator |

With the current Makefile, run positional requirement commands separately rather than combining multiple targets in one `make` invocation.

## Testing Scope and Limitations

The CSV-driven unit-test suite covers 11 functions across three modules:

| Module | Number of functions |
| --- | ---: |
| `vehicle_control.c` | 3 |
| `vehicle_monitor.c` | 5 |
| `vehicle_timer.c` | 3 |
| **Total** | **11** |

Integration tests cover input parsing, reception timeout handling, monitoring, timers, vehicle controls, cluster warnings, and output generation. All tests run on the host computer with GCC rather than on a target MCU.

Passing the defined test suite does not guarantee complete input-space coverage, code coverage, or correctness under all operating conditions.
