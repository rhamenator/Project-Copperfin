#!/usr/bin/env python3
"""Validate the planning registry; never interpret it as installed-pack support."""

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REGISTRY = ROOT / "docs" / "contracts" / "post-v1-locale-registry.json"
TAG = re.compile(r"^[a-z]{2,3}(?:-[A-Z][a-z]{3})?(?:-(?:[A-Z]{2}|[0-9]{3}))?$")
TOP_LEVEL_KEYS = {"schemaVersion", "purpose", "authority", "fallbackPolicy", "entryEvidenceSchema", "locales"}
LOCALE_KEYS = {"tag", "phase", "script", "direction", "preferredParent", "catalogStatus", "releaseStatus", "entryEvidence", "prerequisites"}
RTL_SCRIPTS = {"Arab", "Hebr"}
INVENTORY = ROOT / "resources" / "locales" / "inventory.txt"
REQUIRED = {
    "en-US", "es-419", "pt-BR", "qps-ploc", "en-GB", "en-CA", "en-AU", "en-IN",
    "es-ES", "es-MX", "es-AR", "es-CL", "es-CO", "pt-PT", "fr-FR", "fr-CA",
    "de-DE", "de-AT", "de-CH", "it-IT", "nl-NL", "nl-BE", "da-DK", "sv-SE",
    "nb-NO", "fi-FI", "is-IS", "pl-PL", "cs-CZ", "sk-SK", "hu-HU", "ro-RO",
    "bg-BG", "uk-UA", "ru-RU", "sr-Cyrl-RS", "sr-Latn-RS", "hr-HR", "sl-SI",
    "el-GR", "et-EE", "lv-LV", "lt-LT", "ja-JP", "ko-KR", "zh-Hans-CN",
    "zh-Hant-TW", "zh-Hant-HK", "ar", "ar-EG", "ar-SA", "ar-MA", "he-IL", "fa-IR", "ur-PK", "hi-IN",
    "bn-BD", "ta-IN", "te-IN", "mr-IN", "gu-IN", "pa-IN", "th-TH", "vi-VN",
    "id-ID", "ms-MY", "fil-PH", "tr-TR", "sw-KE", "sw-TZ", "af-ZA",
}


def reject_duplicate_keys(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"duplicate JSON key {key}")
        result[key] = value
    return result


def load_registry(path=REGISTRY):
    return json.loads(path.read_text(encoding="utf-8"), object_pairs_hook=reject_duplicate_keys)


def installed_catalog_status(path=INVENTORY):
    lines = path.read_text(encoding="utf-8").splitlines()
    if not lines or lines[0] != "version 1":
        raise ValueError("malformed installed locale inventory header")
    role_status = {
        "source": "source",
        "unreviewed": "present-unqualified",
        "pseudo": "test-only",
    }
    result = {}
    for line in lines[1:]:
        fields = line.split(" ")
        if len(fields) != 2 or fields[0] in result or fields[1] not in role_status:
            raise ValueError(f"malformed installed locale inventory entry: {line}")
        result[fields[0]] = role_status[fields[1]]
    return result


def validate(data):
    errors = []
    if not isinstance(data, dict):
        return ["registry must be an object"]
    extra = set(data) - TOP_LEVEL_KEYS
    if extra:
        errors.append(f"unknown registry fields: {', '.join(sorted(extra))}")
    if data.get("schemaVersion") != 1 or data.get("purpose") != "planning-only":
        errors.append("registry version/purpose must be planning-only v1")
    locales = data.get("locales")
    if not isinstance(locales, list):
        return ["locales must be an array"]
    by_tag = {}
    try:
        installed = installed_catalog_status()
    except (OSError, ValueError) as error:
        return [str(error)]
    for item in locales:
        if not isinstance(item, dict):
            errors.append("locale entry must be an object")
            continue
        extra = set(item) - LOCALE_KEYS
        if extra:
            errors.append(f"unknown locale fields: {', '.join(sorted(extra))}")
        tag = item.get("tag", "")
        if not isinstance(tag, str):
            errors.append("locale tag must be a string")
            continue
        if tag in by_tag:
            errors.append(f"duplicate tag {tag}")
        by_tag[tag] = item
        if tag != "qps-ploc" and not TAG.fullmatch(tag):
            errors.append(f"noncanonical tag {tag}")
        if item.get("direction") not in {"ltr", "rtl"}:
            errors.append(f"invalid direction {tag}")
        if item.get("script") not in {"Latn", "Cyrl", "Grek", "Hans", "Hant", "Jpan", "Kore", "Arab", "Hebr", "Deva", "Beng", "Taml", "Telu", "Gujr", "Guru", "Thai", "pseudo"}:
            errors.append(f"missing/invalid script {tag}")
        if item.get("phase") not in {"v1-owned", "western", "european", "cjk", "rtl", "south-southeast", "further", "test-only"}:
            errors.append(f"invalid phase {tag}")
        if item.get("catalogStatus") not in {"present-unqualified", "not-created", "source", "test-only"}:
            errors.append(f"invalid catalog status {tag}")
        if item.get("releaseStatus") != "not-qualified" and tag != "en-US":
            errors.append(f"unearned release status {tag}")
        expected_catalog_status = installed.get(tag, "not-created")
        if item.get("catalogStatus") != expected_catalog_status:
            errors.append(
                f"catalog status disagrees with installed inventory {tag}: "
                f"expected {expected_catalog_status}")
        if tag == "en-US" and (
                item.get("releaseStatus") != "source-baseline" or
                item.get("entryEvidence") != "source-baseline" or
                item.get("phase") != "v1-owned" or item.get("preferredParent") is not None):
            errors.append("en-US source baseline fields changed")
        if tag == "qps-ploc" and (
                item.get("phase") != "test-only" or item.get("script") != "pseudo" or
                item.get("direction") != "ltr" or item.get("preferredParent") is not None or
                item.get("releaseStatus") != "not-qualified" or
                item.get("entryEvidence") != "pseudo-generated"):
            errors.append("qps-ploc test-only fields changed")
        expected_direction = "rtl" if item.get("script") in RTL_SCRIPTS else "ltr"
        if item.get("direction") != expected_direction:
            errors.append(f"direction does not match script {tag}")
        tag_parts = tag.split("-")
        script_subtags = [part for part in tag_parts if len(part) == 4 and part.istitle()]
        if script_subtags and item.get("script") != script_subtags[0]:
            errors.append(f"script does not match tag {tag}")
        prerequisites = item.get("prerequisites")
        if (not isinstance(prerequisites, list) or not prerequisites or
                any(not isinstance(value, str) or not value.strip() for value in prerequisites) or
                len(set(prerequisites)) != len(prerequisites)):
            errors.append(f"missing prerequisites {tag}")
        if item.get("entryEvidence") not in {"source-baseline", "required-per-key", "pseudo-generated"}:
            errors.append(f"invalid entry evidence policy {tag}")
    missing = REQUIRED - by_tag.keys()
    errors += [f"missing issue #7075 tag {tag}" for tag in sorted(missing)]
    unexpected = by_tag.keys() - REQUIRED
    errors += [f"undeclared issue #7075 tag {tag}" for tag in sorted(unexpected)]
    for tag in ("es-ES", "pt-PT"):
        if tag in by_tag and by_tag[tag].get("preferredParent") is not None:
            errors.append(f"disallowed automatic regional inheritance {tag}")
    for tag, item in by_tag.items():
        parent = item.get("preferredParent")
        if parent is None:
            continue
        if parent not in by_tag or parent == tag:
            errors.append(f"invalid parent {tag} -> {parent}")
            continue
        if tag != "qps-ploc":
            child_script = item["script"]
            parent_script = by_tag[parent]["script"]
            if child_script != parent_script:
                errors.append(f"cross-script inheritance {tag} -> {parent}")
        seen = {tag}
        node = parent
        while node is not None and node not in seen and node in by_tag:
            seen.add(node)
            node = by_tag[node].get("preferredParent")
        if node in seen:
            errors.append(f"fallback cycle at {tag}")
    return errors


def main():
    try:
        data = load_registry()
        errors = validate(data)
    except (OSError, ValueError, TypeError, json.JSONDecodeError) as error:
        print(f"registry load failed: {error}", file=sys.stderr)
        return 1
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    print(f"post-v1 planning registry: {len(data['locales'])} entries; {len(REQUIRED)} issue tags covered")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
