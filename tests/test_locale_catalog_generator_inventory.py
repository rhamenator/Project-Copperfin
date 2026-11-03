#!/usr/bin/env python3
"""Exercise #7072 catalog discovery with a synthetic fifth locale."""

import json
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


def main() -> int:
    if len(sys.argv) != 5:
        raise SystemExit("usage: test_locale_catalog_generator_inventory.py DOTNET PROJECT REPO CONFIG")
    dotnet, project, repository, configuration = sys.argv[1:]
    repository = Path(repository)
    with tempfile.TemporaryDirectory(prefix="copperfin-locale-inventory-") as temporary:
        root = Path(temporary)
        locales = root / "resources" / "locales"
        locales.mkdir(parents=True)
        for locale in ("en-US", "es-419", "pt-BR", "qps-ploc"):
            shutil.copytree(repository / "resources" / "locales" / locale, locales / locale)
        managed_source = root / "vsix" / "Copperfin.VisualStudio"
        managed_source.mkdir(parents=True)
        shutil.copy2(
            repository / "vsix" / "Copperfin.VisualStudio" / "CopperfinLocalization.cs",
            managed_source / "CopperfinLocalization.cs",
        )
        inventory = (repository / "resources" / "locales" / "inventory.txt").read_text(encoding="utf-8")
        (locales / "inventory.txt").write_text(
            inventory.replace("qps-ploc pseudo\n", "fr-FR unreviewed\nqps-ploc pseudo\n"), encoding="utf-8"
        )
        shutil.copytree(locales / "en-US", locales / "fr-FR")
        path = locales / "fr-FR" / "strings.json"
        complete = json.loads(path.read_text(encoding="utf-8"))

        def check(expected_success: bool, label: str, expected_error: str = "") -> None:
            result = subprocess.run(
                [dotnet, "run", "--project", project, "--configuration", configuration,
                 "--no-build", "--", str(root), "--check"],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.PIPE,
                text=True,
                encoding="utf-8",
                errors="replace",
                check=False,
            )
            if (result.returncode == 0) != expected_success:
                raise AssertionError(f"{label}: unexpected generator result {result.returncode}: {result.stderr[:500]}")
            if not expected_success and expected_error not in result.stderr:
                raise AssertionError(f"{label}: wrong failure: {result.stderr[:500]}")

        check(True, "complete fifth locale")
        missing = dict(complete)
        missing.pop("Studio.OpenDialogTitle")
        path.write_text(json.dumps(missing, ensure_ascii=False), encoding="utf-8")
        check(False, "missing fifth-locale key", "Catalog key count differs from en-US")
        blank = dict(complete)
        blank["Studio.OpenDialogTitle"] = "  "
        path.write_text(json.dumps(blank, ensure_ascii=False), encoding="utf-8")
        check(False, "blank fifth-locale value", "Catalog key is missing or blank")
        placeholder = dict(complete)
        placeholder["Studio.OpenDocumentStatus"] = "Bad {unexpected} placeholder"
        path.write_text(json.dumps(placeholder, ensure_ascii=False), encoding="utf-8")
        check(False, "changed fifth-locale placeholder", "Catalog placeholder drift")
        path.write_bytes(b"\xff")
        check(False, "invalid UTF-8 fifth-locale catalog", "DecoderFallbackException")
        path.write_bytes(b"\xff\xfe{\x00}\x00")
        check(False, "UTF-16 fifth-locale catalog", "DecoderFallbackException")
        path.write_text(json.dumps(complete, ensure_ascii=False), encoding="utf-8")
        (locales / "inventory.txt").write_text(inventory, encoding="utf-8")
        check(False, "undeclared fifth-locale directory", "Locale directories do not match")
        (locales / "inventory.txt").write_text(
            inventory.replace("qps-ploc pseudo\n", "fr-FR unreviewed\nqps-ploc pseudo\n"), encoding="utf-8")
        (locales / "stray.txt").write_text("unexpected", encoding="utf-8")
        check(False, "undeclared locale-root file", "Locale root files do not match inventory contract")
    print("Synthetic fifth-locale generator checks passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
