#!/usr/bin/env python3
"""Audit this repository's quoted includes and focused Unreal build inputs.

This is deliberately not a C++ preprocessor. Local quoted includes are resolved
relative to their including file; external/system headers are not traversed.
Run from any directory: the repository root is derived from this script's path.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src"
SDK = SOURCE / "SDK"
INCLUDE = re.compile(r'^\s*#\s*include\s*"([^"\n]+)"', re.MULTILINE)
COMMENTS = re.compile(r'("(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\')|//[^\n]*|/\*.*?\*/', re.DOTALL)
ADD_FILES = re.compile(r"\badd_files\s*\((.*?)\)", re.DOTALL)
QUOTED = re.compile(r'["\']([^"\']+)["\']')
OLD_SUPPORT = ("UnrealContainers.hpp", "UtfN.hpp", "PropertyFixup.hpp", "NameCollisions.inl")


def active_text(text: str) -> str:
    """Remove comments without erasing include strings or joining code tokens."""
    return COMMENTS.sub(lambda match: match.group(1) or " " + "\n" * match.group(0).count("\n"), text)


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8-sig", errors="replace")


def relative(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


def file_summary(label: str, files: set[Path]) -> None:
    print(f"{label}: {len(files)} files, {sum(path.stat().st_size for path in files)} bytes")


def build_patterns(text: str) -> list[str]:
    return [pattern for call in ADD_FILES.findall(active_text(text)) for pattern in QUOTED.findall(call)
            if pattern.startswith("src/")]


def legacy(build: str) -> int:
    all_sdk = {path.resolve() for path in SDK.rglob("*") if path.is_file()}
    seeds = {path.resolve() for path in SOURCE.glob("*.cpp")}
    errors: list[str] = []
    for pattern in build_patterns(build):
        matches = {path.resolve() for path in ROOT.glob(pattern) if path.is_file()}
        if not matches:
            errors.append(f"Build input has no files: {pattern}")
        seeds.update(matches)
    reached: set[Path] = set()
    pending = list(seeds)
    while pending:
        path = pending.pop()
        if path in reached:
            continue
        reached.add(path)
        for include in INCLUDE.findall(active_text(read(path))):
            candidate = (path.parent / include).resolve()
            if not candidate.is_relative_to(SOURCE):
                continue
            if not candidate.is_file():
                errors.append(f"Unresolved local include: {relative(path)} -> {include}")
            elif candidate not in reached:
                pending.append(candidate)
    retained = all_sdk & reached
    unused = all_sdk - reached
    file_summary("Total SDK", all_sdk)
    file_summary("Reachable SDK", retained)
    file_summary("Unused SDK", unused)
    file_summary("Other reachable local files", reached - all_sdk)
    for label, paths in (("Build seeds", seeds), ("Retained SDK paths", retained), ("Unused SDK paths", unused)):
        print(f"{label}:")
        for path in sorted(paths):
            print(f"  {relative(path)}")
    for error in sorted(set(errors)):
        print(error, file=sys.stderr)
    print(f"Unresolved local includes/build inputs: {len(set(errors))}")
    return int(bool(errors))


def minimal(build: str) -> int:
    integration = SOURCE / "unreal"
    files = {path.resolve() for path in integration.rglob("*") if path.is_file()}
    file_summary("Focused integration", files)
    errors: list[str] = []
    for path in (SDK, SOURCE / "SDK.hpp", *(SOURCE / name for name in OLD_SUPPORT)):
        if path.exists():
            errors.append(f"Legacy path still exists: {relative(path)}")
    references: list[str] = []
    for path in [ROOT / "xmake.lua", *sorted(SOURCE.rglob("*"))]:
        if not path.is_file():
            continue
        text = active_text(read(path))
        for number, line in enumerate(text.splitlines(), 1):
            if re.search(r"\b(?:SDK|UC)\s*::|(?:src/)?SDK[\\/]|\bSDK\.hpp\b", line):
                references.append(f"{relative(path)}:{number}: {line.strip()}")
            for include in INCLUDE.findall(line):
                candidate = (path.parent / include).resolve()
                if candidate in {SOURCE / name for name in OLD_SUPPORT}:
                    references.append(f"{relative(path)}:{number}: {line.strip()}")
    references = sorted(set(references))
    print(f"Legacy references: {len(references)}")
    errors.extend(f"Legacy reference: {reference}" for reference in references)
    # The production target must list nested sources explicitly: src/*.cpp
    # does not include them. A reference only in the test target is insufficient.
    production = re.search(r"\btarget\s*\(\s*name\s*\)(.*?)(?=\btarget\s*\(|\Z)", active_text(build), re.DOTALL)
    production_inputs = build_patterns(production.group(1)) if production else []
    for name in ("Integration.cpp", "detail/Runtime.cpp"):
        path = integration / name
        required = relative(path)
        if not path.is_file():
            errors.append(f"Required integration source missing: {required}")
        if required not in production_inputs:
            errors.append(f"Required production build input missing: {required}")
    byte_count = sum(path.stat().st_size for path in files)
    if byte_count >= 262144:
        errors.append(f"Integration exceeds source budget: {byte_count} bytes (must be below 262144)")
    print("Integration paths:")
    for path in sorted(files):
        print(f"  {relative(path)}")
    for error in errors:
        print(error, file=sys.stderr)
    print(f"Minimal inventory: {'FAIL' if errors else 'PASS'}")
    return int(bool(errors))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--phase", choices=("legacy", "minimal"), required=True)
    arguments = parser.parse_args()
    build = read(ROOT / "xmake.lua")
    return legacy(build) if arguments.phase == "legacy" else minimal(build)


if __name__ == "__main__":
    sys.exit(main())
