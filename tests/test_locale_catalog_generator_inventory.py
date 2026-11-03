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
        inventory = (repository / "resources" / "locales" / "inventory.txt").read_text(encoding="utf-8")
        declared = {line.split()[0] for line in inventory.splitlines()[1:]}
        for locale in declared:
            shutil.copytree(repository / "resources" / "locales" / locale, locales / locale)
        managed_source = root / "vsix" / "Copperfin.VisualStudio"
        managed_source.mkdir(parents=True)
        shutil.copy2(
            repository / "vsix" / "Copperfin.VisualStudio" / "CopperfinLocalization.cs",
            managed_source / "CopperfinLocalization.cs",
        )
        synthetic_index = 1
        while f"zz-{synthetic_index:03d}" in declared:
            synthetic_index += 1
        synthetic = f"zz-{synthetic_index:03d}"
        expanded_inventory = inventory.replace(
            "qps-ploc pseudo\n", f"{synthetic} unreviewed\nqps-ploc pseudo\n")
        if expanded_inventory == inventory:
            raise AssertionError("Inventory has no qps-ploc insertion point")
        (locales / "inventory.txt").write_text(
            expanded_inventory, encoding="utf-8"
        )
        shutil.copytree(locales / "en-US", locales / synthetic)
        path = locales / synthetic / "strings.json"
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
        overlay_locale = f"{synthetic}-DE"
        overlay_inventory = expanded_inventory.replace(
            "qps-ploc pseudo\n", f"{overlay_locale} overlay {synthetic}\nqps-ploc pseudo\n")
        (locales / "inventory.txt").write_text(overlay_inventory, encoding="utf-8")
        overlay_path = locales / overlay_locale / "strings.json"
        overlay_path.parent.mkdir()
        overlay_path.write_text('{"Studio.OpenDialogTitle":"Öffnen"}\n', encoding="utf-8")
        check(True, "sparse regional overlay")
        overlay_path.write_text('{"Studio.OpenDialogTitle":"  "}\n', encoding="utf-8")
        check(False, "blank overlay entry", "Catalog key is blank")
        overlay_path.write_text('{"Studio.OpenDocumentStatus":"{wrong}"}\n', encoding="utf-8")
        check(False, "overlay placeholder drift", "Catalog placeholder drift")
        overlay_path.write_text('{"Unexpected.Key":"value"}\n', encoding="utf-8")
        check(False, "overlay extra key", "Catalog contains undeclared key")
        overlay_path.write_text('{}\n', encoding="utf-8")
        check(True, "empty regional overlay inherits parent")
        (locales / "inventory.txt").write_text(
            overlay_inventory.replace(f"overlay {synthetic}", "overlay en-US"), encoding="utf-8")
        check(False, "cross-language overlay parent", "Overlay parent must be a previously declared runtime fallback ancestor")
        sibling_inventory = overlay_inventory.replace(
            f"{overlay_locale} overlay {synthetic}\n",
            f"{synthetic}-AT unreviewed\n{overlay_locale} overlay {synthetic}-AT\n")
        (locales / "inventory.txt").write_text(sibling_inventory, encoding="utf-8")
        check(False, "sibling regional parent", "Overlay parent must be a previously declared runtime fallback ancestor")
        singleton_inventory = overlay_inventory.replace(
            f"{overlay_locale} overlay {synthetic}\n",
            f"{synthetic}-u unreviewed\n{synthetic}-u-co overlay {synthetic}-u\n")
        (locales / "inventory.txt").write_text(singleton_inventory, encoding="utf-8")
        check(False, "skipped extension parent", "Overlay parent must be a previously declared runtime fallback ancestor")
        for regional in ("es-MX", "es-Latn-MX"):
            regional_path = locales / regional / "strings.json"
            regional_path.parent.mkdir(exist_ok=True)
            regional_path.write_text('{}\n', encoding="utf-8")
            regional_inventory = overlay_inventory.replace(
                "qps-ploc pseudo\n", f"{regional} overlay es-419\nqps-ploc pseudo\n")
            (locales / "inventory.txt").write_text(regional_inventory, encoding="utf-8")
            check(True, f"Latin American {regional} parent")
            shutil.rmtree(regional_path.parent)
        portuguese_inventory = overlay_inventory.replace(
            "qps-ploc pseudo\n", "pt overlay pt-BR\nqps-ploc pseudo\n")
        (locales / "inventory.txt").write_text(portuguese_inventory, encoding="utf-8")
        check(False, "neutral Portuguese cannot overlay Brazil", "Overlay parent must be a previously declared runtime fallback ancestor")
        chained_inventory = overlay_inventory.replace(
            "qps-ploc pseudo\n", f"{overlay_locale}-AT overlay {overlay_locale}\nqps-ploc pseudo\n")
        (locales / "inventory.txt").write_text(chained_inventory, encoding="utf-8")
        check(False, "chained regional overlay", "Overlay parent must be a previously declared runtime fallback ancestor")
        (locales / "inventory.txt").write_text(expanded_inventory, encoding="utf-8")
        shutil.rmtree(overlay_path.parent)
        missing = dict(complete)
        missing.pop("Studio.OpenDialogTitle")
        path.write_text(json.dumps(missing, ensure_ascii=False), encoding="utf-8")
        check(False, "missing fifth-locale key", "Catalog key count differs from en-US")
        blank = dict(complete)
        blank["Studio.OpenDialogTitle"] = "  "
        path.write_text(json.dumps(blank, ensure_ascii=False), encoding="utf-8")
        check(False, "blank fifth-locale value", "Catalog key is blank")
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
        (locales / "inventory.txt").write_text(expanded_inventory, encoding="utf-8")
        (locales / "stray.txt").write_text("unexpected", encoding="utf-8")
        check(False, "undeclared locale-root file", "Locale root files do not match inventory contract")
    print("Synthetic fifth-locale generator checks passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
