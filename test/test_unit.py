
#!/usr/bin/env python3
"""Run CSV test cases against compiled C functions discovered from headers."""

import argparse
import csv
import ctypes
import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
INCLUDE = ROOT / 'include'
SRC = ROOT / 'src'
CASES = ROOT / 'test' / 'testcases'

IDENT = r'[A-Za-z_][A-Za-z_0-9]*'

PROTOTYPE = re.compile(
    rf'\b(?P<ret>{IDENT}(?:\s+{IDENT})*)\s+'
    rf'(?P<name>{IDENT})\s*'
    rf'\((?P<args>[^()]*)\)\s*;',
    re.S
)

PARAM = re.compile(
    rf'^\s*(?P<type>{IDENT}(?:\s+{IDENT})*)\s+'
    rf'(?P<name>{IDENT})\s*$'
)

PRIMITIVES = {
    'int': ctypes.c_int,
    'signed int': ctypes.c_int,
    'unsigned': ctypes.c_uint,
    'unsigned int': ctypes.c_uint,
}


def run(cmd):
    subprocess.run(cmd, check=True)


def headers():
    result = sorted(INCLUDE.glob('*.h'))

    if not result:
        raise ValueError('No header files in include/')

    return result


def clean_header(path):
    source = path.read_text(encoding='utf-8')

    source = re.sub(r'/\*.*?\*/', ' ', source, flags=re.S)
    source = re.sub(r'//[^\n]*', '', source)
    source = re.sub(r'^\s*#.*$', '', source, flags=re.M)

    return source


def discover():
    found = {}

    for header in headers():
        source = clean_header(header)

        # Only declarations at the top level; ignore enum/struct bodies.
        depth = 0
        top = []

        for char in source:
            if char == '{':
                depth += 1
            elif char == '}':
                depth -= 1

                if depth < 0:
                    raise ValueError(f'Unbalanced braces: {header}')

            elif depth == 0:
                top.append(char)

        if depth:
            raise ValueError(f'Unbalanced braces: {header}')

        for match in PROTOTYPE.finditer(''.join(top)):
            name = match['name']

            # Discover only functions with corresponding CSV test cases.
            if not (CASES / f'{name}.csv').is_file():
                continue

            ret = ' '.join(match['ret'].split())
            args = match['args'].strip()
            params = []

            if args and args != 'void':
                for declaration in args.split(','):
                    parsed = PARAM.fullmatch(declaration.strip())

                    if not parsed:
                        raise ValueError(
                            f'Unsupported parameter in {name}: '
                            f'{declaration!r}'
                        )

                    params.append((
                        parsed['name'],
                        ' '.join(parsed['type'].split())
                    ))

            if name in found:
                raise ValueError(f'Duplicate declaration: {name}')

            found[name] = (ret, params)

    return found


def ctype(type_name):
    # Project-specific typedef enums are represented as int in this ABI.
    if type_name in PRIMITIVES:
        return PRIMITIVES[type_name]

    if re.fullmatch(IDENT, type_name) and type_name[0].isupper():
        return ctypes.c_int

    raise ValueError(f'Unsupported C type: {type_name}')


def is_enum(type_name):
    return type_name not in PRIMITIVES


def read_cases(name, params):
    path = CASES / f'{name}.csv'

    if not path.is_file():
        raise ValueError(f'Missing test cases: {path}')

    with path.open(newline='', encoding='utf-8-sig') as file:
        reader = csv.DictReader(file)

        expected = [
            'testCaseId',
            *[param for param, _ in params],
            name
        ]

        if reader.fieldnames != expected:
            raise ValueError(
                f'{name}: CSV columns mismatch\n'
                f'Expected: {expected}\n'
                f'Actual: {reader.fieldnames}'
            )

        rows = list(reader)

    if not rows:
        raise ValueError(f'{name}: empty CSV')

    return rows


def compile_constant_probe(selected, definitions, cases, workdir):
    """Resolve C enum constants and integer macros using the C compiler."""

    constants = set()

    for name in selected:
        ret, params = definitions[name]

        for row in cases[name]:
            if None in row or any(
                value is None for value in row.values()
            ):
                raise ValueError(f'{name}: malformed CSV row')

            for col, typ in [*params, (name, ret)]:
                token = row[col].strip()

                if not token:
                    continue

                # Integer literals do not require compiler resolution.
                if not is_enum(typ):
                    try:
                        int(token, 10)
                        continue
                    except ValueError:
                        pass

                # Resolve symbolic constants using the C compiler.
                if not re.fullmatch(
                    IDENT, token, flags=re.ASCII
                ):
                    raise ValueError(
                        f'Invalid C constant: {token!r}'
                    )

                constants.add(token)

    if not constants:
        return {}

    probe = workdir / 'constant_probe.c'
    exe = workdir / 'constant_probe'

    lines = [
        '#include <stdio.h>',
        '#include <limits.h>'
    ]

    lines += [
        f'#include "{header.name}"'
        for header in headers()
    ]

    lines += ['int main(void) {']

    for token in sorted(constants):
        lines.append(
            f'printf("{token}=%lld\\n", '
            f'(long long)({token}));'
        )

    lines += [
        'return 0;',
        '}'
    ]

    probe.write_text(
        '\n'.join(lines) + '\n',
        encoding='utf-8'
    )

    run([
        'gcc',
        '-std=c11',
        '-I', str(INCLUDE),
        str(probe),
        '-o', str(exe)
    ])

    output = subprocess.check_output(
        [str(exe)],
        text=True
    )

    return {
        key: int(value)
        for key, value in (
            line.split('=', 1)
            for line in output.splitlines()
        )
    }


def convert(token, typ, constants):
    """Convert a CSV token into the integer expected by ctypes."""

    token = token.strip()

    if not token:
        # Don't care: use a zero placeholder.
        return 0

    if token in constants:
        value = constants[token]

    elif is_enum(typ):
        raise ValueError(
            f'Unknown C constant: {token}'
        )

    else:
        try:
            value = int(token, 10)
        except ValueError as exc:
            raise ValueError(
                f'Invalid integer or C constant: {token}'
            ) from exc

    if typ in ('unsigned', 'unsigned int'):
        if not 0 <= value <= 0xffffffff:
            raise ValueError(
                f'Out-of-range unsigned int: {value}'
            )

    elif not -(2**31) <= value < 2**31:
        raise ValueError(
            f'Out-of-range int: {value}'
        )

    return value


def test_one(lib, name, definition, rows, constants):
    ret, params = definition

    func = getattr(lib, name)
    func.argtypes = [
        ctype(typ)
        for _, typ in params
    ]
    func.restype = ctype(ret)

    passed = 0
    failed = 0

    print(f'\n=== {name} ===')

    for row in rows:
        case_id = row.get('testCaseId') or '(missing ID)'

        try:
            args = [
                convert(row[col], typ, constants)
                for col, typ in params
            ]

            if not row[name].strip():
                raise ValueError('Missing expected result')

            expected = convert(
                row[name],
                ret,
                constants
            )

            actual = func(*args)

            if actual == expected:
                passed += 1
                print(f'[PASS] {case_id}')

            else:
                failed += 1
                print(
                    f'[FAIL] {case_id}: '
                    f'expected={row[name]} ({expected}), '
                    f'actual={actual}'
                )

        except (ValueError, KeyError, TypeError) as exc:
            failed += 1
            print(f'[FAIL] {case_id}: {exc}')

    print(
        f'{name}: Passed={passed}, Failed={failed}'
    )

    return passed, failed


def main():
    parser = argparse.ArgumentParser(
        description=(
            'CSV tests for C functions discovered '
            'from include/*.h'
        )
    )

    parser.add_argument(
        'function',
        nargs='?',
        default='all',
        help='Function name or all (default: all)'
    )

    args = parser.parse_args()

    definitions = discover()

    if not definitions:
        raise ValueError(
            'No testable functions with matching CSV files found'
        )

    if (
        args.function != 'all'
        and args.function not in definitions
    ):
        parser.error(
            f'Unknown function {args.function!r}; '
            f'available: {", ".join(sorted(definitions))}'
        )

    selected = (
        sorted(definitions)
        if args.function == 'all'
        else [args.function]
    )

    cases = {
        name: read_cases(name, definitions[name][1])
        for name in selected
    }

    # Validate return and parameter types.
    for name in selected:
        ret, params = definitions[name]

        ctype(ret)

        for _, typ in params:
            ctype(typ)

    sources = sorted(SRC.glob('*.c'))

    if not sources:
        raise ValueError('No C sources found in src/')

    with tempfile.TemporaryDirectory(
        prefix='vehicle_test_'
    ) as temp:

        workdir = Path(temp)
        lib_path = workdir / 'libvehicle.so'

        # Compile project sources into a shared library.
        run([
            'gcc',
            '-std=c11',
            '-Wall',
            '-Wextra',
            '-fPIC',
            '-shared',
            '-I', str(INCLUDE),
            *map(str, sources),
            '-o', str(lib_path)
        ])

        # Resolve enum values and integer macros.
        constants = compile_constant_probe(
            selected,
            definitions,
            cases,
            workdir
        )

        lib = ctypes.CDLL(str(lib_path))

        total_passed = 0
        total_failed = 0

        for name in selected:
            passed, failed = test_one(
                lib,
                name,
                definitions[name],
                cases[name],
                constants
            )

            total_passed += passed
            total_failed += failed

    print(
        '\n=== Total Results ===\n'
        f'Passed: {total_passed}, '
        f'Failed: {total_failed}'
    )

    return 1 if total_failed else 0


if __name__ == '__main__':
    try:
        sys.exit(main())

    except (
        OSError,
        ValueError,
        subprocess.CalledProcessError,
        AttributeError
    ) as exc:
        print(
            f'Test runner error: {exc}',
            file=sys.stderr
        )
        sys.exit(2)
