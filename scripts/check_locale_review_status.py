#!/usr/bin/env python3
"""Track source, translation and independent-review state for v1 locale entries."""

import argparse
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
LOCALES = ("es-419", "pt-BR")
STATUS_ROOT = ROOT / "docs" / "localization-reviews"


def digest(value: str) -> str:
    return hashlib.sha256(value.encode("utf-8")).hexdigest()


def load_catalog(locale: str) -> dict[str, str]:
    path = ROOT / "resources" / "locales" / locale / "strings.json"
    return json.loads(path.read_text(encoding="utf-8"))


def exceptions() -> dict[str, str]:
    data = json.loads((ROOT / "docs" / "localization-invariant-exceptions.json").read_text())
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
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


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
    for locale in LOCALES:
        translated = load_catalog(locale)
        if translated.keys() != source.keys():
            raise ValueError(f"Catalog key mismatch: {locale}")
        path = STATUS_ROOT / f"{locale}.json"
        if args.write:
            STATUS_ROOT.mkdir(parents=True, exist_ok=True)
            records = {}
            for key in sorted(source):
                value = translated[key]
                status = ("invariant" if key in invariant and value == source[key]
                          else "untriaged" if value == source[key] else "candidate")
                records[key] = {
                    "sourceSha256": digest(source[key]),
                    "sourceStatus": "current",
                    "translationSha256": digest(value),
                    "translationStatus": status,
                    "reviewStatus": "unreviewed",
                    "reviewProvenance": None,
                }
            write_ledger(path, {"version": 1, "locale": locale, "entries": records})
        ledger = json.loads(path.read_text(encoding="utf-8"))
        if ledger.get("version") != 1 or ledger.get("locale") != locale or ledger["entries"].keys() != source.keys():
            raise ValueError(f"Malformed or incomplete review ledger: {locale}")
        counts = {"candidate": 0, "untriaged": 0, "invariant": 0, "ai-reviewed": 0, "human-reviewed": 0}
        for key, record in ledger["entries"].items():
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
            if review != "unreviewed" and not record.get("reviewProvenance"):
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
                        evidence_cache[evidence_name] = json.loads(
                            (STATUS_ROOT / evidence_name).read_text(encoding="utf-8"))
                    evidence = evidence_cache[evidence_name]
                    matching = [entry for entry in evidence.get("entries", [])
                                if entry.get("locale") == locale and entry.get("key") == key]
                    expected_reviewer = "independent AI" if review == "ai-reviewed" else "human"
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
    raise SystemExit(main())
