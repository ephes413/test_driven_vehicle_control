#!/usr/bin/env python3
"""Build and test the real ControlWiper() using ctypes and CSV."""
import csv
import ctypes
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CSV_PATH = ROOT / "test/testcases/ControlWiper.csv"
LIB_PATH = ROOT / "test/libvehicle_test.so"


def build():
    cmd = [
        "gcc", "-std=c11", "-Wall", "-Wextra", "-fPIC", "-shared",
        "-DTEST_DRIVER_LIBRARY", "-I", str(ROOT / "include"),
        str(ROOT / "src/vehicle_control.c"),
        str(ROOT / "test/test_driver.c"),
        "-o", str(LIB_PATH),
    ]
    subprocess.run(cmd, check=True)


def main():
    build()
    lib = ctypes.CDLL(str(LIB_PATH))
    lib.test_parse_enum.argtypes = [ctypes.c_char_p]
    lib.test_parse_enum.restype = ctypes.c_int
    lib.ControlWiper.argtypes = [ctypes.c_int] * 6
    lib.ControlWiper.restype = ctypes.c_int

    def enum_value(text, *, required=False):
        if not text:
            if required:
                raise ValueError("expected output must not be empty")
            return 0  # Don't care: placeholder
        value = lib.test_parse_enum(text.encode("utf-8"))
        if value == -1:
            raise ValueError(f"unknown enum: {text!r}")
        return value

    def int_value(text):
        return int(text) if text else 0  # Don't care: placeholder

    passed = failed = 0
    with CSV_PATH.open(newline="", encoding="utf-8-sig") as fp:
        reader = csv.DictReader(fp)
        expected_columns = [
            "testCaseId", "vehicleRxState", "engineState", "wiperSwitch",
            "rainSensorState", "rainAmount", "speed", "ControlWiper",
        ]
        if reader.fieldnames != expected_columns:
            raise ValueError(f"Unexpected CSV columns: {reader.fieldnames}")

        for row in reader:
            case_id = row.get("testCaseId") or "(missing ID)"
            try:
                if None in row or any(v is None for v in row.values()):
                    raise ValueError("malformed CSV row")
                args = [
                    enum_value(row["vehicleRxState"]),
                    enum_value(row["engineState"]),
                    enum_value(row["wiperSwitch"]),
                    enum_value(row["rainSensorState"]),
                    int_value(row["rainAmount"]),
                    int_value(row["speed"]),
                ]
                expected = enum_value(row["ControlWiper"], required=True)
                actual = lib.ControlWiper(*args)
                if actual == expected:
                    print(f"[PASS] {case_id}")
                    passed += 1
                else:
                    print(f"[FAIL] {case_id}: expected={row['ControlWiper']} ({expected}), actual={actual}")
                    failed += 1
            except (ValueError, TypeError) as exc:
                print(f"[FAIL] {case_id}: {exc}")
                failed += 1

    print(f"\nPassed: {passed}, Failed: {failed}")
    return 1 if failed else 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, subprocess.CalledProcessError, ValueError) as exc:
        print(f"Test runner error: {exc}", file=sys.stderr)
        sys.exit(2)
