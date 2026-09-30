#!/usr/bin/env python3
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

"""Verify DCO sign-offs for every commit and recorded co-author in a range."""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
import tempfile
from pathlib import Path


FULL_SHA = re.compile(r"^[0-9a-f]{40}$")
IDENTITY = re.compile(r"^(.+?)\s+<([^<>\s]+@[^<>\s]+)>$")
ATTESTATION_PATH = ".github/dco-historical-attestations.json"


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


def git_succeeds(*args: str, cwd: Path | None = None) -> bool:
    result = subprocess.run(
        ["git", *args],
        cwd=cwd,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        check=False,
    )
    return result.returncode == 0


class Attestations:
    """Owner attestation for specific historical commits that predate the gate.

    Only exact, immutable commit IDs qualify. Each must also be an ancestor of a
    recorded boundary commit and be authored by a recorded owner identity, so an
    attestation can never cover new work or another contributor's commits.
    """

    def __init__(self, commits: set[str], authors: set[str], boundaries: list[str]) -> None:
        self.commits = commits
        self.authors = authors
        self.boundaries = boundaries


def load_attestations(base: str, cwd: Path | None = None) -> Attestations | None:
    """Read the manifest from the trusted base revision, never from the range under test.

    A pull request must not be able to define its own exemptions, so the policy in
    force is the one already merged into the base branch. A change to the manifest
    takes effect only after it has been reviewed and merged.
    """
    if not git_succeeds("cat-file", "-e", f"{base}:{ATTESTATION_PATH}", cwd=cwd):
        return None
    try:
        data = json.loads(run_git("show", f"{base}:{ATTESTATION_PATH}", cwd=cwd))
    except json.JSONDecodeError as error:
        raise SignoffError(f"{ATTESTATION_PATH} is not valid JSON: {error}") from error
    if not isinstance(data, dict) or data.get("version") != 1:
        raise SignoffError(f"{ATTESTATION_PATH} must be a version 1 object")
    commits = data.get("commits")
    boundaries = data.get("boundaries")
    authors = data.get("attested_authors")
    for label, values in (("commits", commits), ("boundaries", boundaries)):
        if not isinstance(values, list) or not values:
            raise SignoffError(f"{ATTESTATION_PATH} {label} must be a non-empty list")
        for value in values:
            if not isinstance(value, str) or not FULL_SHA.fullmatch(value):
                raise SignoffError(f"{ATTESTATION_PATH} {label} entries must be lowercase full commit IDs")
    if len(set(commits)) != len(commits):
        raise SignoffError(f"{ATTESTATION_PATH} lists a commit more than once")
    if not isinstance(authors, list) or not authors:
        raise SignoffError(f"{ATTESTATION_PATH} attested_authors must be a non-empty list")
    for boundary in boundaries:
        run_git("cat-file", "-e", f"{boundary}^{{commit}}", cwd=cwd)
    return Attestations(
        set(commits), {normalize_identity(author) for author in authors}, list(boundaries)
    )


def is_attested(commit: str, attestations: Attestations | None, author: str, cwd: Path | None = None) -> bool:
    if attestations is None or commit not in attestations.commits:
        return False
    if author not in attestations.authors:
        return False
    return any(
        git_succeeds("merge-base", "--is-ancestor", commit, boundary, cwd=cwd)
        for boundary in attestations.boundaries
    )


def check_commit(commit: str, cwd: Path | None = None, attestations: Attestations | None = None) -> None:
    author_fields = run_git("show", "-s", "--format=%an%x00%ae", commit, cwd=cwd).rstrip("\n").split("\0")
    if len(author_fields) != 2:
        raise SignoffError(f"cannot resolve author identity for {commit}")
    author = normalize_identity(f"{author_fields[0]} <{author_fields[1]}>")
    if is_attested(commit, attestations, author, cwd=cwd):
        return
    required = {author}

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
    attestations = load_attestations(base, cwd=cwd)
    for commit in commits:
        check_commit(commit, cwd=cwd, attestations=attestations)
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
            pass
        else:
            raise SignoffError("unsigned self-test commit was accepted")

        def commit_signed(message: str) -> str:
            run_git(
                "commit",
                "--allow-empty",
                "-m",
                f"{message}\n\nSigned-off-by: Contributor <contributor@example.invalid>",
                cwd=root,
            )
            return run_git("rev-parse", "HEAD", cwd=root).strip()

        def write_manifest(commits: list[str], authors: list[str], boundaries: list[str]) -> None:
            manifest = root / ATTESTATION_PATH
            manifest.parent.mkdir(parents=True, exist_ok=True)
            manifest.write_text(
                json.dumps(
                    {
                        "version": 1,
                        "attested_authors": authors,
                        "boundaries": boundaries,
                        "commits": commits,
                    }
                ),
                encoding="utf-8",
            )
            run_git("add", ATTESTATION_PATH, cwd=root)

        def expect(base_rev: str, head_rev: str, ok: bool, message: str) -> None:
            try:
                check_range(base_rev, head_rev, cwd=root)
                accepted = True
            except SignoffError:
                accepted = False
            if accepted != ok:
                raise SignoffError(message)

        owner = ["Contributor <contributor@example.invalid>"]

        # Trusted policy: the manifest is already merged in the base revision.
        run_git("checkout", "--quiet", "-b", "policy", good, cwd=root)
        write_manifest([bad], owner, [bad])
        policy = commit_signed("policy attests bad")
        run_git("checkout", "--quiet", "-b", "joined", policy, cwd=root)
        run_git("merge", "--no-ff", "--no-edit", "-m", "join\n\nSigned-off-by: Contributor <contributor@example.invalid>", bad, cwd=root)
        joined = run_git("rev-parse", "HEAD", cwd=root).strip()
        expect(policy, joined, True, "attested historical commit in base policy was not accepted")

        # New work after the boundary is never covered by the base policy.
        run_git("commit", "--allow-empty", "-m", "new unsigned work", cwd=root)
        new_unsigned = run_git("rev-parse", "HEAD", cwd=root).strip()
        expect(policy, new_unsigned, False, "unsigned commit newer than the attestation boundary was accepted")

        # A range cannot define its own exemption: the manifest edit is in the range
        # under test, not in the base, so the unsigned commit is still rejected.
        run_git("checkout", "--quiet", "-b", "self-attest", bad, cwd=root)
        write_manifest([bad], owner, [bad])
        self_attest = commit_signed("range edits the manifest to exempt bad")
        expect(good, self_attest, False, "a range that edits the manifest exempted its own unsigned commit")

        # The base policy must still match author and boundary.
        for label, authors, boundaries in (
            ("attestation outside every boundary", owner, [good]),
            ("attestation for an unrecorded author", ["Someone Else <else@example.invalid>"], [bad]),
        ):
            run_git("checkout", "--quiet", "-B", "variant", good, cwd=root)
            write_manifest([bad], authors, boundaries)
            variant_policy = commit_signed(f"policy: {label}")
            run_git("merge", "--no-ff", "--no-edit", "-m", "join\n\nSigned-off-by: Contributor <contributor@example.invalid>", bad, cwd=root)
            expect(variant_policy, run_git("rev-parse", "HEAD", cwd=root).strip(), False, f"{label} was accepted")


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
