#!/usr/bin/env python3
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

"""Assemble and verify an immutable Copperfin RC evaluation bundle."""

from __future__ import annotations

import argparse
import hashlib
import io
import json
import re
import shutil
import sys
import tempfile
import zipfile
from pathlib import Path
from typing import BinaryIO


CANDIDATE_TAG_PATTERN = re.compile(r"v0\.1\.0-rc\.[1-9][0-9]*\Z")
RETENTION_DAYS = 90
SOURCE_PREFIX = "Project-Copperfin-source-"
SCAN_BLOCK_SIZE = 1024 * 1024
# Construct the boundaries so this scanner's own source does not contain a
# complete private-key sentinel and falsely reject the Corresponding Source
# archive that necessarily includes this file.
PRIVATE_KEY_LABELS = (
    b"PRIVATE KEY",
    b"ENCRYPTED PRIVATE KEY",
    b"RSA PRIVATE KEY",
    b"EC PRIVATE KEY",
    b"DSA PRIVATE KEY",
    b"OPENSSH PRIVATE KEY",
    b"PGP PRIVATE KEY BLOCK",
)
PRIVATE_BOUNDARIES = tuple(
    (b"-----BEGIN " + label + b"-----", b"-----END " + label + b"-----")
    for label in PRIVATE_KEY_LABELS
) + (
    (
        b"---- " + b"BEGIN SSH2 ENCRYPTED PRIVATE KEY" + b" ----",
        b"---- " + b"END SSH2 ENCRYPTED PRIVATE KEY" + b" ----",
    ),
    (b"PuTTY-" + b"User-Key-File-2:", b"\nPrivate-MAC:"),
    (b"PuTTY-" + b"User-Key-File-3:", b"\nPrivate-MAC:"),
)


class AssemblyError(RuntimeError):
    """Raised when an RC input violates the fail-closed assembly contract."""


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def stream_contains_private_key(stream: BinaryIO) -> bool:
    begin_positions: list[int | None] = [None] * len(PRIVATE_BOUNDARIES)
    overlap_length = max(
        max(len(begin), len(end)) for begin, end in PRIVATE_BOUNDARIES
    ) - 1
    tail = b""
    stream_position = 0
    for block in iter(lambda: stream.read(SCAN_BLOCK_SIZE), b""):
        candidate = tail + block
        candidate_position = stream_position - len(tail)
        for index, (begin, end) in enumerate(PRIVATE_BOUNDARIES):
            begin_position = begin_positions[index]
            if begin_position is None:
                begin_offset = candidate.find(begin)
                if begin_offset == -1:
                    continue
                begin_position = candidate_position + begin_offset
                begin_positions[index] = begin_position
            end_search_offset = max(
                0,
                begin_position + len(begin) - candidate_position,
            )
            if candidate.find(end, end_search_offset) != -1:
                return True
        stream_position += len(block)
        tail = candidate[-overlap_length:]
    return False


def contains_private_key(path: Path) -> bool:
    with path.open("rb") as stream:
        return stream_contains_private_key(stream)


def require_zip_without_private_markers(path: Path, description: str) -> None:
    try:
        with zipfile.ZipFile(path) as archive:
            for member in archive.infolist():
                if member.is_dir():
                    continue
                with archive.open(member) as stream:
                    if stream_contains_private_key(stream):
                        raise AssemblyError(
                            f"{description} contains private-key material in {member.filename}"
                        )
    except zipfile.BadZipFile as error:
        raise AssemblyError(f"{description} is not a valid ZIP container: {path}") from error


def require_regular(path: Path, description: str) -> Path:
    if path.is_symlink() or not path.is_file():
        raise AssemblyError(f"{description} is missing or is not a regular file: {path}")
    lowered = path.name.lower()
    if "private" in lowered or "secret" in lowered or path.suffix.lower() == ".pem":
        raise AssemblyError(f"{description} has a forbidden secret-like filename: {path.name}")
    if contains_private_key(path):
        raise AssemblyError(f"{description} contains private-key material: {path}")
    if path.suffix.lower() in {".zip", ".vsix"}:
        require_zip_without_private_markers(path, description)
    return path


def require_one(root: Path, pattern: str, description: str) -> Path:
    matches = sorted(root.rglob(pattern))
    if len(matches) != 1:
        raise AssemblyError(
            f"{description} must match exactly one regular file; found {len(matches)} below {root} for {pattern}"
        )
    return require_regular(matches[0], description)


def copy_verified(source: Path, destination: Path, description: str) -> None:
    require_regular(source, description)
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(source, destination)


def relative_file_records(bundle_root: Path) -> list[dict[str, object]]:
    records: list[dict[str, object]] = []
    for path in sorted(bundle_root.rglob("*")):
        if path.is_symlink():
            raise AssemblyError(f"Evaluation bundle contains a symbolic link: {path}")
        if not path.is_file():
            continue
        relative = path.relative_to(bundle_root).as_posix()
        if relative in {"rc-validation-manifest.json", "SHA256SUMS.txt"}:
            continue
        records.append(
            {
                "path": relative,
                "sha256": sha256(path),
                "size_bytes": path.stat().st_size,
            }
        )
    return records


def assemble(args: argparse.Namespace) -> Path:
    if CANDIDATE_TAG_PATTERN.fullmatch(args.candidate_tag) is None:
        raise AssemblyError("candidate tag must match v0.1.0-rc.N with a positive RC number")
    if not re.fullmatch(r"[0-9a-f]{40}", args.revision):
        raise AssemblyError("revision must be a lowercase 40-character Git object ID")
    if not re.fullmatch(r"[1-9][0-9]*", args.run_id):
        raise AssemblyError("run ID must be a positive integer")
    if not re.fullmatch(r"[1-9][0-9]*", args.run_attempt):
        raise AssemblyError("run attempt must be a positive integer")

    input_root = args.input_root.resolve()
    repository_root = args.repository_root.resolve()
    output_root = args.output_dir.resolve()
    if output_root.exists():
        if not output_root.is_dir() or output_root.is_symlink() or any(output_root.iterdir()):
            raise AssemblyError(f"output directory must be absent or an empty regular directory: {output_root}")
    output_root.mkdir(parents=True, exist_ok=True)

    producer_roots = {
        "windows": input_root / "copperfin-windows-installers",
        "macos": input_root / "copperfin-macos-installers",
        "linux": input_root / "copperfin-linux-installers",
        "vsix": input_root / "copperfin-visualstudio-vsix",
        "sbom": input_root / "cyclonedx-sbom",
        "source": input_root / "copperfin-release-source",
    }
    for name, root in producer_roots.items():
        if root.is_symlink() or not root.is_dir():
            raise AssemblyError(f"required {name} artifact directory is missing or unsafe: {root}")
        for candidate in root.rglob("*"):
            if candidate.is_symlink():
                raise AssemblyError(f"required {name} artifact directory contains a symbolic link: {candidate}")

    payloads = (
        ("windows", "copperfin-*-Windows.exe", "installers/windows"),
        ("windows", "copperfin-*-Windows.zip", "installers/windows"),
        ("macos", "copperfin-*-Darwin.pkg", "installers/macos"),
        ("macos", "copperfin-*-Darwin.tar.gz", "installers/macos"),
        ("linux", "copperfin-*-Linux.deb", "installers/linux"),
        ("linux", "copperfin-*-Linux.rpm", "installers/linux"),
        ("linux", "copperfin-*-Linux.tar.gz", "installers/linux"),
        ("vsix", "*.vsix", "ide/visual-studio"),
    )
    for producer, pattern, relative_directory in payloads:
        source = require_one(producer_roots[producer], pattern, f"{producer} RC payload")
        copy_verified(source, output_root / relative_directory / source.name, f"{producer} RC payload")

    source_name = f"{SOURCE_PREFIX}{args.revision}.zip"
    source_archive = require_one(
        producer_roots["source"], source_name, "authoritative Corresponding Source"
    )
    copy_verified(source_archive, output_root / "source" / source_name, "Corresponding Source")

    sbom = require_one(producer_roots["sbom"], "sbom.cdx.json", "CycloneDX SBOM")
    copy_verified(sbom, output_root / "sbom" / sbom.name, "CycloneDX SBOM")

    license_paths = (
        "LICENSE",
        "LICENSE.md",
        "SOURCE.md",
        "THIRD_PARTY_NOTICES.md",
        "LICENSES/LicenseRef-Copperfin-Application-Runtime-Toolchain-Exception-1.0.txt",
        "docs/contracts/release-license-metadata.json",
    )
    for relative in license_paths:
        source = require_regular(repository_root / relative, f"release license document {relative}")
        copy_verified(source, output_root / "licensing" / relative, f"release license document {relative}")

    for relative in (
        "THIRD_PARTY_NOTICES.md",
        "LICENSES/LicenseRef-Copperfin-Application-Runtime-Toolchain-Exception-1.0.txt",
        "docs/contracts/release-license-metadata.json",
    ):
        artifact_copy = require_regular(producer_roots["sbom"] / relative, f"SBOM license companion {relative}")
        repository_copy = require_regular(repository_root / relative, f"repository license companion {relative}")
        if sha256(artifact_copy) != sha256(repository_copy):
            raise AssemblyError(f"SBOM license companion differs from the tagged repository: {relative}")

    tester_guide = require_regular(
        repository_root / "docs/35-rc1-evaluation-guide.md", "RC tester guide"
    )
    copy_verified(tester_guide, output_root / "RC-TESTER-README.md", "RC tester guide")

    manifest = {
        "schema_version": 1,
        "kind": "copperfin-private-evaluation-release-candidate",
        "candidate_tag": args.candidate_tag,
        "revision": args.revision,
        "repository": args.repository,
        "official_release": False,
        "workflow_run": {
            "id": int(args.run_id),
            "attempt": int(args.run_attempt),
            "url": f"{args.server_url.rstrip('/')}/{args.repository}/actions/runs/{args.run_id}",
        },
        "validation": {
            "exact_tag_and_revision": "passed",
            "native_release_readiness": "passed",
            "managed_ui": "passed",
            "installers": "passed",
            "visual_studio_vsix": "passed",
            "security_and_sbom": "passed",
        },
        "signing": {
            "windows_launcher_release_trust": "not claimed by this evaluation workflow",
            "macos_platform_signing": "unsupported",
            "linux_platform_signing": "unsupported",
        },
        "limitations": {
            "translations": "machine-generated catalogs retain documented human-review limits",
            "workflow_artifact_retention_days": RETENTION_DAYS,
        },
        "files": relative_file_records(output_root),
    }
    manifest_path = output_root / "rc-validation-manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    checksum_lines = []
    for path in sorted(output_root.rglob("*")):
        if path.is_file() and path.name != "SHA256SUMS.txt":
            checksum_lines.append(f"{sha256(path)}  {path.relative_to(output_root).as_posix()}")
    (output_root / "SHA256SUMS.txt").write_text("\n".join(checksum_lines) + "\n", encoding="utf-8")
    return output_root


def self_test() -> None:
    revision = "a" * 40
    with tempfile.TemporaryDirectory(prefix="copperfin-rc-assembly-") as temporary:
        root = Path(temporary)
        inputs = root / "inputs"
        repository = root / "repository"
        for name in (
            "copperfin-windows-installers",
            "copperfin-macos-installers",
            "copperfin-linux-installers",
            "copperfin-visualstudio-vsix",
            "cyclonedx-sbom",
            "copperfin-release-source",
        ):
            (inputs / name).mkdir(parents=True)
        fixture_files = {
            "copperfin-windows-installers/copperfin-0.1.0-Windows.exe": b"exe",
            "copperfin-windows-installers/copperfin-0.1.0-Windows.zip": b"winzip",
            "copperfin-macos-installers/copperfin-0.1.0-Darwin.pkg": b"pkg",
            "copperfin-macos-installers/copperfin-0.1.0-Darwin.tar.gz": b"mactgz",
            "copperfin-linux-installers/copperfin-0.1.0-Linux.deb": b"deb",
            "copperfin-linux-installers/copperfin-0.1.0-Linux.rpm": b"rpm",
            "copperfin-linux-installers/copperfin-0.1.0-Linux.tar.gz": b"linuxtgz",
            "copperfin-visualstudio-vsix/Copperfin.VisualStudio.vsix": b"vsix",
            "cyclonedx-sbom/sbom.cdx.json": b"{}\n",
        }
        for relative, data in fixture_files.items():
            path = inputs / relative
            if path.suffix in {".zip", ".vsix"}:
                with zipfile.ZipFile(path, "w") as archive:
                    archive.writestr("payload.bin", data)
            else:
                path.write_bytes(data)
        source_name = f"{SOURCE_PREFIX}{revision}.zip"
        for producer in (
            "copperfin-release-source",
        ):
            with zipfile.ZipFile(inputs / producer / source_name, "w") as archive:
                archive.writestr(f"Project-Copperfin-{revision}/README.md", b"exact-source")
                archive.writestr(
                    f"Project-Copperfin-{revision}/scripts/assemble-rc-candidate.py",
                    Path(__file__).read_bytes(),
                )

        licenses = {
            "LICENSE": b"GPL\n",
            "LICENSE.md": b"Plain language\n",
            "SOURCE.md": b"Corresponding Source\n",
            "THIRD_PARTY_NOTICES.md": b"Notices\n",
            "LICENSES/LicenseRef-Copperfin-Application-Runtime-Toolchain-Exception-1.0.txt": b"Exception\n",
            "docs/contracts/release-license-metadata.json": b"{}\n",
            "docs/35-rc1-evaluation-guide.md": b"# RC candidate\n",
        }
        for relative, data in licenses.items():
            path = repository / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
        for relative in (
            "THIRD_PARTY_NOTICES.md",
            "LICENSES/LicenseRef-Copperfin-Application-Runtime-Toolchain-Exception-1.0.txt",
            "docs/contracts/release-license-metadata.json",
        ):
            destination = inputs / "cyclonedx-sbom" / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            destination.write_bytes((repository / relative).read_bytes())

        arguments = argparse.Namespace(
            input_root=inputs,
            output_dir=root / "bundle",
            repository_root=repository,
            candidate_tag="v0.1.0-rc.2",
            revision=revision,
            repository="example/Project-Copperfin",
            run_id="123",
            run_attempt="1",
            server_url="https://github.example",
        )
        bundle = assemble(arguments)
        expected_bundle_files = {
            "RC-TESTER-README.md",
            "SHA256SUMS.txt",
            "ide/visual-studio/Copperfin.VisualStudio.vsix",
            "installers/linux/copperfin-0.1.0-Linux.deb",
            "installers/linux/copperfin-0.1.0-Linux.rpm",
            "installers/linux/copperfin-0.1.0-Linux.tar.gz",
            "installers/macos/copperfin-0.1.0-Darwin.pkg",
            "installers/macos/copperfin-0.1.0-Darwin.tar.gz",
            "installers/windows/copperfin-0.1.0-Windows.exe",
            "installers/windows/copperfin-0.1.0-Windows.zip",
            "licensing/LICENSE",
            "licensing/LICENSE.md",
            "licensing/SOURCE.md",
            "licensing/LICENSES/LicenseRef-Copperfin-Application-Runtime-Toolchain-Exception-1.0.txt",
            "licensing/THIRD_PARTY_NOTICES.md",
            "licensing/docs/contracts/release-license-metadata.json",
            "rc-validation-manifest.json",
            "sbom/sbom.cdx.json",
            f"source/{source_name}",
        }
        actual_bundle_files = {
            path.relative_to(bundle).as_posix() for path in bundle.rglob("*") if path.is_file()
        }
        if actual_bundle_files != expected_bundle_files:
            raise AssemblyError("self-test produced an unexpected evaluation bundle layout")
        manifest = json.loads((bundle / "rc-validation-manifest.json").read_text(encoding="utf-8"))
        if manifest["revision"] != revision or manifest["official_release"] is not False:
            raise AssemblyError("self-test validation manifest is invalid")

        for invalid_tag in (
            "v0.1.0-rc.0",
            "v0.1.0-rc.02",
            "v0.1.0-rc.-1",
            "v0.1.0-rc.2/../main",
            "v0.1.1-rc.2",
        ):
            invalid_tag_arguments = argparse.Namespace(**vars(arguments))
            invalid_tag_arguments.candidate_tag = invalid_tag
            invalid_tag_arguments.output_dir = root / f"invalid-tag-{len(invalid_tag)}"
            try:
                assemble(invalid_tag_arguments)
            except AssemblyError as error:
                if "candidate tag must match" not in str(error):
                    raise
            else:
                raise AssemblyError(f"self-test accepted invalid candidate tag: {invalid_tag}")

        private_envelopes = [
            begin + b"\nQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFB\n" + end
            for begin, end in PRIVATE_BOUNDARIES
        ]
        private_envelopes.append(
            PRIVATE_BOUNDARIES[0][0]
            + b"\n"
            + (b"A" * (2 * 1024 * 1024))
            + b"\n"
            + PRIVATE_BOUNDARIES[0][1]
        )
        end_before_begin = (
            b"A"
            * (
                SCAN_BLOCK_SIZE
                - len(PRIVATE_BOUNDARIES[0][1])
                - len(PRIVATE_BOUNDARIES[0][0])
                - 2
            )
            + PRIVATE_BOUNDARIES[0][1]
            + b"\n"
            + PRIVATE_BOUNDARIES[0][0]
            + b"\n"
            + (b"B" * SCAN_BLOCK_SIZE)
        )
        if stream_contains_private_key(io.BytesIO(end_before_begin)):
            raise AssemblyError("self-test accepted END-before-BEGIN as a private-key envelope")
        for index, private_envelope in enumerate(private_envelopes):
            bad_inputs = root / f"bad-inputs-{index}"
            shutil.copytree(inputs, bad_inputs)
            with zipfile.ZipFile(
                bad_inputs / "copperfin-visualstudio-vsix/Copperfin.VisualStudio.vsix", "w"
            ) as archive:
                archive.writestr("private-key-fixture.txt", private_envelope)
            bad_arguments = argparse.Namespace(**vars(arguments))
            bad_arguments.input_root = bad_inputs
            bad_arguments.output_dir = root / f"bad-bundle-{index}"
            try:
                assemble(bad_arguments)
            except AssemblyError as error:
                if "private-key material" not in str(error):
                    raise
            else:
                raise AssemblyError(
                    f"self-test accepted private-key envelope fixture {index}"
                )


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--self-test", action="store_true")
    parser.add_argument("--input-root", type=Path)
    parser.add_argument("--output-dir", type=Path)
    parser.add_argument("--repository-root", type=Path)
    parser.add_argument("--candidate-tag")
    parser.add_argument("--revision")
    parser.add_argument("--repository")
    parser.add_argument("--run-id")
    parser.add_argument("--run-attempt")
    parser.add_argument("--server-url", default="https://github.com")
    args = parser.parse_args(argv)
    if args.self_test:
        return args
    required = (
        "input_root",
        "output_dir",
        "repository_root",
        "candidate_tag",
        "revision",
        "repository",
        "run_id",
        "run_attempt",
    )
    missing = [name.replace("_", "-") for name in required if getattr(args, name) in (None, "")]
    if missing:
        parser.error("the following arguments are required: " + ", ".join(missing))
    return args


def main(argv: list[str]) -> int:
    try:
        args = parse_args(argv)
        if args.self_test:
            self_test()
            print("Copperfin RC candidate assembly self-test passed.")
        else:
            output = assemble(args)
            print(f"Assembled verified RC evaluation bundle at {output}")
        return 0
    except AssemblyError as error:
        print(f"RC candidate assembly failed: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
