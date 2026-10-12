
#!/usr/bin/env python3
"""Run integration tests against the complete vehicle simulation."""

import csv
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
INCLUDE = ROOT / "include"
SRC = ROOT / "src"
SIMULATION = ROOT / "simulation"
EXPECTED = ROOT / "test" / "expected_output.csv"


def build(executable):
    sources = sorted(SRC.glob("*.c"))

    if not sources:
        raise RuntimeError("No C source files found")

    command = [
        "gcc",
        "-std=c11",
        "-Wall",
        "-Wextra",
        "-Werror",
        "-I", str(INCLUDE),
        *map(str, sources),
        "-o", str(executable),
    ]

    subprocess.run(command, check=True)


def run(executable):
    subprocess.run(
        [str(executable)],
        cwd=ROOT,
        check=True,
    )


def read_csv(path):
    with path.open("r", newline="", encoding="utf-8-sig") as file:
        reader = csv.DictReader(file)

        if not reader.fieldnames:
            raise ValueError(f"Missing CSV header: {path}")

        if len(reader.fieldnames) != len(set(reader.fieldnames)):
            raise ValueError(f"Duplicate CSV columns: {path}")

        rows = list(reader)

        for line_number, row in enumerate(rows, start=2):
            if None in row or any(value is None for value in row.values()):
                raise ValueError(
                    f"Invalid CSV row at {path}:{line_number}"
                )

        return reader.fieldnames, rows


def compare(expected_path, actual_path, input_path):
    input_fields, input_rows = read_csv(input_path)
    expected_fields, expected_rows = read_csv(expected_path)
    actual_fields, actual_rows = read_csv(actual_path)

    if "timeMs" not in input_fields:
        raise ValueError("Missing timeMs in input CSV")

    if "timeMs" not in expected_fields:
        raise ValueError("Missing timeMs in expected CSV")

    if expected_fields != actual_fields:
        raise ValueError(
            "CSV columns do not match:\n"
            f"  expected: {expected_fields}\n"
            f"  actual:   {actual_fields}"
        )

    def index_by_time(rows, label):
        indexed = {}

        for row in rows:
            time_ms = row["timeMs"].strip()

            if not time_ms.isdecimal():
                raise ValueError(
                    f"Invalid timeMs in {label}: {time_ms!r}"
                )

            time_ms = int(time_ms)

            if time_ms in indexed:
                raise ValueError(
                    f"Duplicate timeMs in {label}: {time_ms}"
                )

            indexed[time_ms] = row

        return indexed

    inputs = index_by_time(input_rows, "input")
    expected = index_by_time(expected_rows, "expected")
    actual = index_by_time(actual_rows, "actual")

    passed = 0
    failed = 0

    print("\n=== Integration Test ===")

    if len(expected_rows) != len(input_rows):
        print(
            "[ERROR] Input and expected row counts differ: "
            f"input={len(input_rows)}, expected={len(expected_rows)}"
        )
        failed += 1

    for time_ms in inputs:
        if time_ms not in expected:
            print(f"[ERROR] Missing expected row: timeMs={time_ms}")
            failed += 1

    for time_ms in expected:
        if time_ms not in inputs:
            print(f"[ERROR] Unexpected expected row: timeMs={time_ms}")
            failed += 1

    for time_ms, expected_row in expected.items():
        if time_ms not in actual:
            print(f"[FAIL] timeMs={time_ms}: missing output")
            failed += 1
            continue

        actual_row = actual[time_ms]

        differences = []

        for field in expected_fields:
            expected_value = expected_row[field].strip()
            actual_value = actual_row[field].strip()

            if expected_value != actual_value:
                differences.append(
                    (field, expected_value, actual_value)
                )

        if differences:
            print(f"[FAIL] timeMs={time_ms}")

            for field, expected_value, actual_value in differences:
                print(f"  {field}:")
                print(f"    expected = {expected_value}")
                print(f"    actual   = {actual_value}")

            failed += 1
        else:
            print(f"[PASS] timeMs={time_ms}")
            passed += 1

    for time_ms in actual:
        if time_ms not in expected:
            print(f"[FAIL] Unexpected output: timeMs={time_ms}")
            failed += 1

    print(f"\nPassed: {passed}")
    print(f"Failed: {failed}")

    return failed == 0


def main():
    input_path = SIMULATION / "status.csv"
    output_path = SIMULATION / "result.csv"

    for path in (input_path, EXPECTED):
        if not path.is_file():
            print(f"[ERROR] File not found: {path}", file=sys.stderr)
            return 1

    try:
        # Validate test data before running the simulation.
        read_csv(input_path)
        read_csv(EXPECTED)

        with tempfile.TemporaryDirectory() as directory:
            executable = Path(directory) / "vehicle_simulation"

            build(executable)

            # Prevent comparison with stale output.
            output_path.unlink(missing_ok=True)

            run(executable)

        if not output_path.is_file():
            raise RuntimeError("Simulation did not create result.csv")

        return 0 if compare(
            EXPECTED,
            output_path,
            input_path,
        ) else 1

    except (OSError, ValueError, RuntimeError,
            subprocess.CalledProcessError) as error:
        print(f"[ERROR] {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
