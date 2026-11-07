#!/usr/bin/env python3
"""Validate the planning registry; never interpret it as installed-pack support."""

import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REGISTRY = ROOT / "docs" / "contracts" / "post-v1-locale-registry.json"
TAG = re.compile(r"^[a-z]{2,3}(?:-[A-Z][a-z]{3})?(?:-(?:[A-Z]{2}|[0-9]{3}))?$")
REQUIRED = {
    "en-US", "es-419", "pt-BR", "qps-ploc", "en-GB", "en-CA", "en-AU", "en-IN",
    "es-ES", "es-MX", "es-AR", "es-CL", "es-CO", "pt-PT", "fr-FR", "fr-CA",
    "de-DE", "de-AT", "de-CH", "it-IT", "nl-NL", "nl-BE", "da-DK", "sv-SE",
    "nb-NO", "fi-FI", "is-IS", "pl-PL", "cs-CZ", "sk-SK", "hu-HU", "ro-RO",
    "bg-BG", "uk-UA", "ru-RU", "sr-Cyrl-RS", "sr-Latn-RS", "hr-HR", "sl-SI",
    "el-GR", "et-EE", "lv-LV", "lt-LT", "ja-JP", "ko-KR", "zh-Hans-CN",
    "zh-Hant-TW", "zh-Hant-HK", "ar", "he-IL", "fa-IR", "ur-PK", "hi-IN",
    "bn-BD", "ta-IN", "te-IN", "mr-IN", "gu-IN", "pa-IN", "th-TH", "vi-VN",
    "id-ID", "ms-MY", "fil-PH", "tr-TR", "sw-KE", "sw-TZ", "af-ZA",
}


def validate(data):
    errors = []
    if data.get("schemaVersion") != 1 or data.get("purpose") != "planning-only":
        errors.append("registry version/purpose must be planning-only v1")
    locales = data.get("locales")
    if not isinstance(locales, list):
        return ["locales must be an array"]
    by_tag = {}
    for item in locales:
        tag = item.get("tag", "")
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
        if not isinstance(item.get("prerequisites"), list) or not item["prerequisites"]:
            errors.append(f"missing prerequisites {tag}")
        if item.get("entryEvidence") not in {"source-baseline", "required-per-key", "pseudo-generated"}:
            errors.append(f"invalid entry evidence policy {tag}")
    missing = REQUIRED - by_tag.keys()
    errors += [f"missing issue #7075 tag {tag}" for tag in sorted(missing)]
    for tag, item in by_tag.items():
        parent = item.get("preferredParent")
        if parent is None:
            continue
        if parent not in by_tag or parent == tag:
            errors.append(f"invalid parent {tag} -> {parent}")
            continue
        if tag != "qps-ploc" and parent != "en-US":
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
    data = json.loads(REGISTRY.read_text(encoding="utf-8"))
    errors = validate(data)
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    print(f"post-v1 planning registry: {len(data['locales'])} entries; {len(REQUIRED)} issue tags covered")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
