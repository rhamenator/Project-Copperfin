#!/usr/bin/env python3
"""Negative-path tests for the locale review evidence validator."""

from __future__ import annotations

import contextlib
import errno
import importlib.util
import io
import json
import os
import signal
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


def expect_rejected(module: object, *arguments: str) -> None:
    alarm_supported = hasattr(signal, "SIGALRM") and hasattr(signal, "setitimer")
    previous_handler = None
    if alarm_supported:
        def timeout_handler(_signal: int, _frame: object) -> None:
            raise TimeoutError("unsafe review-status path check timed out")

        previous_handler = signal.signal(signal.SIGALRM, timeout_handler)
        signal.setitimer(signal.ITIMER_REAL, 2.0)
    try:
        try:
            result = run_main(module, *arguments)
        except TimeoutError as error:
            raise AssertionError(str(error)) from error
        except (OSError, ValueError) as error:
            message = str(error).lower()
            expected_messages = (
                "must be a regular file",
                "unable to open regular",
                "review-status directory must",
                "unable to open managed review-status directory",
            )
            assert any(expected in message for expected in expected_messages), (
                f"unexpected rejection: {error}")
            return
        raise AssertionError(
            f"unsafe review-status file returned {result} instead of a path rejection for {arguments}")
    finally:
        if alarm_supported:
            signal.setitimer(signal.ITIMER_REAL, 0)
            signal.signal(signal.SIGALRM, previous_handler)


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

        with tempfile.TemporaryDirectory() as external_temporary:
            external_root = Path(external_temporary)
            outside_ledger = external_root / "outside-ledger.json"
            write_json(outside_ledger, {**ledger, "probeMarker": "preserve"})
            outside_before = outside_ledger.read_bytes()
            try:
                ledger_path.unlink()
                ledger_path.symlink_to(outside_ledger)
            except OSError:
                write_json(ledger_path, ledger)
            else:
                expect_rejected(module)
                expect_rejected(module, "--write")
                assert outside_ledger.read_bytes() == outside_before
                ledger_path.unlink()
                write_json(ledger_path, ledger)

            outside_evidence = external_root / "outside-evidence.json"
            write_json(outside_evidence, evidence)
            outside_evidence_before = outside_evidence.read_bytes()
            try:
                evidence_path.unlink()
                evidence_path.symlink_to(outside_evidence)
            except OSError:
                write_json(evidence_path, evidence)
            else:
                expect_rejected(module)
                expect_rejected(module, "--write")
                assert outside_evidence.read_bytes() == outside_evidence_before
                evidence_path.unlink()
                write_json(evidence_path, evidence)

            if hasattr(os, "mkfifo"):
                ledger_path.unlink()
                os.mkfifo(ledger_path)
                expect_rejected(module)
                expect_rejected(module, "--write")
                ledger_path.unlink()

                fifo_target = external_root / "outside-ledger.fifo"
                os.mkfifo(fifo_target)
                try:
                    ledger_path.symlink_to(fifo_target)
                except OSError:
                    write_json(ledger_path, ledger)
                else:
                    expect_rejected(module)
                    expect_rejected(module, "--write")
                    ledger_path.unlink()
                    write_json(ledger_path, ledger)

                evidence_path.unlink()
                os.mkfifo(evidence_path)
                expect_rejected(module)
                expect_rejected(module, "--write")
                evidence_path.unlink()
                write_json(evidence_path, evidence)

            original_directory_fd_support = module.HAS_DIRECTORY_FDS
            module.HAS_DIRECTORY_FDS = False
            try:
                ledger_path.unlink()
                try:
                    ledger_path.symlink_to(outside_ledger)
                except OSError:
                    write_json(ledger_path, ledger)
                else:
                    expect_rejected(module)
                    expect_rejected(module, "--write")
                    assert outside_ledger.read_bytes() == outside_before
                    ledger_path.unlink()
                    write_json(ledger_path, ledger)

                evidence_path.unlink()
                try:
                    evidence_path.symlink_to(outside_evidence)
                except OSError:
                    write_json(evidence_path, evidence)
                else:
                    expect_rejected(module)
                    expect_rejected(module, "--write")
                    assert outside_evidence.read_bytes() == outside_evidence_before
                    evidence_path.unlink()
                    write_json(evidence_path, evidence)

                if hasattr(os, "mkfifo"):
                    ledger_path.unlink()
                    os.mkfifo(ledger_path)
                    expect_rejected(module)
                    expect_rejected(module, "--write")
                    ledger_path.unlink()
                    write_json(ledger_path, ledger)

                    evidence_path.unlink()
                    os.mkfifo(evidence_path)
                    expect_rejected(module)
                    expect_rejected(module, "--write")
                    evidence_path.unlink()
                    write_json(evidence_path, evidence)

                assert run_main(module) == 0
                assert run_main(module, "--write") == 0
                assert b"\r\n" not in ledger_path.read_bytes()

                real_open = os.open
                real_replace = os.replace
                tracked_temporary = {}

                def track_temporary_open(path: object, flags: int, mode: int = 0o777,
                                         *, dir_fd: object = None) -> int:
                    if dir_fd is None:
                        descriptor = real_open(path, flags, mode)
                    else:
                        descriptor = real_open(path, flags, mode, dir_fd=dir_fd)
                    if ".tmp" in Path(path).name:
                        tracked_temporary["descriptor"] = descriptor
                    return descriptor

                def require_closed_before_replace(source: object, destination: object,
                                                   **keywords: object) -> None:
                    descriptor = tracked_temporary.get("descriptor")
                    assert descriptor is not None
                    try:
                        os.fstat(descriptor)
                    except OSError as error:
                        assert error.errno == errno.EBADF
                    else:
                        raise AssertionError("temporary ledger was still open during replacement")
                    real_replace(source, destination, **keywords)

                with mock.patch.object(module.os, "open", track_temporary_open), \
                        mock.patch.object(module.os, "replace", require_closed_before_replace):
                    module.write_ledger(ledger_path, ledger)
            finally:
                module.HAS_DIRECTORY_FDS = original_directory_fd_support

            relocated_status = root / "docs" / "localization-reviews-real"
            write_json(external_root / "test-locale.json", ledger)
            write_json(external_root / "evidence.json", evidence)
            outside_directory_ledger = (external_root / "test-locale.json").read_bytes()
            module.STATUS_ROOT.rename(relocated_status)
            try:
                module.STATUS_ROOT.symlink_to(external_root, target_is_directory=True)
            except OSError:
                relocated_status.rename(module.STATUS_ROOT)
            else:
                expect_rejected(module)
                expect_rejected(module, "--write")
                module.HAS_DIRECTORY_FDS = False
                try:
                    expect_rejected(module)
                    expect_rejected(module, "--write")
                finally:
                    module.HAS_DIRECTORY_FDS = original_directory_fd_support
                assert (external_root / "test-locale.json").read_bytes() == outside_directory_ledger
                module.STATUS_ROOT.unlink()
                relocated_status.rename(module.STATUS_ROOT)

            if original_directory_fd_support:
                real_open_status_root = module.open_status_root

                def open_then_rebind() -> int:
                    descriptor = real_open_status_root()
                    module.STATUS_ROOT.rename(relocated_status)
                    module.STATUS_ROOT.symlink_to(external_root, target_is_directory=True)
                    return descriptor

                with mock.patch.object(module, "open_status_root", open_then_rebind):
                    anchored = module.load_status_object(ledger_path, "Review ledger")
                module.STATUS_ROOT.unlink()
                relocated_status.rename(module.STATUS_ROOT)
                assert anchored["locale"] == "test-locale"

                outside_directory_ledger = (external_root / "test-locale.json").read_bytes()
                with mock.patch.object(module, "open_status_root", open_then_rebind):
                    module.write_ledger(ledger_path, ledger)
                module.STATUS_ROOT.unlink()
                relocated_status.rename(module.STATUS_ROOT)
                assert (external_root / "test-locale.json").read_bytes() == outside_directory_ledger

        ledger_mode = ledger_path.stat().st_mode & 0o777
        assert run_main(module, "--write") == 0
        assert ledger_path.stat().st_mode & 0o777 == ledger_mode
        assert b"\r\n" not in ledger_path.read_bytes()
        assert run_main(module) == 0

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
