
#!/usr/bin/env python3
"""Compare actual vehicle status with expected status."""

import csv
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

ACTUAL_FILE = ROOT / "simulation" / "status.csv"
EXPECTED_FILE = ROOT / "test" / "expected_status.csv"


def read_csv(path):
    with path.open("r", newline="", encoding="utf-8-sig") as file:
        reader = csv.DictReader(file)

        if reader.fieldnames is None:
            raise ValueError(f"Empty CSV file: {path}")

        if len(reader.fieldnames) != len(set(reader.fieldnames)):
            raise ValueError(f"Duplicate CSV columns: {path}")

        rows = {}

        for row in reader:
            if None in row or any(value is None for value in row.values()):
                raise ValueError(f"Invalid CSV row: {row}")

            time_ms = row["timeMs"]

            if time_ms in rows:
                raise ValueError(f"Duplicate timeMs={time_ms}: {path}")

            rows[time_ms] = row

        return reader.fieldnames, rows


def main():
    try:
        expected_columns, expected_rows = read_csv(EXPECTED_FILE)
        actual_columns, actual_rows = read_csv(ACTUAL_FILE)

        if set(expected_columns) != set(actual_columns):
            print("[FAIL] CSV column mismatch")
            print(f"Expected: {expected_columns}")
            print(f"Actual:   {actual_columns}")
            return 1

        print("\n=== Driving Integration Test ===")

        passed = 0
        failed = 0

        all_times = sorted(
            set(expected_rows) | set(actual_rows),
            key=int
        )

        for time_ms in all_times:
            expected = expected_rows.get(time_ms)
            actual = actual_rows.get(time_ms)

            if expected is None:
                print(f"[FAIL] timeMs={time_ms}: unexpected row")
                failed += 1
                continue

            if actual is None:
                print(f"[FAIL] timeMs={time_ms}: missing row")
                failed += 1
                continue

            differences = [
                column
                for column in expected_columns
                if expected[column] != actual[column]
            ]

            if differences:
                print(f"[FAIL] timeMs={time_ms}")

                for column in differences:
                    print(
                        f"  {column}: "
                        f"expected={expected[column]}, "
                        f"actual={actual[column]}"
                    )

                failed += 1
            else:
                print(f"[PASS] timeMs={time_ms}")
                passed += 1

        print(f"\nPassed: {passed}")
        print(f"Failed: {failed}")

        return 1 if failed else 0

    except (OSError, ValueError, KeyError) as error:
        print(f"[ERROR] {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
