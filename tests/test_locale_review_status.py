#!/usr/bin/env python3
"""Negative-path tests for the locale review evidence validator."""

from __future__ import annotations

import contextlib
import importlib.util
import io
import json
import sys
import tempfile
from pathlib import Path
from unittest import mock


SOURCE_ROOT = Path(__file__).resolve().parent.parent
SCRIPT = SOURCE_ROOT / "scripts" / "check_locale_review_status.py"


def write_json(path: Path, value: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8", newline="\n") as stream:
        stream.write(json.dumps(value, ensure_ascii=False) + "\n")


def run_main(module: object, *arguments: str) -> int:
    with mock.patch.object(sys, "argv", [str(SCRIPT), *arguments]):
        with contextlib.redirect_stdout(io.StringIO()):
            return module.main()


def main() -> int:
    sys.dont_write_bytecode = True
    spec = importlib.util.spec_from_file_location("locale_review_status", SCRIPT)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)

    try:
        module.reject_duplicate_keys([("duplicate", 1), ("duplicate", 2)])
    except ValueError:
        pass
    else:
        raise AssertionError("duplicate JSON key was accepted")

    with tempfile.TemporaryDirectory() as temporary:
        root = Path(temporary)
        module.ROOT = root
        module.STATUS_ROOT = root / "docs" / "localization-reviews"
        module.LOCALES = ("test-locale",)

        (root / "resources" / "locales").mkdir(parents=True)
        with (root / "resources" / "locales" / "inventory.txt").open(
                "w", encoding="utf-8", newline="\n") as stream:
            stream.write("version 1\nen-US source\ntest-locale unreviewed\nqps-ploc pseudo\n")
        source = {"Reviewed": "Source", "Pending": "Pending"}
        translated = {"Reviewed": "Translation", "Pending": "Pending"}
        write_json(root / "resources/locales/en-US/strings.json", source)
        write_json(root / "resources/locales/test-locale/strings.json", translated)
        write_json(root / "docs/localization-invariant-exceptions.json", {"version": 1, "entries": []})

        source_hash = module.digest(source["Reviewed"])
        translation_hash = module.digest(translated["Reviewed"])
        evidence = {
            "date": "2026-11-13",
            "reviewerType": "independent AI",
            "model": "test-model",
            "sessions": ["test-session"],
            "scope": "Synthetic validator fixture",
            "residualUncertainty": "Synthetic evidence only",
            "entries": [{
                "locale": "test-locale", "key": "Reviewed", "verdict": "accept",
                "sourceSha256": source_hash, "translationSha256": translation_hash,
            }],
        }
        evidence_path = module.STATUS_ROOT / "evidence.json"
        write_json(evidence_path, evidence)
        ledger = {
            "version": 1,
            "locale": "test-locale",
            "entries": {
                "Reviewed": {
                    "sourceSha256": source_hash, "sourceStatus": "current",
                    "translationSha256": translation_hash, "translationStatus": "candidate",
                    "reviewStatus": "ai-reviewed",
                    "reviewProvenance": "evidence.json#test-locale/Reviewed",
                },
                "Pending": {
                    "sourceSha256": module.digest("Pending"), "sourceStatus": "current",
                    "translationSha256": module.digest("Pending"), "translationStatus": "untriaged",
                    "reviewStatus": "unreviewed", "reviewProvenance": None,
                },
            },
        }
        ledger_path = module.STATUS_ROOT / "test-locale.json"
        write_json(ledger_path, ledger)

        assert run_main(module) == 0
        assert run_main(module, "--release") == 1

        translated["Pending"] = "Translated pending"
        write_json(root / "resources/locales/test-locale/strings.json", translated)
        pending_source_hash = module.digest(source["Pending"])
        pending_translation_hash = module.digest(translated["Pending"])
        evidence["entries"].append({
            "locale": "test-locale", "key": "Pending", "verdict": "accept",
            "sourceSha256": pending_source_hash,
            "translationSha256": pending_translation_hash,
        })
        write_json(evidence_path, evidence)
        ledger["entries"]["Pending"] = {
            "sourceSha256": pending_source_hash, "sourceStatus": "current",
            "translationSha256": pending_translation_hash, "translationStatus": "candidate",
            "reviewStatus": "ai-reviewed",
            "reviewProvenance": "evidence.json#test-locale/Pending",
        }
        write_json(ledger_path, ledger)
        assert run_main(module, "--release") == 0

        evidence_without_model = dict(evidence)
        evidence_without_model.pop("model")
        write_json(evidence_path, evidence_without_model)
        try:
            run_main(module)
        except ValueError:
            pass
        else:
            raise AssertionError("AI evidence without model metadata was accepted")
        write_json(evidence_path, evidence)

        evidence["entries"].append(dict(evidence["entries"][0]))
        write_json(evidence_path, evidence)
        assert run_main(module) == 1
        evidence["entries"].pop()

        evidence["reviewerType"] = "human"
        ledger["entries"]["Reviewed"]["reviewStatus"] = "human-reviewed"
        ledger["entries"]["Pending"]["reviewStatus"] = "human-reviewed"
        write_json(evidence_path, evidence)
        write_json(ledger_path, ledger)
        try:
            run_main(module)
        except ValueError:
            pass
        else:
            raise AssertionError("human evidence without reviewer metadata was accepted")
        evidence["reviewerType"] = "independent AI"
        ledger["entries"]["Reviewed"]["reviewStatus"] = "ai-reviewed"
        ledger["entries"]["Pending"]["reviewStatus"] = "ai-reviewed"
        write_json(evidence_path, evidence)
        write_json(ledger_path, ledger)

        ledger["entries"]["Reviewed"]["sourceSha256"] = "0" * 64
        write_json(ledger_path, ledger)
        assert run_main(module) == 1
        ledger["entries"]["Reviewed"]["sourceSha256"] = source_hash

        ledger["entries"]["Reviewed"]["reviewProvenance"] = "../evidence.json#test-locale/Reviewed"
        write_json(ledger_path, ledger)
        assert run_main(module) == 1
        ledger["entries"]["Reviewed"]["reviewProvenance"] = "evidence.json#test-locale/Reviewed"
        write_json(ledger_path, ledger)

        evidence.pop("residualUncertainty")
        write_json(evidence_path, evidence)
        try:
            run_main(module)
        except ValueError:
            pass
        else:
            raise AssertionError("evidence without residual uncertainty was accepted")

        with (root / "resources/locales/inventory.txt").open(
                "w", encoding="utf-8", newline="\n") as stream:
            stream.write("version 1\nmalformed inventory entry with extra fields\n")
        try:
            module.review_locales()
        except ValueError:
            pass
        else:
            raise AssertionError("malformed inventory entry was accepted")

    print("locale review negative paths: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
