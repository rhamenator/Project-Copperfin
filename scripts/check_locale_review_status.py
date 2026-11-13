#!/usr/bin/env python3
"""Track source, translation and independent-review state for v1 locale entries."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
LOCALES = ("es-419", "pt-BR")
STATUS_ROOT = ROOT / "docs" / "localization-reviews"
ISO_DATE = re.compile(r"\d{4}-\d{2}-\d{2}\Z")


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
    """Keep one reviewable JSON line per semantic key."""
    entries = list(ledger["entries"].items())
    lines = ["{", '  "version": 1,',
             f'  "locale": {json.dumps(ledger["locale"], ensure_ascii=False)},',
             '  "entries": {']
    for index, (key, record) in enumerate(entries):
        separator = "," if index + 1 < len(entries) else ""
        lines.append(f"    {json.dumps(key, ensure_ascii=False)}: "
                     f"{json.dumps(record, ensure_ascii=False, separators=(',', ':'))}{separator}")
    lines.extend(["  }", "}"])
    with path.open("w", encoding="utf-8", newline="\n") as stream:
        stream.write("\n".join(lines) + "\n")


def review_locales() -> tuple[str, ...]:
    inventory = ROOT / "resources" / "locales" / "inventory.txt"
    lines = inventory.read_text(encoding="utf-8").splitlines()
    if not lines or lines[0] != "version 1":
        raise ValueError("Malformed locale inventory header")
    entries = []
    for line in lines[1:]:
        fields = line.split()
        if len(fields) != 2 or fields[1] not in ("source", "unreviewed", "pseudo"):
            raise ValueError(f"Malformed locale inventory entry: {line}")
        if fields[1] == "unreviewed":
            entries.append(fields[0])
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
            STATUS_ROOT.mkdir(parents=True, exist_ok=True)
            previous = load_object(path, "Review ledger") if path.exists() else {}
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
        ledger = load_object(path, "Review ledger")
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
                        evidence_cache[evidence_name] = load_object(
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
