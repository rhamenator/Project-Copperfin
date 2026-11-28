#!/usr/bin/env python3
"""Exercise #7072 catalog discovery with a synthetic fifth locale."""

import json
import os
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
        (locales / "inventory.txt").write_text(inventory, encoding="utf-8")
        for locale in declared:
            shutil.copytree(repository / "resources" / "locales" / locale, locales / locale)
        managed_source = root / "vsix" / "Copperfin.VisualStudio"
        managed_source.mkdir(parents=True)
        shutil.copy2(
            repository / "vsix" / "Copperfin.VisualStudio" / "CopperfinLocalization.cs",
            managed_source / "CopperfinLocalization.cs",
        )

        def check(expected_success: bool, label: str, expected_error: str = "",
                  check_only: bool = True) -> None:
            command = [dotnet, "run", "--project", project, "--configuration", configuration,
                       "--no-build", "--", str(root)]
            if check_only:
                command.append("--check")
            try:
                result = subprocess.run(
                    command,
                    stdout=subprocess.DEVNULL,
                    stderr=subprocess.PIPE,
                    text=True,
                    encoding="utf-8",
                    errors="replace",
                    check=False,
                    timeout=30,
                )
            except subprocess.TimeoutExpired as error:
                raise AssertionError(f"{label}: generator exceeded the bounded timeout") from error
            if (result.returncode == 0) != expected_success:
                raise AssertionError(
                    f"{label}: unexpected generator result {result.returncode}: {result.stderr[:500]}")
            if not expected_success and expected_error not in result.stderr:
                raise AssertionError(f"{label}: wrong failure: {result.stderr[:500]}")

        inventory_path = locales / "inventory.txt"
        managed_source_path = managed_source / "CopperfinLocalization.cs"
        catalog_path = locales / "es-419" / "strings.json"
        regular_catalog = json.loads(catalog_path.read_text(encoding="utf-8"))
        regular_catalog.pop("Studio.OpenDialogTitle")
        catalog_path.write_text(
            json.dumps(regular_catalog, ensure_ascii=False), encoding="utf-8")
        if os.name != "nt":
            catalog_path.chmod(0o640)
        original_mode = catalog_path.stat().st_mode & 0o777
        check(True, "regular catalog generation", check_only=False)
        generated_catalog = json.loads(catalog_path.read_text(encoding="utf-8"))
        if "Studio.OpenDialogTitle" not in generated_catalog:
            raise AssertionError("regular catalog generation did not restore the managed key")
        if catalog_path.stat().st_mode & 0o777 != original_mode:
            raise AssertionError("regular catalog generation changed the catalog mode")
        if list(catalog_path.parent.glob(".strings.json.*.tmp")):
            raise AssertionError("regular catalog generation retained a temporary file")

        with tempfile.TemporaryDirectory(prefix="copperfin-locale-outside-") as outside_temporary:
            outside = Path(outside_temporary)
            outside_inventory = outside / "inventory.txt"
            outside_source = outside / "CopperfinLocalization.cs"
            outside_catalog = outside / "strings.json"
            shutil.copy2(inventory_path, outside_inventory)
            shutil.copy2(managed_source_path, outside_source)
            shutil.copy2(catalog_path, outside_catalog)

            def reject_link(path: Path, target: Path, label: str) -> None:
                original = target.read_bytes()
                path.unlink()
                try:
                    path.symlink_to(target)
                except OSError as error:
                    if os.name != "nt":
                        raise AssertionError(
                            f"{label}: symlink fixture creation failed unexpectedly") from error
                    print(f"Skipping {label} symlink fixture: {error}", file=sys.stderr)
                    shutil.copy2(target, path)
                    return
                check(False, f"linked {label} check", "regular file")
                check(False, f"linked {label} generation", "regular file", check_only=False)
                if target.read_bytes() != original:
                    raise AssertionError(f"linked {label} target was modified")
                path.unlink()
                shutil.copy2(target, path)

            reject_link(inventory_path, outside_inventory, "inventory")
            reject_link(managed_source_path, outside_source, "managed source")
            reject_link(catalog_path, outside_catalog, "catalog")

            missing_catalog = json.loads(outside_catalog.read_text(encoding="utf-8"))
            missing_catalog.pop("Studio.OpenDialogTitle")
            outside_catalog.write_text(
                json.dumps(missing_catalog, ensure_ascii=False), encoding="utf-8")
            reject_link(catalog_path, outside_catalog, "incomplete catalog")

            if os.name != "nt":
                shutil.copy2(
                    repository / "resources/locales/es-419/strings.json", catalog_path)
                read_only_catalog = json.loads(catalog_path.read_text(encoding="utf-8"))
                read_only_catalog.pop("Studio.OpenDialogTitle")
                catalog_path.write_text(
                    json.dumps(read_only_catalog, ensure_ascii=False), encoding="utf-8")
                catalog_path.chmod(0o440)
                try:
                    check(False, "read-only catalog generation", "read-only", check_only=False)
                finally:
                    catalog_path.chmod(0o640)
                check(True, "writable catalog generation", check_only=False)

            if hasattr(os, "mkfifo"):
                inventory_path.unlink()
                os.mkfifo(inventory_path)
                check(False, "FIFO inventory check", "regular file")
                check(False, "FIFO inventory generation", "regular file", check_only=False)
                inventory_path.unlink()
                shutil.copy2(outside_inventory, inventory_path)

                catalog_path.unlink()
                os.mkfifo(catalog_path)
                check(False, "FIFO catalog check", "regular file")
                check(False, "FIFO catalog generation", "regular file", check_only=False)
                catalog_path.unlink()
                shutil.copy2(repository / "resources/locales/es-419/strings.json", catalog_path)

            catalog_path.unlink()
            catalog_path.mkdir()
            check(False, "directory catalog check", "regular file")
            check(False, "directory catalog generation", "regular file", check_only=False)
            catalog_path.rmdir()
            shutil.copy2(repository / "resources/locales/es-419/strings.json", catalog_path)

            outside_locales = outside / "linked-locales"
            shutil.copytree(locales, outside_locales)
            outside_linked_catalog = outside_locales / "es-419" / "strings.json"
            linked_catalog_data = json.loads(outside_linked_catalog.read_text(encoding="utf-8"))
            linked_catalog_data.pop("Studio.OpenDialogTitle")
            outside_linked_catalog.write_text(
                json.dumps(linked_catalog_data, ensure_ascii=False), encoding="utf-8")
            outside_linked_bytes = outside_linked_catalog.read_bytes()
            saved_locales = root / "saved-locales"
            locales.rename(saved_locales)
            try:
                try:
                    locales.symlink_to(outside_locales, target_is_directory=True)
                except OSError as error:
                    if os.name != "nt":
                        raise AssertionError(
                            "locale-root symlink fixture creation failed unexpectedly") from error
                    print(f"Skipping locale-root symlink fixture: {error}", file=sys.stderr)
                else:
                    check(False, "linked locale-root check", "ancestor")
                    check(False, "linked locale-root generation", "ancestor", check_only=False)
                    if outside_linked_catalog.read_bytes() != outside_linked_bytes:
                        raise AssertionError("linked locale-root catalog was modified")
                    locales.unlink()
            finally:
                if locales.is_symlink():
                    locales.unlink()
                if not locales.exists():
                    saved_locales.rename(locales)

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

        check(True, "complete fifth locale")
        overlay_locale = f"{synthetic}-abcde"
        overlay_inventory = expanded_inventory.replace(
            "qps-ploc pseudo\n", f"{overlay_locale} overlay {synthetic}\nqps-ploc pseudo\n")
        (locales / "inventory.txt").write_text(overlay_inventory, encoding="utf-8")
        overlay_path = locales / overlay_locale / "strings.json"
        overlay_path.parent.mkdir()
        overlay_path.write_text('{"Studio.OpenDialogTitle":"Öffnen"}\n', encoding="utf-8")
        check(True, "sparse variant overlay")
        overlay_path.write_text('{"Studio.OpenDialogTitle":"  "}\n', encoding="utf-8")
        check(False, "blank overlay entry", "Catalog key is blank")
        overlay_path.write_text('{"Studio.OpenDocumentStatus":"{wrong}"}\n', encoding="utf-8")
        check(False, "overlay placeholder drift", "Catalog placeholder drift")
        overlay_path.write_text('{"Unexpected.Key":"value"}\n', encoding="utf-8")
        check(False, "overlay extra key", "Catalog contains undeclared key")
        overlay_path.write_text('{}\n', encoding="utf-8")
        check(True, "empty variant overlay inherits parent")
        (locales / "inventory.txt").write_text(
            overlay_inventory.replace(f"overlay {synthetic}", "overlay en-US"), encoding="utf-8")
        check(False, "cross-language overlay parent", "Overlay parent must be a previously declared runtime fallback ancestor")
        sibling_inventory = overlay_inventory.replace(
            f"{overlay_locale} overlay {synthetic}\n",
            f"{synthetic}-abcdf unreviewed\n{overlay_locale} overlay {synthetic}-abcdf\n")
        (locales / "inventory.txt").write_text(sibling_inventory, encoding="utf-8")
        check(False, "sibling variant parent", "Overlay parent must be a previously declared runtime fallback ancestor")
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
            "qps-ploc pseudo\n", f"{overlay_locale}-fghij overlay {overlay_locale}\nqps-ploc pseudo\n")
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

        external_catalog = locales / "en-GB" / "strings.json"
        external_before = external_catalog.read_bytes()
        generated_key = "Studio.OpenDialogTitle"
        for managed_locale in ("es-419", "pt-BR"):
            managed_path = locales / managed_locale / "strings.json"
            managed_values = json.loads(managed_path.read_text(encoding="utf-8"))
            managed_values.pop(generated_key)
            managed_path.write_text(
                json.dumps(managed_values, ensure_ascii=False), encoding="utf-8"
            )
        write_mode = subprocess.run(
            [dotnet, "run", "--project", project, "--configuration", configuration,
             "--no-build", "--", str(root)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            encoding="utf-8",
            errors="replace",
            check=False,
        )
        if write_mode.returncode != 0:
            raise AssertionError(
                f"write mode should skip external-only catalogs and sync later managed catalogs: "
                f"{write_mode.stderr[:500]}"
            )
        if external_catalog.read_bytes() != external_before:
            raise AssertionError("write mode must preserve external-only catalog bytes")
        for managed_locale in ("es-419", "pt-BR"):
            managed_values = json.loads(
                (locales / managed_locale / "strings.json").read_text(encoding="utf-8")
            )
            if generated_key not in managed_values:
                raise AssertionError(f"write mode did not synchronize {managed_locale}")

        (locales / "inventory.txt").write_text(inventory, encoding="utf-8")
        check(False, "undeclared fifth-locale directory", "Locale directories do not match")
        (locales / "inventory.txt").write_text(expanded_inventory, encoding="utf-8")
        (locales / synthetic / ".note").write_text("unexpected", encoding="utf-8")
        check(False, "hidden per-locale file", "Locale directory content does not match inventory contract")
        (locales / synthetic / ".note").unlink()
        (locales / "stray.txt").write_text("unexpected", encoding="utf-8")
        check(False, "undeclared locale-root file", "Locale root files do not match inventory contract")
    print("Synthetic fifth-locale generator checks passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
