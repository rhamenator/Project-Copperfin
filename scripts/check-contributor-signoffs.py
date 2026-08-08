#!/usr/bin/env python3
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

"""Verify DCO sign-offs for every commit and recorded co-author in a range."""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
import tempfile
from pathlib import Path


FULL_SHA = re.compile(r"^[0-9a-f]{40}$")
IDENTITY = re.compile(r"^(.+?)\s+<([^<>\s]+@[^<>\s]+)>$")


class SignoffError(RuntimeError):
    pass


def run_git(*args: str, cwd: Path | None = None, input_text: str | None = None) -> str:
    result = subprocess.run(
        ["git", *args],
        cwd=cwd,
        input=input_text,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if result.returncode != 0:
        raise SignoffError(result.stderr.strip() or f"git {' '.join(args)} failed")
    return result.stdout


def normalize_identity(value: str) -> str:
    match = IDENTITY.fullmatch(value.strip())
    if not match:
        raise SignoffError(f"invalid contributor identity: {value!r}")
    name = " ".join(match.group(1).split())
    email = match.group(2).casefold()
    return f"{name} <{email}>"


def parsed_trailers(message: str, cwd: Path | None = None) -> list[tuple[str, str]]:
    parsed = run_git("interpret-trailers", "--parse", cwd=cwd, input_text=message)
    trailers: list[tuple[str, str]] = []
    for line in parsed.splitlines():
        key, separator, value = line.partition(":")
        if separator:
            trailers.append((key.strip().casefold(), value.strip()))
    return trailers


def check_commit(commit: str, cwd: Path | None = None) -> None:
    author_fields = run_git("show", "-s", "--format=%an%x00%ae", commit, cwd=cwd).rstrip("\n").split("\0")
    if len(author_fields) != 2:
        raise SignoffError(f"cannot resolve author identity for {commit}")
    required = {normalize_identity(f"{author_fields[0]} <{author_fields[1]}>")}

    message = run_git("show", "-s", "--format=%B", commit, cwd=cwd)
    trailers = parsed_trailers(message, cwd=cwd)
    for key, value in trailers:
        if key == "co-authored-by":
            required.add(normalize_identity(value))

    signed = {
        normalize_identity(value)
        for key, value in trailers
        if key == "signed-off-by"
    }
    missing = sorted(required - signed)
    if missing:
        raise SignoffError(f"{commit} lacks Signed-off-by for: {', '.join(missing)}")


def check_range(base: str, head: str, cwd: Path | None = None) -> int:
    for revision, label in ((base, "base"), (head, "head")):
        if not FULL_SHA.fullmatch(revision):
            raise SignoffError(f"{label} revision must be a lowercase full Git object ID")
        run_git("cat-file", "-e", f"{revision}^{{commit}}", cwd=cwd)

    commits = [line for line in run_git("rev-list", "--reverse", f"{base}..{head}", cwd=cwd).splitlines() if line]
    if not commits:
        raise SignoffError("contribution range contains no commits")
    for commit in commits:
        check_commit(commit, cwd=cwd)
    return len(commits)


def self_test() -> None:
    with tempfile.TemporaryDirectory(prefix="copperfin-signoff-") as directory:
        root = Path(directory)
        run_git("init", "--quiet", cwd=root)
        run_git("config", "user.name", "Contributor", cwd=root)
        run_git("config", "user.email", "contributor@example.invalid", cwd=root)
        run_git("commit", "--allow-empty", "-m", "base\n\nSigned-off-by: Contributor <contributor@example.invalid>", cwd=root)
        base = run_git("rev-parse", "HEAD", cwd=root).strip()

        run_git(
            "commit",
            "--allow-empty",
            "-m",
            "good\n\nCo-authored-by: Reviewer <reviewer@example.invalid>\nSigned-off-by: Contributor <contributor@example.invalid>\nSigned-off-by: Reviewer <reviewer@example.invalid>",
            cwd=root,
        )
        good = run_git("rev-parse", "HEAD", cwd=root).strip()
        if check_range(base, good, cwd=root) != 1:
            raise SignoffError("valid self-test range returned the wrong commit count")

        run_git("commit", "--allow-empty", "-m", "unsigned", cwd=root)
        bad = run_git("rev-parse", "HEAD", cwd=root).strip()
        try:
            check_range(good, bad, cwd=root)
        except SignoffError:
            return
        raise SignoffError("unsigned self-test commit was accepted")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--base")
    parser.add_argument("--head")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    try:
        if args.self_test:
            self_test()
            print("Contributor sign-off self-test passed")
            return 0
        if not args.base or not args.head:
            parser.error("--base and --head are required unless --self-test is used")
        count = check_range(args.base, args.head)
        print(f"Contributor sign-off contract passed for {count} commit(s)")
        return 0
    except SignoffError as error:
        print(f"Contributor sign-off contract failed: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
