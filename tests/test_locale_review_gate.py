#!/usr/bin/env python3
"""Exercise the release gate on a small, independent locale inventory."""

import hashlib
import json
import subprocess
import sys
import tempfile
from pathlib import Path


SCRIPT = Path(__file__).resolve().parents[1] / "scripts" / "check_locale_review_status.py"


def write_json(path: Path, value: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False) + "\n", encoding="utf-8")


def digest(value: str) -> str:
    return hashlib.sha256(value.encode("utf-8")).hexdigest()


def run(root: Path, *args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run([sys.executable, str(root / "scripts" / SCRIPT.name), *args],
                          text=True, capture_output=True, check=False)


def main() -> None:
    with tempfile.TemporaryDirectory(prefix="copperfin-locale-review-") as directory:
        root = Path(directory)
        (root / "scripts").mkdir()
        (root / "scripts" / SCRIPT.name).write_bytes(SCRIPT.read_bytes())
        (root / "resources/locales").mkdir(parents=True)
        (root / "resources/locales/inventory.txt").write_text(
            "version 1\nen-US source\nde-DE unreviewed\nqps-ploc pseudo\n", encoding="utf-8")
        source = {"Greeting": "Hello", "Token": "VFP"}
        translated = {"Greeting": "Hallo", "Token": "VFP"}
        write_json(root / "resources/locales/en-US/strings.json", source)
        write_json(root / "resources/locales/de-DE/strings.json", translated)
        write_json(root / "docs/localization-invariant-exceptions.json", {"version": 1, "entries": []})
        created = run(root, "--write")
        assert created.returncode == 0, created.stdout + created.stderr
        ledger_path = root / "docs/localization-reviews/de-DE.json"
        ledger = json.loads(ledger_path.read_text(encoding="utf-8"))
        ledger["entries"]["Token"]["translationStatus"] = "intentional-source-text"
        evidence = {"date": "2026-11-03", "reviewerType": "independent AI",
                    "model": "test-reviewer", "sessions": ["test-session"],
                    "scope": "Two synthetic review-gate strings", "entries": []}
        for key in source:
            record = ledger["entries"][key]
            record["reviewStatus"] = "ai-reviewed"
            record["reviewProvenance"] = f"fixture.json#de-DE/{key}"
            evidence["entries"].append({"locale": "de-DE", "key": key,
                                        "sourceSha256": digest(source[key]),
                                        "translationSha256": digest(translated[key]),
                                        "verdict": "accept", "reason": "Synthetic fixture"})
        write_json(ledger_path, ledger)
        write_json(root / "docs/localization-reviews/fixture.json", evidence)
        passed = run(root, "--release")
        assert passed.returncode == 0, passed.stdout + passed.stderr
        rewritten = run(root, "--write")
        assert rewritten.returncode == 0, rewritten.stdout + rewritten.stderr
        assert json.loads(ledger_path.read_text(encoding="utf-8"))["entries"]["Token"][
            "translationStatus"] == "intentional-source-text"
        write_json(root / "resources/locales/de-DE/strings.json", {"Greeting": "Guten Tag", "Token": "VFP"})
        stale = run(root, "--release")
        assert stale.returncode != 0 and "changed since status" in stale.stdout
        write_json(root / "resources/locales/de-DE/strings.json", translated)
        (root / "resources/locales/inventory.txt").write_text(
            "version 1\nen-US source\nde-DE unreviewed\nde-AT overlay de-DE\nqps-ploc pseudo\n",
            encoding="utf-8")
        overlay_path = root / "resources/locales/de-AT/strings.json"
        write_json(overlay_path, {})
        overlay_ledger = run(root, "--write")
        assert overlay_ledger.returncode == 0, overlay_ledger.stdout + overlay_ledger.stderr
        pending = run(root, "--release")
        assert pending.returncode != 0 and "regional review evidence is missing" in pending.stdout
        regional = {"locale": "de-AT", "parent": "de-DE",
                    "parentCatalogSha256": hashlib.sha256(
                        (root / "resources/locales/de-DE/strings.json").read_bytes()).hexdigest(),
                    "overlayCatalogSha256": hashlib.sha256(overlay_path.read_bytes()).hexdigest(),
                    "reviewerType": "independent AI", "model": "test-reviewer",
                    "sessions": ["test-regional-session"], "verdict": "accept",
                    "residualUncertainty": "Synthetic fixture only"}
        write_json(root / "docs/localization-reviews/de-AT-regional.json", regional)
        inherited = run(root, "--release")
        assert inherited.returncode == 0, inherited.stdout + inherited.stderr
        (root / "resources/locales/de-DE/strings.json").write_text(
            json.dumps(translated, indent=2) + "\n", encoding="utf-8")
        stale_parent = run(root, "--release")
        assert stale_parent.returncode != 0 and "regional review evidence is stale" in stale_parent.stdout
        (root / "resources/locales/inventory.txt").write_text(
            "version 1\nen-US source\nde-DE unreviewed\nfr-FR unreviewed\nqps-ploc pseudo\n",
            encoding="utf-8")
        missing = run(root, "--release")
        assert missing.returncode != 0
    print("Locale release review gate fixture passed")


if __name__ == "__main__":
    main()
