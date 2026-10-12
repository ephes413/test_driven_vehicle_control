#!/usr/bin/env python3
"""Validate requirements-to-implementation-to-test traceability without running tests."""
import argparse
import csv
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
REQ_COLUMNS = ('requirementId', 'category', 'description', 'priority', 'verification', 'precedence')
LINK_COLUMNS = ('requirementId', 'sourceFile', 'function', 'testFile', 'testCaseId')


def read_table(path, columns, errors):
    try:
        with path.open(newline='', encoding='utf-8-sig') as stream:
            reader = csv.DictReader(stream)
            fields = reader.fieldnames or []
            if len(fields) != len(set(fields)):
                errors.append(f'{path}: duplicate column names')
            missing = set(columns) - set(fields)
            if missing:
                errors.append(f'{path}: missing columns: {", ".join(sorted(missing))}')
                return []
            rows = []
            for line, row in enumerate(reader, 2):
                if None in row or any(value is None for value in row.values()):
                    errors.append(f'{path}:{line}: malformed CSV row')
                    continue
                rows.append((line, {k: v.strip() for k, v in row.items()}))
            return rows
    except OSError as exc:
        errors.append(str(exc))
        return []


def safe_path(value, line, field, errors):
    path = Path(value)
    if not value or path.is_absolute() or '..' in path.parts:
        errors.append(f'traceability.csv:{line}: invalid {field}: {value!r}')
        return None
    resolved = (ROOT / path).resolve()
    if not resolved.is_relative_to(ROOT.resolve()):
        errors.append(f'traceability.csv:{line}: {field} escapes project root')
        return None
    return resolved


def run(strict=False):
    errors, warnings = [], []
    requirements = read_table(ROOT / 'requirements/requirements.csv', REQ_COLUMNS, errors)
    links = read_table(ROOT / 'traceability/traceability.csv', LINK_COLUMNS, errors)
    ids = set()
    for line, row in requirements:
        rid = row['requirementId']
        if not rid:
            errors.append(f'requirements.csv:{line}: empty requirementId')
        elif rid in ids:
            errors.append(f'requirements.csv:{line}: duplicate requirementId {rid}')
        ids.add(rid)
        if not row['description']:
            errors.append(f'requirements.csv:{line}: {rid}: empty description')

    seen, linked = set(), Counter()
    source_cache, case_cache = {}, {}
    for line, row in links:
        rid, source, func, test, case = (row[k] for k in LINK_COLUMNS)
        prefix = f'traceability.csv:{line}'
        if not all((rid, source, func, test, case)):
            errors.append(f'{prefix}: empty required field')
            continue
        if rid not in ids:
            errors.append(f'{prefix}: unknown requirementId {rid}')
        else:
            linked[rid] += 1
        key = (rid, source, func, test, case)
        if key in seen:
            errors.append(f'{prefix}: duplicate traceability link: {rid}/{func}/{case}')
        seen.add(key)
        src_path = safe_path(source, line, 'sourceFile', errors)
        test_path = safe_path(test, line, 'testFile', errors)
        if src_path is not None:
            if not src_path.is_file():
                errors.append(f'{prefix}: source file not found: {source}')
            else:
                if src_path not in source_cache:
                    try:
                        source_cache[src_path] = src_path.read_text(encoding='utf-8')
                    except (OSError, UnicodeError) as exc:
                        errors.append(f'{prefix}: cannot read {source}: {exc}')
                        source_cache[src_path] = ''
                # Match a function definition, not merely a call or declaration.
                pattern = rf'\b{re.escape(func)}\s*\([^;{{}}]*\)\s*\{{'
                if not re.search(pattern, source_cache[src_path], re.S):
                    errors.append(f'{prefix}: function definition {func}() not found in {source}')
        if test_path is not None:
            if test_path.name != f'{func}.csv':
                errors.append(f'{prefix}: test filename {test_path.name} does not match {func}.csv')
            if not test_path.is_file():
                errors.append(f'{prefix}: test file not found: {test}')
            else:
                if test_path not in case_cache:
                    local_errors = []
                    rows = read_table(test_path, ('testCaseId', func), local_errors)
                    errors.extend(local_errors)
                    case_ids = [r['testCaseId'] for _, r in rows]
                    for duplicated, count in Counter(case_ids).items():
                        if count > 1:
                            errors.append(f'{test}: duplicate testCaseId {duplicated}')
                    case_cache[test_path] = set(case_ids)
                if case not in case_cache[test_path]:
                    errors.append(f'{prefix}: testCaseId {case} not found in {test}')

    for rid in sorted(ids - linked.keys()):
        errors.append(f'requirement {rid}: no traceability links')
    if not requirements:
        errors.append('requirements.csv: no valid requirement rows')
    if not links:
        errors.append('traceability.csv: no valid traceability rows')

    print('=== Traceability Consistency Check ===')
    print(f'Requirements: {len(requirements)}, Links: {len(links)}, Test files checked: {len(case_cache)}')
    for warning in warnings:
        print(f'[WARN] {warning}')
    for error in errors:
        print(f'[FAIL] {error}')
    print(f'Result: {len(errors)} error(s), {len(warnings)} warning(s)')
    return 1 if errors or (strict and warnings) else 0


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--strict', action='store_true', help='Treat warnings as failures')
    args = parser.parse_args()
    return run(strict=args.strict)


if __name__ == '__main__':
    sys.exit(main())
