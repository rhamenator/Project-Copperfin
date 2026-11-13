#!/usr/bin/env python3
"""Track source, translation and independent-review state for v1 locale entries."""

from __future__ import annotations

import argparse
import datetime
import errno
import hashlib
import json
import os
import re
import stat
import uuid
from pathlib import Path
from typing import Optional


ROOT = Path(__file__).resolve().parent.parent
LOCALES = ("es-419", "pt-BR")
STATUS_ROOT = ROOT / "docs" / "localization-reviews"
ISO_DATE = re.compile(r"\d{4}-\d{2}-\d{2}\Z")
INVENTORY_ENTRY = re.compile(r"[a-z][A-Za-z0-9-]* (source|unreviewed|pseudo)\Z")
HAS_DIRECTORY_FDS = all(
    operation in os.supports_dir_fd for operation in (os.open, os.stat, os.unlink, os.rename))


def digest(value: str) -> str:
    return hashlib.sha256(value.encode("utf-8")).hexdigest()


def reject_duplicate_keys(pairs: list[tuple[str, object]]) -> dict[str, object]:
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"Duplicate JSON key: {key}")
        result[key] = value
    return result


def load_json(path: Path) -> object:
    return json.loads(
        path.read_text(encoding="utf-8"), object_pairs_hook=reject_duplicate_keys)


def load_catalog(locale: str) -> dict[str, str]:
    path = ROOT / "resources" / "locales" / locale / "strings.json"
    data = load_json(path)
    if not isinstance(data, dict) or any(
            not isinstance(key, str) or not isinstance(value, str)
            for key, value in data.items()):
        raise ValueError(f"Catalog must map string keys to string values: {path}")
    return data


def load_object(path: Path, label: str) -> dict:
    data = load_json(path)
    if not isinstance(data, dict):
        raise ValueError(f"{label} must be a JSON object: {path}")
    return data


def status_member_name(path: Path) -> str:
    """Return a direct STATUS_ROOT member name after containment checks."""
    if path.parent != STATUS_ROOT or path.name in ("", ".", ".."):
        raise ValueError(f"Review-status path is not a direct managed file: {path}")
    validate_status_root()
    return path.name


def validate_status_root() -> os.stat_result:
    """Validate containment and return the directory identity checked by path."""
    try:
        details = os.stat(STATUS_ROOT, follow_symlinks=False)
        if not stat.S_ISDIR(details.st_mode):
            raise ValueError
        project_root = ROOT.resolve(strict=True)
        status_root = STATUS_ROOT.resolve(strict=True)
        status_root.relative_to(project_root)
    except (OSError, ValueError):
        raise ValueError(
            f"Review-status directory must be a real directory inside the project: {STATUS_ROOT}") from None
    return details


def open_status_root() -> Optional[int]:
    """Walk from the selected project root without following directory links."""
    if not HAS_DIRECTORY_FDS:
        return None
    if STATUS_ROOT != ROOT / "docs" / "localization-reviews":
        raise ValueError(f"Unexpected managed review-status directory: {STATUS_ROOT}")
    flags = os.O_RDONLY | getattr(os, "O_CLOEXEC", 0)
    flags |= getattr(os, "O_DIRECTORY", 0) | getattr(os, "O_NOFOLLOW", 0)
    root_descriptor = None
    docs_descriptor = None
    try:
        root_descriptor = os.open(ROOT, flags)
        docs_descriptor = os.open("docs", flags, dir_fd=root_descriptor)
        return os.open("localization-reviews", flags, dir_fd=docs_descriptor)
    except OSError as error:
        raise ValueError(f"Unable to open managed review-status directory: {STATUS_ROOT}") from error
    finally:
        if docs_descriptor is not None:
            os.close(docs_descriptor)
        if root_descriptor is not None:
            os.close(root_descriptor)


def ensure_regular_status_fd(file_descriptor: int, path: Path) -> None:
    details = os.fstat(file_descriptor)
    if not stat.S_ISREG(details.st_mode):
        raise ValueError(f"Review-status file must be a regular file: {path}")


def load_status_object(path: Path, label: str) -> dict:
    """Load a direct managed JSON file without following links or blocking on FIFOs."""
    name = status_member_name(path)
    directory = open_status_root()
    flags = os.O_RDONLY | getattr(os, "O_CLOEXEC", 0) | getattr(os, "O_BINARY", 0)
    flags |= getattr(os, "O_NOFOLLOW", 0) | getattr(os, "O_NONBLOCK", 0)
    try:
        try:
            if directory is None:
                details = os.stat(path, follow_symlinks=False)
                if not stat.S_ISREG(details.st_mode):
                    raise ValueError(f"Review-status file must be a regular file: {path}")
                descriptor = os.open(path, flags)
            else:
                descriptor = os.open(name, flags, dir_fd=directory)
        except OSError as error:
            raise ValueError(f"Unable to open regular {label.lower()}: {path}") from error
        try:
            ensure_regular_status_fd(descriptor, path)
            with os.fdopen(descriptor, "r", encoding="utf-8", closefd=False) as stream:
                data = json.load(stream, object_pairs_hook=reject_duplicate_keys)
        finally:
            os.close(descriptor)
    finally:
        if directory is not None:
            os.close(directory)
    if not isinstance(data, dict):
        raise ValueError(f"{label} must be a JSON object: {path}")
    return data


def exceptions() -> dict[str, str]:
    data = load_object(ROOT / "docs" / "localization-invariant-exceptions.json", "Invariant exceptions")
    if data.get("version") != 1:
        raise ValueError("Unsupported invariant exception version")
    result = {}
    for entry in data["entries"]:
        key, value = entry["key"], entry["sourceValue"]
        if key in result or not entry["reason"].strip():
            raise ValueError(f"Duplicate or unexplained invariant exception: {key}")
        result[key] = value
    return result


def write_ledger(path: Path, ledger: dict) -> None:
    """Atomically replace one direct, regular managed ledger without following links."""
    entries = list(ledger["entries"].items())
    lines = ["{", '  "version": 1,',
             f'  "locale": {json.dumps(ledger["locale"], ensure_ascii=False)},',
             '  "entries": {']
    for index, (key, record) in enumerate(entries):
        separator = "," if index + 1 < len(entries) else ""
        lines.append(f"    {json.dumps(key, ensure_ascii=False)}: "
                     f"{json.dumps(record, ensure_ascii=False, separators=(',', ':'))}{separator}")
    lines.extend(["  }", "}"])
    payload = ("\n".join(lines) + "\n").encode("utf-8")
    name = status_member_name(path)
    directory = open_status_root()
    temporary_name = f".{name}.{uuid.uuid4().hex}.tmp"
    create_flags = (os.O_WRONLY | os.O_CREAT | os.O_EXCL |
                    getattr(os, "O_CLOEXEC", 0) | getattr(os, "O_BINARY", 0))
    create_flags |= getattr(os, "O_NOFOLLOW", 0)
    temporary_descriptor = None
    temporary_path = STATUS_ROOT / temporary_name
    try:
        try:
            current = (os.stat(path, follow_symlinks=False) if directory is None else
                       os.stat(name, dir_fd=directory, follow_symlinks=False))
        except FileNotFoundError:
            current = None
        except OSError as error:
            raise ValueError(f"Unable to inspect review ledger: {path}") from error
        if current is not None and not stat.S_ISREG(current.st_mode):
            raise ValueError(f"Review ledger must be a regular file: {path}")
        destination_mode = stat.S_IMODE(current.st_mode) if current is not None else 0o666
        try:
            if directory is None:
                temporary_descriptor = os.open(temporary_path, create_flags, destination_mode)
            else:
                temporary_descriptor = os.open(
                    temporary_name, create_flags, destination_mode, dir_fd=directory)
            ensure_regular_status_fd(temporary_descriptor, path)
            if current is not None and hasattr(os, "fchmod"):
                os.fchmod(temporary_descriptor, destination_mode)
            with os.fdopen(temporary_descriptor, "wb", closefd=False) as stream:
                stream.write(payload)
                stream.flush()
                os.fsync(temporary_descriptor)
            # Windows does not grant delete sharing to this os.open handle, so
            # release it before os.replace attempts to rename the temporary file.
            os.close(temporary_descriptor)
            temporary_descriptor = None
            # Reject a static link/nonregular destination. If it is replaced after
            # this check, rename replaces that directory entry rather than following it.
            try:
                destination = (os.stat(path, follow_symlinks=False) if directory is None else
                               os.stat(name, dir_fd=directory, follow_symlinks=False))
            except FileNotFoundError:
                destination = None
            if destination is not None and not stat.S_ISREG(destination.st_mode):
                raise ValueError(f"Review ledger must be a regular file: {path}")
            if directory is None:
                os.replace(temporary_path, path)
            else:
                os.replace(temporary_name, name, src_dir_fd=directory, dst_dir_fd=directory)
                try:
                    os.fsync(directory)
                except OSError as error:
                    if error.errno not in (errno.EINVAL, errno.ENOTSUP, errno.ENOSYS):
                        raise
            temporary_name = ""
        finally:
            if temporary_descriptor is not None:
                os.close(temporary_descriptor)
            if temporary_name:
                try:
                    if directory is None:
                        temporary_path.unlink()
                    else:
                        os.unlink(temporary_name, dir_fd=directory)
                except FileNotFoundError:
                    pass
    finally:
        if directory is not None:
            os.close(directory)


def review_locales() -> tuple[str, ...]:
    inventory = ROOT / "resources" / "locales" / "inventory.txt"
    lines = inventory.read_text(encoding="utf-8").splitlines()
    if not lines or lines[0] != "version 1":
        raise ValueError("Malformed locale inventory header")
    entries = []
    for line in lines[1:]:
        if not INVENTORY_ENTRY.fullmatch(line):
            raise ValueError(f"Malformed locale inventory entry: {line}")
        locale, role = line.split(" ")
        if role == "unreviewed":
            entries.append(locale)
    if tuple(entries) != LOCALES:
        raise ValueError(
            f"Review locale list does not match unreviewed inventory entries: {entries}")
    return tuple(entries)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--write", action="store_true", help="refresh the per-entry ledger")
    parser.add_argument("--release", action="store_true", help="fail while translation or review is unresolved")
    args = parser.parse_args()
    source = load_catalog("en-US")
    invariant = exceptions()
    for key, value in invariant.items():
        if source.get(key) != value:
            raise ValueError(f"Invariant exception source changed: {key}")
    failures = []
    evidence_cache = {}
    for locale in review_locales():
        translated = load_catalog(locale)
        if translated.keys() != source.keys():
            raise ValueError(f"Catalog key mismatch: {locale}")
        path = STATUS_ROOT / f"{locale}.json"
        if args.write:
            previous = load_status_object(path, "Review ledger") if os.path.lexists(path) else {}
            previous_entries = previous.get("entries", {})
            if not isinstance(previous_entries, dict):
                raise ValueError(f"Malformed review ledger entries: {path}")
            records = {}
            for key in sorted(source):
                value = translated[key]
                status = ("invariant" if key in invariant and value == source[key]
                          else "untriaged" if value == source[key] else "candidate")
                record = {
                    "sourceSha256": digest(source[key]),
                    "sourceStatus": "current",
                    "translationSha256": digest(value),
                    "translationStatus": status,
                    "reviewStatus": "unreviewed",
                    "reviewProvenance": None,
                }
                old = previous_entries.get(key)
                if (isinstance(old, dict) and old.get("sourceSha256") == record["sourceSha256"] and
                        old.get("translationSha256") == record["translationSha256"] and
                        old.get("reviewStatus") in ("ai-reviewed", "human-reviewed") and
                        isinstance(old.get("reviewProvenance"), str)):
                    record["reviewStatus"] = old["reviewStatus"]
                    record["reviewProvenance"] = old["reviewProvenance"]
                records[key] = record
            write_ledger(path, {"version": 1, "locale": locale, "entries": records})
        ledger = load_status_object(path, "Review ledger")
        entries = ledger.get("entries")
        if (ledger.get("version") != 1 or ledger.get("locale") != locale or
                not isinstance(entries, dict) or entries.keys() != source.keys()):
            raise ValueError(f"Malformed or incomplete review ledger: {locale}")
        counts = {"candidate": 0, "untriaged": 0, "invariant": 0, "ai-reviewed": 0, "human-reviewed": 0}
        for key, record in ledger["entries"].items():
            if not isinstance(record, dict):
                failures.append(f"{locale}/{key}: review record must be an object")
                continue
            if (record.get("sourceSha256") != digest(source[key]) or
                    record.get("translationSha256") != digest(translated[key]) or
                    record.get("sourceStatus") != "current"):
                failures.append(f"{locale}/{key}: source or translation changed since status recording")
                continue
            status = record.get("translationStatus")
            review = record.get("reviewStatus")
            if status not in ("candidate", "untriaged", "invariant") or review not in (
                    "unreviewed", "ai-reviewed", "human-reviewed"):
                failures.append(f"{locale}/{key}: invalid status")
                continue
            if (status == "invariant") != (key in invariant and translated[key] == source[key]):
                failures.append(f"{locale}/{key}: invariant exception mismatch")
            if status == "untriaged" and translated[key] != source[key]:
                failures.append(f"{locale}/{key}: changed value marked untriaged")
            if status == "candidate" and translated[key] == source[key]:
                failures.append(f"{locale}/{key}: source-identical value marked translated candidate")
            if review != "unreviewed" and not isinstance(record.get("reviewProvenance"), str):
                failures.append(f"{locale}/{key}: reviewed status lacks provenance")
            elif review != "unreviewed":
                provenance = record["reviewProvenance"]
                evidence_name, _, entry_id = provenance.partition("#")
                if (not evidence_name.endswith(".json") or
                        Path(evidence_name).name != evidence_name or
                        entry_id != f"{locale}/{key}"):
                    failures.append(f"{locale}/{key}: invalid review provenance path")
                else:
                    if evidence_name not in evidence_cache:
                        evidence_cache[evidence_name] = load_status_object(
                            STATUS_ROOT / evidence_name, "Review evidence")
                    evidence = evidence_cache[evidence_name]
                    evidence_entries = evidence.get("entries")
                    if not isinstance(evidence_entries, list) or any(
                            not isinstance(entry, dict) for entry in evidence_entries):
                        raise ValueError(f"Malformed review evidence entries: {evidence_name}")
                    if (not isinstance(evidence.get("date"), str) or
                            not ISO_DATE.fullmatch(evidence["date"]) or
                            not isinstance(evidence.get("scope"), str) or not evidence["scope"].strip() or
                            not isinstance(evidence.get("residualUncertainty"), str) or
                            not evidence["residualUncertainty"].strip()):
                        raise ValueError(
                            f"Review evidence lacks date, scope, or residual uncertainty: {evidence_name}")
                    try:
                        datetime.date.fromisoformat(evidence["date"])
                    except ValueError:
                        raise ValueError(
                            f"Review evidence has an invalid date: {evidence_name}") from None
                    matching = [entry for entry in evidence.get("entries", [])
                                if entry.get("locale") == locale and entry.get("key") == key]
                    expected_reviewer = "independent AI" if review == "ai-reviewed" else "human"
                    if review == "ai-reviewed" and (
                            not isinstance(evidence.get("model"), str) or not evidence["model"].strip() or
                            not isinstance(evidence.get("sessions"), list) or not evidence["sessions"] or
                            any(not isinstance(session, str) or not session.strip()
                                for session in evidence["sessions"])):
                        raise ValueError(f"AI review evidence lacks model or session metadata: {evidence_name}")
                    if review == "human-reviewed" and (
                            not isinstance(evidence.get("reviewerId"), str) or
                            not evidence["reviewerId"].strip() or
                            not isinstance(evidence.get("qualifications"), str) or
                            not evidence["qualifications"].strip()):
                        raise ValueError(
                            f"Human review evidence lacks reviewer identity or qualifications: {evidence_name}")
                    if (evidence.get("reviewerType") != expected_reviewer or
                            len(matching) != 1 or matching[0].get("verdict") != "accept" or
                            matching[0].get("sourceSha256") != record["sourceSha256"] or
                            matching[0].get("translationSha256") != record["translationSha256"]):
                        failures.append(f"{locale}/{key}: review evidence does not match the entry")
            counts[status] += 1
            if review in ("ai-reviewed", "human-reviewed"):
                counts[review] += 1
        print(f"{locale}: {len(source)} populated; {counts['candidate']} translation candidates; "
              f"{counts['untriaged']} unchanged/untriaged; {counts['invariant']} exact invariants; "
              f"{counts['ai-reviewed']} AI-reviewed; {counts['human-reviewed']} human-reviewed")
        if args.release and (counts["untriaged"] or counts["ai-reviewed"] + counts["human-reviewed"] != len(source)):
            failures.append(f"{locale}: translation triage or independent review incomplete")
    for failure in failures[:20]:
        print(f"FAIL: {failure}")
    if len(failures) > 20:
        print(f"... and {len(failures)-20} more failures")
    return 1 if failures else 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, KeyError, TypeError, AttributeError, json.JSONDecodeError) as error:
        print(f"FAIL: {error}")
        raise SystemExit(1) from None
