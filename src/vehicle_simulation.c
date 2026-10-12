#include "vehicle_simulation.h"
#include "vehicle_monitor.h"
#include "vehicle_timer.h"
#include "vehicle_control.h"
#include "vehicle_driving.h"
#include <limits.h>

#define RX_TIMEOUT_MS 500U
#define SPEED_NOT_RECEIVED INT_MIN

static void UpdateVehicleReception(
    const VehicleStatus *input,
    VehicleReception *reception
);

static void MonitorVehicleState(
    const VehicleStatus *input,
    const VehicleReception *reception,
    VehicleMonitor *monitor
);

static void UpdateVehicleTimers(
    const VehicleStatus *input,
    VehicleControlState *control
);

static void UpdateVehicleControl(
    const VehicleStatus *input,
    const VehicleReception *reception,
    const VehicleMonitor *monitor,
    VehicleControlState *control
);

static void UpdateVehicleReception(
    const VehicleStatus *input,
    VehicleReception *reception
) {
    unsigned int timeMs = input->timeMs;

    /* Update speed reception */
    if (input->speed != SPEED_NOT_RECEIVED) {
        reception->speed = input->speed;
        reception->lastSpeedRxTime = timeMs;
    }

    /* Update gear reception */
    if (input->gear != GEAR_NOT_RECEIVED) {
        reception->gear = input->gear;
        reception->lastGearRxTime = timeMs;
    }

    /* Update engine reception */
    if (input->engineState != ENGINE_NOT_RECEIVED) {
        reception->engineState = input->engineState;
        reception->lastEngineRxTime = timeMs;
    }

    /* Check reception timeout */
    reception->speedRxState =
        (timeMs - reception->lastSpeedRxTime >= RX_TIMEOUT_MS)
        ? RX_TIMEOUT : RX_NORMAL;

    reception->gearRxState =
        (timeMs - reception->lastGearRxTime >= RX_TIMEOUT_MS)
        ? RX_TIMEOUT : RX_NORMAL;

    reception->engineRxState =
        (timeMs - reception->lastEngineRxTime >= RX_TIMEOUT_MS)
        ? RX_TIMEOUT : RX_NORMAL;
}

static void MonitorVehicleState(
    const VehicleStatus *input,
    const VehicleReception *reception,
    VehicleMonitor *monitor
) {
    monitor->vehicleRxState = CheckVehicleStateRx(
        reception->speedRxState,
        reception->gearRxState,
        reception->engineRxState
    );

    monitor->speedState = CheckSpeedValidity(
        monitor->vehicleRxState,
        reception->speed
    );

    monitor->lightSensorState = CheckLightSensorFault(
        input->illuminance
    );

    monitor->rainSensorState = CheckRainSensorFault(
        input->rainfall
    );

    monitor->doorOpenWarningState = CheckDoorOpenWhileDriving(
        monitor->speedState,
        reception->speed,
        input->doorFL,
        input->doorFR,
        input->doorRL,
        input->doorRR
    );

    monitor->clusterWarning = DetermineClusterWarning(
        monitor->doorOpenWarningState,
        monitor->lightSensorState,
        monitor->rainSensorState
    );
}

static void UpdateVehicleTimers(
    const VehicleStatus *input,
    VehicleControlState *control
) {
    control->lowIlluminanceTime = UpdateLowIlluminanceTime(
        input->illuminance,
        control->lowIlluminanceTime
    );

    control->highIlluminanceTime = UpdateHighIlluminanceTime(
        input->illuminance,
        control->highIlluminanceTime
    );

    control->wiperActiveTime = UpdateWiperActiveTime(
        control->wiperState,
        control->wiperActiveTime
    );
}

static void UpdateVehicleControl(
    const VehicleStatus *input,
    const VehicleReception *reception,
    const VehicleMonitor *monitor,
    VehicleControlState *control
) {
    control->headLightState = ControlHeadLight(
        monitor->vehicleRxState,
        reception->engineState,
        monitor->lightSensorState,
        input->lightSwitch,
        control->wiperActiveTime,
        control->lowIlluminanceTime,
        control->highIlluminanceTime,
        control->headLightState
    );

    control->wiperState = ControlWiper(
        monitor->vehicleRxState,
        reception->engineState,
        input->wiperSwitch,
        monitor->rainSensorState,
        input->rainfall,
        monitor->speedState,
        reception->speed
    );

    control->doorLockState = ControlDoorLock(
        monitor->vehicleRxState,
        reception->engineState,
        reception->gear,
        monitor->speedState,
        reception->speed,
        control->doorLockState
    );
}

int RunDrivingSimulation(void) {
    DrivingFiles files;

    if (OpenDrivingFiles(&files) != 0) {
        return 1;
    }

    VehicleCommand command = {0};
    VehicleStatus status = {0};

    int speed = 0;
    GearState gear = GEAR_P;
    EngineState engineState = ENGINE_OFF;

    int error = 0;
    ReadResult commandResult;
    ReadResult statusResult;

    while (1) {
        commandResult = ReadVehicleCommand(
            files.commandFile,
            &command
        );

        statusResult = ReadVehicleStatus(
            files.statusFile,
            &status
        );

        if (commandResult == READ_ERROR ||
            statusResult == READ_ERROR) {
            fprintf(stderr, "Failed to read simulation CSV\n");
            error = 1;
            break;
        }

        if (commandResult == READ_EOF &&
            statusResult == READ_EOF) {
            break;
        }

        if (commandResult != READ_SUCCESS ||
            statusResult != READ_SUCCESS) {
            fprintf(stderr, "CSV row count mismatch\n");
            error = 1;
            break;
        }

        if (command.timeMs != status.timeMs) {
            fprintf(
                stderr,
                "Timestamp mismatch: command=%u, status=%u\n",
                command.timeMs,
                status.timeMs
            );
            error = 1;
            break;
        }

        engineState = ControlEngine(
            speed,
            engineState,
            command.engineCommand
        );

        gear = ControlGear(
            speed,
            gear,
            command.gearCommand
        );

        speed = UpdateVehicleSpeed(
            engineState,
            gear,
            command.brakeCommand,
            command.acceleratorCommand,
            speed
        );

        status.speed = speed;
        status.gear = gear;
        status.engineState = engineState;

        WriteVehicleStatus(
            files.tempFile,
            &status
        );

        if (ferror(files.tempFile)) {
            fprintf(stderr, "Failed to write vehicle status\n");
            error = 1;
            break;
        }
    }

    return CloseDrivingFiles(&files, !error);
}

int RunControlSimulation(void) {
    ControlFiles files;

    if (OpenControlFiles(&files) != 0) {
        return 1;
    }

    VehicleStatus input = {0};

    VehicleReception reception = {
        .speed = 0,
        .gear = GEAR_P,
        .engineState = ENGINE_OFF,
        .lastSpeedRxTime = 0,
        .lastGearRxTime = 0,
        .lastEngineRxTime = 0,
        .speedRxState = RX_NORMAL,
        .gearRxState = RX_NORMAL,
        .engineRxState = RX_NORMAL
    };

    VehicleMonitor monitor = {0};

    VehicleControlState control = {
        .lowIlluminanceTime = 0,
        .highIlluminanceTime = 0,
        .wiperActiveTime = 0,
        .headLightState = HEADLIGHT_OFF,
        .wiperState = WIPER_STOP,
        .doorLockState = DOOR_UNLOCKED
    };

    int isFirstCycle = 1;
    ReadResult readResult;

    while (
        (readResult = ReadVehicleStatus(
            files.statusFile,
            &input
        )) == READ_SUCCESS
    ) {
        UpdateVehicleReception(
            &input,
            &reception
        );

        MonitorVehicleState(
            &input,
            &reception,
            &monitor
        );

        if (isFirstCycle) {
            isFirstCycle = 0;
        } else {
            UpdateVehicleTimers(
                &input,
                &control
            );
        }

        UpdateVehicleControl(
            &input,
            &reception,
            &monitor,
            &control
        );

        WriteVehicleOutput(
            files.resultFile,
            input.timeMs,
            control.headLightState,
            monitor.lightSensorState,
            control.wiperState,
            monitor.rainSensorState,
            control.doorLockState,
            monitor.clusterWarning
        );

        if (ferror(files.resultFile)) {
            fprintf(stderr, "Failed to write result.csv\n");
            break;
        }
    }

    int error = readResult == READ_ERROR ||
                ferror(files.resultFile);

    if (CloseControlFiles(&files) != 0) {
        error = 1;
    }

    return error ? 1 : 0;
}
