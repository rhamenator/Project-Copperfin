#!/usr/bin/env python3
"""Track source, translation and independent-review state for v1 locale entries."""

import argparse
import hashlib
import json
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
STATUS_ROOT = ROOT / "docs" / "localization-reviews"


def reviewed_locales() -> dict[str, tuple[str, str | None]]:
    lines = (ROOT / "resources" / "locales" / "inventory.txt").read_text(
        encoding="utf-8").splitlines()
    if not lines or lines[0] != "version 1":
        raise ValueError("Unsupported or missing locale inventory version")
    locales = {}
    declared = set()
    roles = {}
    for line in lines[1:]:
        fields = line.split(" ")
        if len(fields) not in (2, 3):
            raise ValueError(f"Malformed locale inventory entry: {line}")
        locale, role = fields[:2]
        parent = fields[2] if len(fields) == 3 else None
        if (locale in declared or role not in ("source", "unreviewed", "overlay", "pseudo") or
                (role == "overlay") != (parent is not None) or
                (role == "source" and locale != "en-US") or
                (role == "pseudo" and locale != "qps-ploc") or
                (parent is not None and (parent not in locales or
                 roles[parent] == "overlay" or not (
                     (locale.startswith(parent + "-") and
                      len(parent.rsplit("-", 1)[-1]) != 1) or
                     (parent == "es-419" and is_latin_american_spanish(locale)))))):
            raise ValueError(f"Invalid locale inventory entry: {line}")
        declared.add(locale)
        roles[locale] = role
        if role not in ("source", "pseudo"):
            locales[locale] = (role, parent)
    if not locales or sum(role == "source" for role in roles.values()) != 1 or \
            sum(role == "pseudo" for role in roles.values()) != 1:
        raise ValueError("Locale inventory requires reviewed locales, en-US source, and qps-ploc pseudo")
    return locales


def is_latin_american_spanish(locale: str) -> bool:
    parts = locale.split("-")
    if parts[0] != "es" or len(parts) < 2:
        return False
    region_index = 2 if len(parts[1]) == 4 and parts[1].isascii() and parts[1].isalpha() else 1
    return len(parts) > region_index and parts[region_index] in {
        "AR", "BO", "BR", "BZ", "CL", "CO", "CR", "CU", "DO", "EC", "GT", "HN",
        "JP", "MX", "NI", "PA", "PE", "PR", "PY", "SV", "US", "UY", "VE", "419",
    }


def digest(value: str) -> str:
    return hashlib.sha256(value.encode("utf-8")).hexdigest()


def load_catalog(locale: str) -> dict[str, str]:
    path = ROOT / "resources" / "locales" / locale / "strings.json"
    return json.loads(path.read_text(encoding="utf-8"))


def load_object(path: Path, label: str) -> dict:
    data = json.loads(path.read_text(encoding="utf-8"))
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
    for locale, (role, parent) in reviewed_locales().items():
        translated = load_catalog(locale)
        if (role == "overlay" and not translated.keys() <= source.keys()) or (
                role != "overlay" and translated.keys() != source.keys()):
            raise ValueError(f"Catalog key mismatch: {locale}")
        path = STATUS_ROOT / f"{locale}.json"
        if args.write:
            STATUS_ROOT.mkdir(parents=True, exist_ok=True)
            previous = load_object(path, "Review ledger") if path.exists() else {}
            previous_entries = previous.get("entries", {})
            if not isinstance(previous_entries, dict):
                raise ValueError(f"Malformed review ledger entries: {path}")
            records = {}
            for key in sorted(translated):
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
                if (status == "untriaged" and isinstance(old, dict) and
                        old.get("sourceSha256") == record["sourceSha256"] and
                        old.get("translationSha256") == record["translationSha256"] and
                        old.get("translationStatus") == "intentional-source-text"):
                    record["translationStatus"] = "intentional-source-text"
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
                not isinstance(entries, dict) or entries.keys() != translated.keys()):
            raise ValueError(f"Malformed or incomplete review ledger: {locale}")
        counts = {"candidate": 0, "untriaged": 0, "invariant": 0,
                  "intentional-source-text": 0, "ai-reviewed": 0, "human-reviewed": 0}
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
            if status not in ("candidate", "untriaged", "invariant", "intentional-source-text") or review not in (
                    "unreviewed", "ai-reviewed", "human-reviewed"):
                failures.append(f"{locale}/{key}: invalid status")
                continue
            if (status == "invariant") != (key in invariant and translated[key] == source[key]):
                failures.append(f"{locale}/{key}: invariant exception mismatch")
            if status == "untriaged" and translated[key] != source[key]:
                failures.append(f"{locale}/{key}: changed value marked untriaged")
            if status == "intentional-source-text" and (
                    translated[key] != source[key] or review == "unreviewed"):
                failures.append(f"{locale}/{key}: intentional source text requires unchanged value and review")
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
                    if (not isinstance(evidence.get("date"), str) or not evidence["date"].strip() or
                            not isinstance(evidence.get("scope"), str) or not evidence["scope"].strip()):
                        raise ValueError(f"Review evidence lacks date or scope: {evidence_name}")
                    matching = [entry for entry in evidence.get("entries", [])
                                if entry.get("locale") == locale and entry.get("key") == key]
                    expected_reviewer = "independent AI" if review == "ai-reviewed" else "human"
                    if review == "ai-reviewed" and (
                            not isinstance(evidence.get("model"), str) or not evidence["model"].strip() or
                            not isinstance(evidence.get("sessions"), list) or not evidence["sessions"] or
                            any(not isinstance(session, str) or not session.strip()
                                for session in evidence["sessions"])):
                        raise ValueError(f"AI review evidence lacks model or session metadata: {evidence_name}")
                    if (evidence.get("reviewerType") != expected_reviewer or
                            len(matching) != 1 or matching[0].get("verdict") != "accept" or
                            matching[0].get("sourceSha256") != record["sourceSha256"] or
                            matching[0].get("translationSha256") != record["translationSha256"]):
                        failures.append(f"{locale}/{key}: review evidence does not match the entry")
            counts[status] += 1
            if review in ("ai-reviewed", "human-reviewed"):
                counts[review] += 1
        print(f"{locale}: {len(translated)} populated; {counts['candidate']} translation candidates; "
              f"{counts['untriaged']} unchanged/untriaged; "
              f"{counts['intentional-source-text']} intentional source text; "
              f"{counts['invariant']} exact invariants; "
              f"{counts['ai-reviewed']} AI-reviewed; {counts['human-reviewed']} human-reviewed")
        if args.release and (counts["untriaged"] or counts["ai-reviewed"] + counts["human-reviewed"] != len(translated)):
            failures.append(f"{locale}: translation triage or independent review incomplete")
        if role == "overlay":
            regional_path = STATUS_ROOT / f"{locale}-regional.json"
            if not regional_path.exists():
                if args.release:
                    failures.append(f"{locale}: regional review evidence is missing")
            else:
                regional = load_object(regional_path, "Regional review evidence")
                parent_path = ROOT / "resources" / "locales" / parent / "strings.json"
                overlay_path = ROOT / "resources" / "locales" / locale / "strings.json"
                expected_parent_hash = hashlib.sha256(parent_path.read_bytes()).hexdigest()
                expected_overlay_hash = hashlib.sha256(overlay_path.read_bytes()).hexdigest()
                if (regional.get("locale") != locale or regional.get("parent") != parent or
                        regional.get("parentCatalogSha256") != expected_parent_hash or
                        regional.get("overlayCatalogSha256") != expected_overlay_hash or
                        regional.get("reviewerType") != "independent AI" or
                        regional.get("verdict") != "accept" or
                        not isinstance(regional.get("model"), str) or not regional["model"].strip() or
                        not isinstance(regional.get("sessions"), list) or not regional["sessions"] or
                        not isinstance(regional.get("residualUncertainty"), str) or
                        not regional["residualUncertainty"].strip()):
                    failures.append(f"{locale}: regional review evidence is stale or incomplete")
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
