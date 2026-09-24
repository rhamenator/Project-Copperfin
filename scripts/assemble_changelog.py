#!/usr/bin/env python3
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

"""Validate and assemble changelog fragments (#6554).

Each pull request adds one fragment file under ``changelog.d/`` instead of
editing the top of ``CHANGELOG.md``, so concurrent pull requests never
conflict. This script folds the fragments into ``CHANGELOG.md`` (newest
first) and deletes them, or, with ``--check``, only validates them.

Fragment rules:
- name: ``YYYY-MM-DD-<issue-or-slug>.md`` (lowercase letters, digits, dashes);
- content: exactly one ``CHANGELOG.md`` bullet -- a first line starting with
  ``- YYYY-MM-DD:`` matching the file-name date, continuation lines indented
  by two spaces, no blank lines.
"""

from __future__ import annotations

import argparse
import re
import sys
import tempfile
from pathlib import Path


FRAGMENT_NAME = re.compile(r"^(\d{4}-\d{2}-\d{2})-[a-z0-9][a-z0-9-]*\.md$")
FRAGMENT_DIR = "changelog.d"
IGNORED = {"README.md"}


class FragmentError(RuntimeError):
    pass


def fragment_paths(root: Path) -> list[Path]:
    directory = root / FRAGMENT_DIR
    if not directory.is_dir():
        return []
    return sorted(
        (path for path in directory.iterdir() if path.is_file() and path.name not in IGNORED),
        key=lambda path: path.name,
        reverse=True,
    )


def read_fragment(path: Path) -> str:
    match = FRAGMENT_NAME.match(path.name)
    if match is None:
        raise FragmentError(f"{path.name}: name must be YYYY-MM-DD-<issue-or-slug>.md (lowercase, digits, dashes)")
    date = match.group(1)
    text = path.read_text(encoding="utf-8").replace("\r\n", "\n").rstrip("\n")
    lines = text.split("\n")
    if not lines or not lines[0].startswith(f"- {date}:"):
        raise FragmentError(f"{path.name}: first line must start with '- {date}:'")
    for number, line in enumerate(lines[1:], start=2):
        if not line.strip():
            raise FragmentError(f"{path.name}: line {number} is blank; a fragment is a single bullet")
        if not line.startswith("  "):
            raise FragmentError(f"{path.name}: line {number} must be indented by two spaces")
    return text + "\n"


def check(root: Path) -> list[Path]:
    paths = fragment_paths(root)
    for path in paths:
        read_fragment(path)
    return paths


def assemble(root: Path) -> int:
    paths = check(root)
    if not paths:
        return 0
    changelog = root / "CHANGELOG.md"
    existing = changelog.read_text(encoding="utf-8") if changelog.exists() else ""
    changelog.write_text("".join(read_fragment(path) for path in paths) + existing, encoding="utf-8")
    for path in paths:
        path.unlink()
    return len(paths)


def self_test() -> None:
    with tempfile.TemporaryDirectory() as temp:
        root = Path(temp)
        (root / "CHANGELOG.md").write_text("- 2026-01-01: Old entry.\n", encoding="utf-8")
        fragments = root / FRAGMENT_DIR
        fragments.mkdir()
        (fragments / "README.md").write_text("ignored\n", encoding="utf-8")
        (fragments / "2026-09-24-6551-append-for.md").write_text(
            "- 2026-09-24: Newer entry\n  continued.\n", encoding="utf-8")
        (fragments / "2026-09-23-6500-other.md").write_text("- 2026-09-23: Older entry.\n", encoding="utf-8")

        if len(check(root)) != 2:
            raise AssertionError("check should find two fragments and ignore README.md")
        if assemble(root) != 2:
            raise AssertionError("assemble should fold two fragments")
        expected = "- 2026-09-24: Newer entry\n  continued.\n- 2026-09-23: Older entry.\n- 2026-01-01: Old entry.\n"
        actual = (root / "CHANGELOG.md").read_text(encoding="utf-8")
        if actual != expected:
            raise AssertionError(f"unexpected CHANGELOG.md after assembly:\n{actual}")
        if [path.name for path in fragments.iterdir()] != ["README.md"]:
            raise AssertionError("assembled fragments should be deleted, README.md kept")
        if assemble(root) != 0:
            raise AssertionError("assembling with no fragments should be a no-op")

        bad_cases = {
            "notes.md": "- 2026-09-24: x\n",
            "2026-09-24-Bad_Name.md": "- 2026-09-24: x\n",
            "2026-09-24-wrong-date.md": "- 2026-09-25: x\n",
            "2026-09-24-blank-line.md": "- 2026-09-24: x\n\n  y\n",
            "2026-09-24-unindented.md": "- 2026-09-24: x\ny\n",
        }
        for name, content in bad_cases.items():
            path = fragments / name
            path.write_text(content, encoding="utf-8")
            try:
                check(root)
            except FragmentError:
                pass
            else:
                raise AssertionError(f"{name} should be rejected")
            path.unlink()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parent.parent)
    parser.add_argument("--check", action="store_true", help="validate fragments without writing")
    parser.add_argument("--self-test", action="store_true", help="run the built-in self test")
    args = parser.parse_args()
    try:
        if args.self_test:
            self_test()
            print("assemble_changelog self-test passed")
            return 0
        if args.check:
            paths = check(args.root)
            print(f"{len(paths)} changelog fragment(s) valid")
            return 0
        count = assemble(args.root)
        print(f"assembled {count} changelog fragment(s) into CHANGELOG.md")
        return 0
    except FragmentError as error:
        print(f"error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
