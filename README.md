# Test-Driven Vehicle Control

A C-based vehicle driving and control simulation verified using requirements-derived, CSV-driven tests. The project includes unit testing, control integration testing, requirement-linked testing, traceability consistency checks, and change impact analysis.

## Project Structure

```text
test_driven_vehicle_control/
├── Makefile
├── README.md
├── include/
│   ├── simulation_io.h
│   ├── vehicle_control.h
│   ├── vehicle_driving.h
│   ├── vehicle_monitor.h
│   ├── vehicle_simulation.h
│   ├── vehicle_timer.h
│   └── vehicle_types.h
├── requirements/
│   └── requirements.csv
├── simulation/
│   ├── command.csv
│   ├── status.csv
│   └── result.csv
├── src/
│   ├── main.c
│   ├── simulation_io.c
│   ├── vehicle_control.c
│   ├── vehicle_driving.c
│   ├── vehicle_monitor.c
│   ├── vehicle_simulation.c
│   └── vehicle_timer.c
├── test/
│   ├── analyze_impact.py
│   ├── check_traceability.py
│   ├── expected_output.csv
│   ├── test_control.py
│   ├── test_requirement.py
│   ├── test_unit.py
│   └── testcases/
│       ├── CheckDoorOpenWhileDriving.csv
│       ├── CheckLightSensorFault.csv
│       ├── CheckRainSensorFault.csv
│       ├── CheckSpeedValidity.csv
│       ├── CheckVehicleStateRx.csv
│       ├── ControlDoorLock.csv
│       ├── ControlEngine.csv
│       ├── ControlGear.csv
│       ├── ControlHeadLight.csv
│       ├── ControlWiper.csv
│       ├── DetermineClusterWarning.csv
│       ├── UpdateHighIlluminanceTime.csv
│       ├── UpdateLowIlluminanceTime.csv
│       ├── UpdateVehicleSpeed.csv
│       └── UpdateWiperActiveTime.csv
├── traceability/
│   └── traceability.csv
└── vehicle_simulation          # Generated executable
```

## Simulation

The program runs two stages in order:

1. **Driving simulation:** Read `simulation/command.csv` and the existing `simulation/status.csv` row by row. Calculate engine state, gear, and speed; update those three columns in `status.csv` while retaining sensor, door, and switch values.
2. **Vehicle control simulation:** Read the updated `status.csv`, evaluate reception and sensor states, update timers and vehicle controls, and write `simulation/result.csv`.

### Driving stage

`RunDrivingSimulation()` invokes:

1. `ControlEngine()` — determine engine state from the command and current speed.
2. `ControlGear()` — determine gear from the command and current speed.
3. `UpdateVehicleSpeed()` — calculate speed from engine, gear, brake, and accelerator states.
4. `WriteVehicleStatus()` — write the updated status row.

The driving stage requires corresponding rows in `command.csv` and `status.csv` to have matching `timeMs` values. A mismatch or read error stops the stage. The updated status is written to a temporary file before replacing `status.csv` after successful processing.

### Vehicle control stage

`RunControlSimulation()` processes each status row in this order:

1. `UpdateVehicleReception()` — update received signals and timeout states.
2. `MonitorVehicleState()` — check reception, speed validity, sensor faults, and warnings.
3. `UpdateVehicleTimers()` — update illuminance and wiper timers (skipped on the first cycle).
4. `UpdateVehicleControl()` — compute headlight, wiper, and door-lock states.
5. `WriteVehicleOutput()` — write control results to `result.csv`.

The four update/monitor helper functions are internal (`static`) to `vehicle_simulation.c`.

### Modules

| Module | Responsibility |
| --- | --- |
| `main.c` | Invoke the driving and control stages |
| `simulation_io.c` | Open/close files, parse and write CSV rows, and replace the updated status file |
| `vehicle_simulation.c` | Coordinate driving and vehicle-control execution |
| `vehicle_driving.c` | Engine, gear, and speed calculation |
| `vehicle_control.c` | Headlight, wiper, and door-lock control |
| `vehicle_monitor.c` | Signal validity, sensor fault detection, and warnings |
| `vehicle_timer.c` | Illuminance and wiper timer calculations |

### CSV files

**Driver commands — `simulation/command.csv`**

```csv
timeMs,engineCommand,gearCommand,brakeCommand,acceleratorCommand
```

Supported commands include `ENGINE_CMD_OFF`/`ENGINE_CMD_ON`, `GEAR_CMD_P`/`GEAR_CMD_R`/`GEAR_CMD_N`/`GEAR_CMD_D`, and `PEDAL_RELEASED`/`PEDAL_PRESSED`.

**Vehicle status — `simulation/status.csv`**

```csv
timeMs,speed,gear,engineState,illuminance,rainfall,doorFL,doorFR,doorRL,doorRR,lightSwitch,wiperSwitch
```

The driving stage updates `speed`, `gear`, and `engineState`; it preserves the remaining values from the corresponding input row. This file is both an input and an updated artifact, so **running the simulator modifies it**. Preserve a copy if you need to restore the original test data.

**Control results — `simulation/result.csv`**

```csv
timeMs,headLightState,lightSensorState,wiperState,rainSensorState,doorLockState,clusterWarning
```

CSV enumeration fields must match the C enum names exactly. In the current parser, avoid spaces around field values. Keep command and status timestamps aligned.

### Reception timeout

Speed, gear, and engine reception are tracked independently. The existing status input uses these missing-value representations:

- Speed: `-1`
- Gear: `GEAR_NOT_RECEIVED`
- Engine: `ENGINE_NOT_RECEIVED`

A signal that has not been received for 500 ms enters `RX_TIMEOUT`. **Known limitation:** signed reverse speed can also equal `-1`, which conflicts with the existing missing-speed marker. Reception encoding should be revised before relying on reverse-driving scenarios involving `-1`.

## Build and Run

Requires GCC, Make, and Python 3 on a compatible host environment. The Makefile builds all `src/*.c` files using `-Wall -Wextra -std=c11 -Iinclude`.

```bash
make           # Build vehicle_simulation
make run       # Build if needed, then run both simulation stages
make clean     # Remove the executable
```

## Testing

### CSV-driven unit tests

`test/test_unit.py` runs function-level tests using CSV test cases in `test/testcases/`. Each file is named after the function under test and uses this format:

```csv
testCaseId,parameter1,parameter2,FunctionName
```

The first column identifies the test case, intermediate columns contain function inputs in parameter order, and the last column contains the expected return value. Blank input cells represent *Don't Care* in the test specification; check the runner's handling before assuming exhaustive coverage.

Examples:

```bash
make test-unit                       # All supported CSV unit tests
make test-unit ControlWiper          # One function
make test-unit ControlEngine
make test-unit ControlGear
make test-unit UpdateVehicleSpeed
```

The suite includes driving (`ControlEngine`, `ControlGear`, `UpdateVehicleSpeed`), vehicle-control, monitoring, and timer functions.

### Control integration tests

```bash
make test-control
```

Runs `test/test_control.py`. The repository also contains `test/expected_output.csv` for expected control results. The current Makefile has no separate `test-driving` target.

### All default tests

```bash
make test
```

Runs `test-unit` and `test-control`. Requirement-based testing and traceability checks are separate commands.

## Requirements and Traceability

`requirements/requirements.csv` defines requirements with the columns:

```csv
requirementId,category,description,priority,verification,precedence
```

`traceability/traceability.csv` links requirements to implementation functions and test cases:

```csv
requirementId,sourceFile,function,testFile,testCaseId
```

One requirement may map to multiple tests, and one test may support multiple requirements. The traceability mapping supports review and impact analysis; it does not by itself prove complete coverage.

### Requirement-based testing

```bash
make test-requirement              # Test cases mapped to requirements
make test-requirement L1           # Specific requirement
make test-requirement W3
make test-requirement R1
```

### Change impact analysis

```bash
make analyze-impact D3
make analyze-impact W5
make analyze-impact R3
```

`test/analyze_impact.py` reports artifacts linked to the selected requirement. These are potential impacts identified through traceability, not automatic source-code change detection.

### Traceability consistency

```bash
make check-traceability
```

Runs `test/check_traceability.py` to inspect consistency between requirements, implementation references, and test mappings.

### Makefile command reference

| Command | Purpose |
| --- | --- |
| `make` | Build the simulator |
| `make run` | Run driving and control simulations |
| `make test` | Run unit and control integration tests |
| `make test-unit` | Run all supported unit tests |
| `make test-unit ControlWiper` | Run unit tests for one function |
| `make test-control` | Run control integration tests |
| `make test-requirement` | Run requirement-linked tests |
| `make test-requirement R1` | Run tests linked to one requirement |
| `make analyze-impact R3` | Show potentially impacted artifacts |
| `make check-traceability` | Check traceability consistency |
| `make clean` | Remove the executable |

Positional arguments such as `ControlWiper` and `R1` are forwarded through the Makefile's auxiliary target handling. Run parameterized commands individually.

## Scope and Limitations

Tests execute on the host rather than a target automotive MCU. Passing tests demonstrates conformance to the defined cases, not complete input-space coverage, structural code coverage, or production functional-safety compliance. The driving simulation currently updates `status.csv` in place (via a temporary file), and its command/status inputs must be synchronized.
