#!/usr/bin/env python3
"""CSV-driven tests of the actual C ControlWiper function; no C test driver needed."""
import csv
import ctypes
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
HEADER_DIR = ROOT / 'include'
SOURCE = ROOT / 'src' / 'vehicle_control.c'
CSV_PATH = ROOT / 'test' / 'testcases' / 'ControlWiper.csv'
COLUMNS = ['testCaseId', 'vehicleRxState', 'engineState', 'wiperSwitch',
           'rainSensorState', 'rainAmount', 'speed', 'ControlWiper']
ENUM_COLUMNS = ['vehicleRxState', 'engineState', 'wiperSwitch',
                'rainSensorState', 'ControlWiper']


def run_gcc(command):
    subprocess.run(command, check=True)


def read_cases():
    with CSV_PATH.open(newline='', encoding='utf-8-sig') as stream:
        reader = csv.DictReader(stream)
        if reader.fieldnames != COLUMNS:
            raise ValueError(f'Unexpected CSV columns: {reader.fieldnames}')
        rows = list(reader)
    if not rows:
        raise ValueError('No test cases found')
    return rows


def enum_values(rows, workdir):
    """Ask the C compiler for actual enum constants; never assume enum numbering."""
    names = sorted({row[col].strip() for row in rows for col in ENUM_COLUMNS
                    if row.get(col)})
    probe = workdir / 'enum_probe.c'
    executable = workdir / 'enum_probe'
    lines = ['#include <stdio.h>', '#include "vehicle_control.h"',
             'int main(void) {']
    for name in names:
        # Names are generated only from identifier-shaped CSV tokens.
        if not name.isascii() or not name.isidentifier():
            raise ValueError(f'Invalid C enum identifier: {name!r}')
        lines.append(f'    printf("{name}=%d\\n", (int)({name}));')
    lines += ['    return 0;', '}']
    probe.write_text('\n'.join(lines) + '\n', encoding='utf-8')
    run_gcc(['gcc', '-std=c11', '-I', str(HEADER_DIR), str(probe), '-o', str(executable)])
    output = subprocess.check_output([str(executable)], text=True)
    return {name: int(value) for name, value in
            (line.split('=', 1) for line in output.splitlines())}


def main():
    rows = read_cases()
    with tempfile.TemporaryDirectory(prefix='vehicle_test_') as tmp:
        workdir = Path(tmp)
        library = workdir / 'libvehicle_control.so'
        run_gcc(['gcc', '-std=c11', '-Wall', '-Wextra', '-fPIC', '-shared',
                 '-I', str(HEADER_DIR), str(SOURCE), '-o', str(library)])
        values = enum_values(rows, workdir)
        lib = ctypes.CDLL(str(library))
        function = lib.ControlWiper
        function.argtypes = [ctypes.c_int] * 6
        function.restype = ctypes.c_int

        passed = failed = 0
        for row in rows:
            case_id = row.get('testCaseId') or '(missing ID)'
            try:
                if None in row or any(value is None for value in row.values()):
                    raise ValueError('Malformed CSV row')

                def enum(col):
                    token = row[col].strip()
                    return values[token] if token else 0  # Don't care placeholder

                def number(col):
                    token = row[col].strip()
                    return int(token) if token else 0

                if not row['ControlWiper'].strip():
                    raise ValueError('Missing expected result')
                expected = enum('ControlWiper')
                actual = function(enum('vehicleRxState'), enum('engineState'),
                                  enum('wiperSwitch'), enum('rainSensorState'),
                                  number('rainAmount'), number('speed'))
                if actual == expected:
                    passed += 1
                    print(f'[PASS] {case_id}')
                else:
                    failed += 1
                    print(f'[FAIL] {case_id}: expected={row["ControlWiper"]} '
                          f'({expected}), actual={actual}')
            except (ValueError, KeyError) as error:
                failed += 1
                print(f'[FAIL] {case_id}: {error}')
        print(f'\nPassed: {passed}, Failed: {failed}')
        return 1 if failed else 0


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (OSError, subprocess.CalledProcessError, ValueError) as error:
        print(f'Test runner error: {error}', file=sys.stderr)
        sys.exit(2)
