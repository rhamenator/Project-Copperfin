#!/usr/bin/env python3
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

"""Assemble and verify an immutable Copperfin RC evaluation bundle.

Traceability: RQ-CF-REL-001; DQ-rc-evidence-v2-scope-separation;
DV-rc-evidence-v2-assembly-self-test; DV-rc-evidence-v2-schema-validation;
RQ-CF-REL-002; DQ-windows-installer-lifecycle-scope;
DV-windows-installer-lifecycle-contract; HZ-system-failure-01;
HZ-data-corruption-01; HZ-doc-command-01; RQ-CF-REL-003;
DQ-windows-vsix-lifecycle-scope; DV-windows-vsix-lifecycle-contract.
"""

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
VALIDATION_MANIFEST_SCHEMA = "rc-validation-manifest.schema.json"
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


def _json_values_equal(left: object, right: object) -> bool:
    """Compare JSON values without treating booleans as integers."""
    return type(left) is type(right) and left == right


SCHEMA_KEYWORDS = {
    "$schema",
    "$id",
    "$comment",
    "$defs",
    "title",
    "type",
    "const",
    "enum",
    "required",
    "additionalProperties",
    "properties",
    "pattern",
    "minLength",
    "minimum",
    "items",
}


def check_schema_vocabulary(schema: object, location: str = "$") -> None:
    if not isinstance(schema, dict):
        raise AssemblyError(f"manifest schema at {location} must be an object")
    if location == "$" and schema.get("$schema") != "https://json-schema.org/draft/2020-12/schema":
        raise AssemblyError("manifest schema must declare JSON Schema Draft 2020-12")
    unsupported = sorted(set(schema) - SCHEMA_KEYWORDS)
    if unsupported:
        raise AssemblyError(
            f"manifest schema at {location} uses unsupported keywords: {', '.join(unsupported)}"
        )
    for container_name in ("properties", "$defs"):
        children = schema.get(container_name, {})
        if not isinstance(children, dict):
            raise AssemblyError(
                f"manifest schema {container_name} at {location} must be an object"
            )
        for name, child in children.items():
            check_schema_vocabulary(child, f"{location}.{container_name}.{name}")
    if "items" in schema:
        check_schema_vocabulary(schema["items"], f"{location}.items")


def validate_schema_instance(instance: object, schema: object, location: str = "$") -> None:
    """Validate against the closed JSON Schema subset used by the RC manifest.

    This dependency-free validator deliberately rejects unsupported schema
    keywords. That keeps the release workflow fail-closed if the bundled
    Draft 2020-12 schema evolves beyond the vocabulary implemented here.
    """
    check_schema_vocabulary(schema, location)

    expected_type = schema.get("type")
    type_matches = {
        "object": lambda value: isinstance(value, dict),
        "array": lambda value: isinstance(value, list),
        "string": lambda value: isinstance(value, str),
        "integer": lambda value: isinstance(value, int) and not isinstance(value, bool),
        "boolean": lambda value: isinstance(value, bool),
    }
    if expected_type is not None:
        if expected_type not in type_matches:
            raise AssemblyError(
                f"manifest schema at {location} has unsupported type {expected_type!r}"
            )
        if not type_matches[expected_type](instance):
            raise AssemblyError(f"manifest value at {location} must be {expected_type}")

    if "const" in schema and not _json_values_equal(instance, schema["const"]):
        raise AssemblyError(f"manifest value at {location} does not match its required constant")
    if "enum" in schema:
        enum_values = schema["enum"]
        if not isinstance(enum_values, list):
            raise AssemblyError(f"manifest schema enum at {location} must be an array")
        if not any(_json_values_equal(instance, value) for value in enum_values):
            raise AssemblyError(f"manifest value at {location} is outside its allowed values")

    if isinstance(instance, dict):
        required = schema.get("required", [])
        properties = schema.get("properties", {})
        if not isinstance(required, list) or not all(isinstance(name, str) for name in required):
            raise AssemblyError(f"manifest schema required list at {location} is invalid")
        if not isinstance(properties, dict):
            raise AssemblyError(f"manifest schema properties at {location} must be an object")
        missing = [name for name in required if name not in instance]
        if missing:
            raise AssemblyError(
                f"manifest value at {location} is missing required properties: {', '.join(missing)}"
            )
        if schema.get("additionalProperties") is False:
            extras = sorted(set(instance) - set(properties))
            if extras:
                raise AssemblyError(
                    f"manifest value at {location} has unexpected properties: {', '.join(extras)}"
                )
        for name, value in instance.items():
            if name in properties:
                validate_schema_instance(value, properties[name], f"{location}.{name}")

    if isinstance(instance, list) and "items" in schema:
        for index, value in enumerate(instance):
            validate_schema_instance(value, schema["items"], f"{location}[{index}]")

    if isinstance(instance, str):
        minimum_length = schema.get("minLength")
        if minimum_length is not None:
            if not isinstance(minimum_length, int) or minimum_length < 0:
                raise AssemblyError(f"manifest schema minLength at {location} is invalid")
            if len(instance) < minimum_length:
                raise AssemblyError(f"manifest string at {location} is shorter than allowed")
        pattern = schema.get("pattern")
        if pattern is not None:
            if not isinstance(pattern, str):
                raise AssemblyError(f"manifest schema pattern at {location} must be a string")
            try:
                matches = re.search(pattern, instance) is not None
            except re.error as error:
                raise AssemblyError(
                    f"manifest schema pattern at {location} is invalid: {error}"
                ) from error
            if not matches:
                raise AssemblyError(f"manifest string at {location} does not match its pattern")

    if isinstance(instance, int) and not isinstance(instance, bool) and "minimum" in schema:
        minimum = schema["minimum"]
        if not isinstance(minimum, (int, float)) or isinstance(minimum, bool):
            raise AssemblyError(f"manifest schema minimum at {location} is invalid")
        if instance < minimum:
            raise AssemblyError(f"manifest integer at {location} is below its minimum")


def require_windows_installer_lifecycle_evidence(path: Path, installer: Path) -> None:
    try:
        evidence = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise AssemblyError(f"Windows installer lifecycle evidence is not valid JSON: {path}") from error
    expected_keys = {
        "schema_version",
        "kind",
        "installer_sha256",
        "install_root",
        "fresh_install",
        "installed_tree_contract",
        "locale_catalog_contract",
        "installed_cli_smoke",
        "same_version_maintenance_reinstall",
        "upgrade_from_previous_version",
        "silent_uninstall",
        "install_root_residue",
        "uninstall_registration_residue",
        "uninstall_registration_count_after_install",
        "installed_file_count",
        "installed_cli_stdout",
    }
    if not isinstance(evidence, dict) or set(evidence) != expected_keys:
        raise AssemblyError("Windows installer lifecycle evidence has an unexpected object shape")
    expected_pass_fields = (
        "fresh_install",
        "installed_tree_contract",
        "locale_catalog_contract",
        "installed_cli_smoke",
        "same_version_maintenance_reinstall",
        "silent_uninstall",
        "install_root_residue",
        "uninstall_registration_residue",
    )
    if (
        evidence["schema_version"] != 1
        or evidence["kind"] != "copperfin-windows-installer-lifecycle-result"
        or any(evidence[field] != "PASS" for field in expected_pass_fields)
        or evidence["upgrade_from_previous_version"] != "NOT_RUN"
        or evidence["installer_sha256"] != sha256(installer)
        or evidence["uninstall_registration_count_after_install"] != 1
        or not isinstance(evidence["installed_file_count"], int)
        or isinstance(evidence["installed_file_count"], bool)
        or evidence["installed_file_count"] < 1
        or not isinstance(evidence["install_root"], str)
        or not evidence["install_root"]
        or not isinstance(evidence["installed_cli_stdout"], str)
        or "copperfin_inspect" not in evidence["installed_cli_stdout"]
    ):
        raise AssemblyError("Windows installer lifecycle evidence does not prove the required bounded lifecycle")


def require_windows_vsix_lifecycle_evidence(path: Path, vsix: Path) -> None:
    try:
        evidence = json.loads(path.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise AssemblyError(f"Windows VSIX lifecycle evidence is not valid JSON: {path}") from error
    expected_keys = {
        "schema_version",
        "kind",
        "vsix_sha256",
        "extension_id",
        "extension_version",
        "visual_studio_instance_id",
        "installation",
        "package_registration_and_load",
        "extension_version_check",
        "supported_prg_open_and_command",
        "runner_owned_solution_identity",
        "same_version_reinstall",
        "upgrade_from_previous_version",
        "disablement",
        "uninstall",
        "extension_residue_check",
        "development_checkout_dependency",
    }
    if not isinstance(evidence, dict) or set(evidence) != expected_keys:
        raise AssemblyError("Windows VSIX lifecycle evidence has an unexpected object shape")
    expected_pass_fields = (
        "installation",
        "package_registration_and_load",
        "extension_version_check",
        "supported_prg_open_and_command",
        "runner_owned_solution_identity",
        "uninstall",
        "extension_residue_check",
    )
    if (
        evidence["schema_version"] != 1
        or evidence["kind"] != "copperfin-windows-vsix-lifecycle-result"
        or any(evidence[field] != "PASS" for field in expected_pass_fields)
        or any(
            evidence[field] != "NOT_RUN"
            for field in (
                "same_version_reinstall",
                "upgrade_from_previous_version",
                "disablement",
            )
        )
        or evidence["development_checkout_dependency"] != "PASS"
        or evidence["vsix_sha256"] != sha256(vsix)
        or evidence["extension_id"] != "Copperfin.VisualStudio"
        or not isinstance(evidence["extension_version"], str)
        or re.fullmatch(r"[0-9]+\.[0-9]+\.[0-9]+", evidence["extension_version"]) is None
        or not isinstance(evidence["visual_studio_instance_id"], str)
        or not evidence["visual_studio_instance_id"]
    ):
        raise AssemblyError("Windows VSIX lifecycle evidence does not prove the required bounded lifecycle")


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

    windows_installer = require_one(output_root / "installers/windows", "*.exe", "bundled Windows installer")
    windows_lifecycle = require_one(
        producer_roots["windows"],
        "windows-installer-lifecycle.json",
        "Windows installer lifecycle evidence",
    )
    require_windows_installer_lifecycle_evidence(windows_lifecycle, windows_installer)
    copy_verified(
        windows_lifecycle,
        output_root / "evidence/windows-installer-lifecycle.json",
        "Windows installer lifecycle evidence",
    )

    bundled_vsix = require_one(output_root / "ide/visual-studio", "*.vsix", "bundled Visual Studio VSIX")
    windows_vsix_lifecycle = require_one(
        producer_roots["vsix"],
        "windows-vsix-lifecycle.json",
        "Windows VSIX lifecycle evidence",
    )
    require_windows_vsix_lifecycle_evidence(windows_vsix_lifecycle, bundled_vsix)
    copy_verified(
        windows_vsix_lifecycle,
        output_root / "evidence/windows-vsix-lifecycle.json",
        "Windows VSIX lifecycle evidence",
    )

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

    manifest_schema = require_regular(
        repository_root / "docs/contracts/rc-validation-manifest-v3.schema.json",
        "RC validation manifest schema",
    )
    try:
        manifest_schema_document = json.loads(manifest_schema.read_text(encoding="utf-8"))
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise AssemblyError(f"RC validation manifest schema is not valid JSON: {error}") from error
    check_schema_vocabulary(manifest_schema_document)
    copy_verified(
        manifest_schema,
        output_root / VALIDATION_MANIFEST_SCHEMA,
        "RC validation manifest schema",
    )

    manifest = {
        "schema_version": 3,
        "schema": VALIDATION_MANIFEST_SCHEMA,
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
            "exact_tag_and_revision": "PASS",
            "native_release_readiness": "PASS",
            "managed_ui_build_and_smoke": "PASS",
            "installer_artifact_build_and_static_checks": "PASS",
            "installer_lifecycle": {
                "windows_fresh_install": "PASS",
                "windows_installed_cli_smoke": "PASS",
                "windows_same_version_maintenance_reinstall": "PASS",
                "windows_upgrade_from_previous_version": "NOT_RUN",
                "windows_silent_uninstall": "PASS",
                "windows_residue_checks": "PASS",
                "macos_productbuild": "NOT_RUN",
                "linux_deb": "NOT_RUN",
                "linux_rpm": "NOT_RUN",
            },
            "visual_studio_vsix_build_and_static_checks": "PASS",
            "visual_studio_vsix_lifecycle": {
                "windows_installation": "PASS",
                "windows_package_registration_and_load": "PASS",
                "windows_extension_version": "PASS",
                "windows_supported_prg_open_and_command": "PASS",
                "windows_same_version_reinstall": "NOT_RUN",
                "windows_upgrade_from_previous_version": "NOT_RUN",
                "windows_disablement": "NOT_RUN",
                "windows_uninstall": "PASS",
                "windows_extension_residue_check": "PASS",
                "development_checkout_independence": "PASS",
            },
            "security_and_sbom": "PASS",
        },
        "signing": {
            "windows_launcher_release_trust": "NOT_RUN",
            "windows_authenticode": "UNSUPPORTED_AND_DISCLOSED",
            "visual_studio_vsix": "UNSUPPORTED_AND_DISCLOSED",
            "macos_developer_id_and_notarization": "UNSUPPORTED_AND_DISCLOSED",
            "linux_package_repository": "UNSUPPORTED_AND_DISCLOSED",
        },
        "localization": {
            "catalog_structure_and_routing_checks": "PASS",
            "spanish_portuguese_linguistic_review": "NOT_RUN",
        },
        "compatibility": {
            "real_installed_vfp9_samples": "NOT_RUN",
        },
        "limitations": {
            "translations": "machine-generated catalogs retain documented human-review limits",
            "workflow_artifact_retention_days": RETENTION_DAYS,
        },
        "files": relative_file_records(output_root),
    }
    validate_schema_instance(manifest, manifest_schema_document)
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
        windows_installer_fixture = inputs / "copperfin-windows-installers/copperfin-0.1.0-Windows.exe"
        windows_lifecycle_fixture = {
            "schema_version": 1,
            "kind": "copperfin-windows-installer-lifecycle-result",
            "installer_sha256": sha256(windows_installer_fixture),
            "install_root": "C:\\hosted-runner\\copperfin-lifecycle",
            "fresh_install": "PASS",
            "installed_tree_contract": "PASS",
            "locale_catalog_contract": "PASS",
            "installed_cli_smoke": "PASS",
            "same_version_maintenance_reinstall": "PASS",
            "upgrade_from_previous_version": "NOT_RUN",
            "silent_uninstall": "PASS",
            "install_root_residue": "PASS",
            "uninstall_registration_residue": "PASS",
            "uninstall_registration_count_after_install": 1,
            "installed_file_count": 12,
            "installed_cli_stdout": "Usage: copperfin_inspect <path-to-vfp-asset>",
        }
        (inputs / "copperfin-windows-installers/windows-installer-lifecycle.json").write_text(
            json.dumps(windows_lifecycle_fixture, sort_keys=True) + "\n", encoding="utf-8"
        )
        vsix_fixture_path = inputs / "copperfin-visualstudio-vsix/Copperfin.VisualStudio.vsix"
        windows_vsix_lifecycle_fixture = {
            "schema_version": 1,
            "kind": "copperfin-windows-vsix-lifecycle-result",
            "vsix_sha256": sha256(vsix_fixture_path),
            "extension_id": "Copperfin.VisualStudio",
            "extension_version": "0.1.0",
            "visual_studio_instance_id": "fixture-instance",
            "installation": "PASS",
            "package_registration_and_load": "PASS",
            "extension_version_check": "PASS",
            "supported_prg_open_and_command": "PASS",
            "runner_owned_solution_identity": "PASS",
            "same_version_reinstall": "NOT_RUN",
            "upgrade_from_previous_version": "NOT_RUN",
            "disablement": "NOT_RUN",
            "uninstall": "PASS",
            "extension_residue_check": "PASS",
            "development_checkout_dependency": "PASS",
        }
        (inputs / "copperfin-visualstudio-vsix/windows-vsix-lifecycle.json").write_text(
            json.dumps(windows_vsix_lifecycle_fixture, sort_keys=True) + "\n", encoding="utf-8"
        )
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
            "docs/contracts/rc-validation-manifest-v3.schema.json": Path(
                __file__
            ).parent.parent.joinpath(
                "docs/contracts/rc-validation-manifest-v3.schema.json"
            ).read_bytes(),
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
            "evidence/windows-installer-lifecycle.json",
            "evidence/windows-vsix-lifecycle.json",
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
            VALIDATION_MANIFEST_SCHEMA,
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
        bundled_schema_document = json.loads(
            (bundle / VALIDATION_MANIFEST_SCHEMA).read_text(encoding="utf-8")
        )
        validate_schema_instance(manifest, bundled_schema_document)

        def require_schema_rejection(mutated_manifest: object, description: str) -> None:
            try:
                validate_schema_instance(mutated_manifest, bundled_schema_document)
            except AssemblyError:
                return
            raise AssemblyError(f"self-test schema accepted {description}")

        for required_field in (
            "kind",
            "candidate_tag",
            "repository",
            "workflow_run",
            "limitations",
            "files",
        ):
            missing_field_manifest = json.loads(json.dumps(manifest))
            del missing_field_manifest[required_field]
            require_schema_rejection(
                missing_field_manifest,
                f"manifest without required field {required_field}",
            )
        for malformed_url in (
            "not a URI",
            "https://github.example/example/Project-Copperfin/actions/runs/123\n",
            "https://github..example/example/Project-Copperfin/actions/runs/123",
        ):
            malformed_url_manifest = json.loads(json.dumps(manifest))
            malformed_url_manifest["workflow_run"]["url"] = malformed_url
            require_schema_rejection(malformed_url_manifest, "malformed workflow URL")

        expected_validation = {
            "exact_tag_and_revision": "PASS",
            "native_release_readiness": "PASS",
            "managed_ui_build_and_smoke": "PASS",
            "installer_artifact_build_and_static_checks": "PASS",
            "installer_lifecycle": {
                "windows_fresh_install": "PASS",
                "windows_installed_cli_smoke": "PASS",
                "windows_same_version_maintenance_reinstall": "PASS",
                "windows_upgrade_from_previous_version": "NOT_RUN",
                "windows_silent_uninstall": "PASS",
                "windows_residue_checks": "PASS",
                "macos_productbuild": "NOT_RUN",
                "linux_deb": "NOT_RUN",
                "linux_rpm": "NOT_RUN",
            },
            "visual_studio_vsix_build_and_static_checks": "PASS",
            "visual_studio_vsix_lifecycle": {
                "windows_installation": "PASS",
                "windows_package_registration_and_load": "PASS",
                "windows_extension_version": "PASS",
                "windows_supported_prg_open_and_command": "PASS",
                "windows_same_version_reinstall": "NOT_RUN",
                "windows_upgrade_from_previous_version": "NOT_RUN",
                "windows_disablement": "NOT_RUN",
                "windows_uninstall": "PASS",
                "windows_extension_residue_check": "PASS",
                "development_checkout_independence": "PASS",
            },
            "security_and_sbom": "PASS",
        }
        expected_signing = {
            "windows_launcher_release_trust": "NOT_RUN",
            "windows_authenticode": "UNSUPPORTED_AND_DISCLOSED",
            "visual_studio_vsix": "UNSUPPORTED_AND_DISCLOSED",
            "macos_developer_id_and_notarization": "UNSUPPORTED_AND_DISCLOSED",
            "linux_package_repository": "UNSUPPORTED_AND_DISCLOSED",
        }
        expected_localization = {
            "catalog_structure_and_routing_checks": "PASS",
            "spanish_portuguese_linguistic_review": "NOT_RUN",
        }
        expected_compatibility = {
            "real_installed_vfp9_samples": "NOT_RUN",
        }
        if (
            manifest["schema_version"] != 3
            or manifest["schema"] != VALIDATION_MANIFEST_SCHEMA
            or manifest["revision"] != revision
            or manifest["official_release"] is not False
            or manifest["validation"] != expected_validation
            or manifest["signing"] != expected_signing
            or manifest["localization"] != expected_localization
            or manifest["compatibility"] != expected_compatibility
        ):
            raise AssemblyError("self-test validation manifest is invalid")
        if "installers" in manifest["validation"] or "visual_studio_vsix" in manifest["validation"]:
            raise AssemblyError("self-test validation manifest retains an ambiguous lifecycle claim")
        bundled_schema = bundle / VALIDATION_MANIFEST_SCHEMA
        if sha256(bundled_schema) != sha256(
            repository / "docs/contracts/rc-validation-manifest-v3.schema.json"
        ):
            raise AssemblyError("self-test validation manifest schema is not the exact repository schema")

        lifecycle_path = bundle / "evidence/windows-installer-lifecycle.json"
        lifecycle = json.loads(lifecycle_path.read_text(encoding="utf-8"))
        lifecycle_mutations = (
            ("false fresh-install status", "fresh_install", "NOT_RUN"),
            ("false previous-version upgrade", "upgrade_from_previous_version", "PASS"),
            ("wrong installer digest", "installer_sha256", "0" * 64),
            ("missing installed file inventory", "installed_file_count", 0),
        )
        for description, field, replacement in lifecycle_mutations:
            mutated = dict(lifecycle)
            mutated[field] = replacement
            mutation_path = root / f"bad-lifecycle-{field}.json"
            mutation_path.write_text(json.dumps(mutated) + "\n", encoding="utf-8")
            try:
                require_windows_installer_lifecycle_evidence(
                    mutation_path,
                    bundle / "installers/windows/copperfin-0.1.0-Windows.exe",
                )
            except AssemblyError:
                pass
            else:
                raise AssemblyError(f"self-test accepted {description}")

        vsix_lifecycle_path = bundle / "evidence/windows-vsix-lifecycle.json"
        vsix_lifecycle = json.loads(vsix_lifecycle_path.read_text(encoding="utf-8"))
        vsix_lifecycle_mutations = (
            ("false VSIX installation status", "installation", "NOT_RUN"),
            ("false VSIX disablement status", "disablement", "PASS"),
            ("wrong VSIX digest", "vsix_sha256", "0" * 64),
            ("wrong VSIX identity", "extension_id", "Other.Extension"),
            (
                "unproved runner-owned solution identity",
                "runner_owned_solution_identity",
                "NOT_RUN",
            ),
            ("unproved checkout independence", "development_checkout_dependency", "NOT_RUN"),
        )
        for description, field, replacement in vsix_lifecycle_mutations:
            mutated = dict(vsix_lifecycle)
            mutated[field] = replacement
            mutation_path = root / f"bad-vsix-lifecycle-{field}.json"
            mutation_path.write_text(json.dumps(mutated) + "\n", encoding="utf-8")
            try:
                require_windows_vsix_lifecycle_evidence(
                    mutation_path,
                    bundle / "ide/visual-studio/Copperfin.VisualStudio.vsix",
                )
            except AssemblyError:
                pass
            else:
                raise AssemblyError(f"self-test accepted {description}")

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
