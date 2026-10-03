#!/usr/bin/env bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# Traceability: RQ-CF-REL-007; DQ-macos-installer-lifecycle-scope;
# DV-macos-installer-lifecycle-contract; HZ-system-failure-01;
# HZ-data-corruption-01; HZ-doc-command-01.

set -euo pipefail

usage() {
    printf 'Usage: %s --package <pkg> --package-version <version> --product-identifier <identifier> --evidence-directory <directory> --allow-system-mutation [--process-timeout-seconds <seconds>]\n' "$0" >&2
}

package_path=""
package_version=""
product_identifier=""
evidence_directory=""
process_timeout_seconds=180
allow_system_mutation=0
while (($# > 0)); do
    case "$1" in
        --package)
            package_path=${2-}
            shift 2
            ;;
        --package-version)
            package_version=${2-}
            shift 2
            ;;
        --product-identifier)
            product_identifier=${2-}
            shift 2
            ;;
        --evidence-directory)
            evidence_directory=${2-}
            shift 2
            ;;
        --process-timeout-seconds)
            process_timeout_seconds=${2-}
            shift 2
            ;;
        --allow-system-mutation)
            allow_system_mutation=1
            shift
            ;;
        *)
            usage
            exit 2
            ;;
    esac
done

if [[ -z "$package_path" || -z "$package_version" || -z "$product_identifier" \
    || -z "$evidence_directory" || ! "$process_timeout_seconds" =~ ^[1-9][0-9]*$ \
    || "$allow_system_mutation" != 1 ]]; then
    usage
    exit 2
fi
if [[ ${GITHUB_ACTIONS:-} != "true" || ${RUNNER_ENVIRONMENT:-} != "github-hosted" ]]; then
    printf 'macOS installer lifecycle validation is restricted to a GitHub-hosted Actions runner.\n' >&2
    exit 1
fi
if ((EUID != 0)); then
    printf 'macOS installer lifecycle validation must run as root.\n' >&2
    exit 1
fi
if [[ $(uname -s) != "Darwin" ]]; then
    printf 'macOS installer lifecycle validation requires Darwin.\n' >&2
    exit 1
fi
if [[ ! "$product_identifier" =~ ^[A-Za-z0-9][A-Za-z0-9.-]*$ ]]; then
    printf 'Product identifier is invalid: %s\n' "$product_identifier" >&2
    exit 1
fi

for command_name in awk dirname find grep installer mktemp pkgutil plutil python3 \
    rmdir shasum sort tee uname; do
    if ! command -v "$command_name" >/dev/null 2>&1; then
        printf 'Required command is unavailable: %s\n' "$command_name" >&2
        exit 1
    fi
done

resolve_existing_path() {
    python3 - "$1" <<'PY'
import pathlib
import sys
print(pathlib.Path(sys.argv[1]).resolve(strict=True))
PY
}

if [[ -L "$package_path" || ! -f "$package_path" ]]; then
    printf 'Package must be an existing regular file: %s\n' "$package_path" >&2
    exit 1
fi
if [[ -L "$evidence_directory" || ! -d "$evidence_directory" ]]; then
    printf 'Evidence directory must be an existing regular directory: %s\n' "$evidence_directory" >&2
    exit 1
fi
package_path=$(resolve_existing_path "$package_path")
evidence_directory=$(resolve_existing_path "$evidence_directory")

runner_temp=${RUNNER_TEMP:-}
if [[ -z "$runner_temp" || -L "$runner_temp" || ! -d "$runner_temp" ]]; then
    printf 'RUNNER_TEMP must name an existing regular directory.\n' >&2
    exit 1
fi
runner_temp=$(resolve_existing_path "$runner_temp")

component_receipts=(
    "${product_identifier}.Documentation"
    "${product_identifier}.Unspecified"
)
for receipt in "${component_receipts[@]}"; do
    if pkgutil --pkg-info "$receipt" >/dev/null 2>&1; then
        printf 'Refusing to disturb a pre-existing package receipt: %s\n' "$receipt" >&2
        exit 1
    fi
done

fixture_root=$(mktemp -d "$runner_temp/copperfin-macos-installer-lifecycle.XXXXXX")
fixture_root=$(resolve_existing_path "$fixture_root")
case "$fixture_root" in
    "$runner_temp"/copperfin-macos-installer-lifecycle.*) ;;
    *)
        printf 'Fixture directory escaped RUNNER_TEMP: %s\n' "$fixture_root" >&2
        exit 1
        ;;
esac

payload_paths_raw="$fixture_root/payload-paths.raw.txt"
payload_paths="$fixture_root/payload-paths.txt"
created_directories="$fixture_root/created-directories.txt"
receipt_paths="$fixture_root/receipt-paths.txt"
inspect_stdout="$fixture_root/copperfin-inspect.stdout"
external_fixture="$fixture_root/external-fixture.prg"

pkgutil --payload-files "$package_path" >"$payload_paths_raw"
python3 - "$payload_paths_raw" "$payload_paths" <<'PY'
from pathlib import PurePosixPath
import sys

source, destination = sys.argv[1:]
normalized = set()
for raw in open(source, encoding="utf-8"):
    entry = raw.rstrip("\r\n")
    while entry.startswith("./"):
        entry = entry[2:]
    if not entry or entry == ".":
        continue
    path = PurePosixPath(entry)
    if (
        path.is_absolute()
        or ".." in path.parts
        or not (entry == "Applications" or entry.startswith("Applications/"))
    ):
        raise SystemExit(f"Unsafe productbuild payload path: {raw.rstrip()}")
    normalized.add(path.as_posix())
if not normalized:
    raise SystemExit("The productbuild package exposes no payload paths")
with open(destination, "w", encoding="utf-8", newline="\n") as output:
    for entry in sorted(normalized):
        output.write(entry + "\n")
PY

expected_installed_files=(
    /Applications/bin/copperfin_build_host
    /Applications/bin/copperfin_inspect
    /Applications/bin/copperfin_mcp_host
    /Applications/bin/copperfin_runtime_host
    /Applications/bin/copperfin_studio_host
    /Applications/share/copperfin/locales/en-US/strings.json
)
for expected_path in "${expected_installed_files[@]}"; do
    relative_path=${expected_path#/}
    if ! grep -Fqx "$relative_path" "$payload_paths"; then
        printf 'Package payload omits expected path: %s\n' "$expected_path" >&2
        exit 1
    fi
done
if [[ -e /Applications/share/copperfin || -L /Applications/share/copperfin ]]; then
    printf 'Refusing to overwrite a pre-existing package-specific directory.\n' >&2
    exit 1
fi

: >"$created_directories"
while IFS= read -r relative_path; do
    installed_path="/$relative_path"
    if [[ -d "$installed_path" && ! -L "$installed_path" ]]; then
        continue
    fi
    if [[ -e "$installed_path" || -L "$installed_path" ]]; then
        printf 'Refusing to overwrite a pre-existing package path: %s\n' "$installed_path" >&2
        exit 1
    fi
    case "$relative_path" in
        */*)
            parent_directory=$(dirname "$installed_path")
            if [[ ! -d "$parent_directory" && ! -L "$parent_directory" ]]; then
                printf '%s\n' "$parent_directory" >>"$created_directories"
            fi
            ;;
    esac
done <"$payload_paths"
LC_ALL=C sort -u -o "$created_directories" "$created_directories"

run_bounded() {
    python3 - "$process_timeout_seconds" "$@" <<'PY'
import subprocess
import os
import signal
import sys

timeout = int(sys.argv[1])
command = sys.argv[2:]
process = subprocess.Popen(command, start_new_session=True)
try:
    return_code = process.wait(timeout=timeout)
except subprocess.TimeoutExpired:
    print(f"Timed out after {timeout} seconds: {command!r}", file=sys.stderr)
    os.killpg(process.pid, signal.SIGKILL)
    process.wait()
    raise SystemExit(124)
raise SystemExit(return_code)
PY
}

remove_installed_payload() {
    local relative_path installed_path
    while IFS= read -r relative_path; do
        installed_path="/$relative_path"
        if [[ -f "$installed_path" || -L "$installed_path" ]]; then
            /bin/rm -f "$installed_path"
        fi
    done <"$payload_paths"
    python3 - "$created_directories" <<'PY' | while IFS= read -r installed_directory; do
from pathlib import PurePosixPath
import sys

paths = {line.rstrip("\r\n") for line in open(sys.argv[1], encoding="utf-8") if line.strip()}
for path in sorted(paths, key=lambda value: (len(PurePosixPath(value).parts), value), reverse=True):
    print(path)
PY
        if [[ -d "$installed_directory" && ! -L "$installed_directory" ]]; then
            rmdir "$installed_directory" 2>/dev/null || true
        fi
    done
    for receipt in "${component_receipts[@]}"; do
        if pkgutil --pkg-info "$receipt" >/dev/null 2>&1; then
            pkgutil --forget "$receipt" >/dev/null
        fi
    done
}

package_installed=0
cleanup() {
    local original_status=$?
    trap - EXIT
    if ((package_installed)); then
        remove_installed_payload || true
    fi
    if [[ -n "${fixture_root:-}" && -d "$fixture_root" ]]; then
        find "$fixture_root" -depth -delete
    fi
    exit "$original_status"
}
trap cleanup EXIT

printf 'PROCEDURE lifecycle_fixture\nRETURN .T.\n' >"$external_fixture"
external_fixture_sha256=$(shasum -a 256 "$external_fixture" | awk '{print $1}')
package_sha256=$(shasum -a 256 "$package_path" | awk '{print $1}')
runner_architecture=$(uname -m)

printf 'Installing productbuild package %s at system root.\n' "$package_path"
package_installed=1
run_bounded installer -pkg "$package_path" -target /

: >"$receipt_paths"
for receipt in "${component_receipts[@]}"; do
    if ! pkgutil --pkg-info "$receipt" >/dev/null 2>&1; then
        printf 'Fresh install did not establish expected receipt: %s\n' "$receipt" >&2
        exit 1
    fi
    receipt_version=$(pkgutil --pkg-info-plist "$receipt" \
        | plutil -extract pkg-version raw -o - -)
    if [[ "$receipt_version" != "$package_version" ]]; then
        printf 'Receipt %s has version %s; expected %s.\n' \
            "$receipt" "$receipt_version" "$package_version" >&2
        exit 1
    fi
    pkgutil --files "$receipt" >>"$receipt_paths"
done
LC_ALL=C sort -u -o "$receipt_paths" "$receipt_paths"

installed_file_count=0
while IFS= read -r relative_path; do
    installed_path="/$relative_path"
    if [[ -f "$installed_path" || -L "$installed_path" ]]; then
        installed_file_count=$((installed_file_count + 1))
    fi
done <"$payload_paths"
if ((installed_file_count < ${#expected_installed_files[@]})); then
    printf 'Installed file inventory is unexpectedly small: %s\n' "$installed_file_count" >&2
    exit 1
fi

for expected_path in "${expected_installed_files[@]}"; do
    if [[ ! -f "$expected_path" || -L "$expected_path" ]]; then
        printf 'Installed package is missing expected regular file: %s\n' "$expected_path" >&2
        exit 1
    fi
    if ! grep -Fqx "${expected_path#/}" "$receipt_paths"; then
        printf 'Package receipts omit installed expected file: %s\n' "$expected_path" >&2
        exit 1
    fi
done

run_bounded /Applications/bin/copperfin_inspect --locale en-US "$external_fixture" \
    | tee "$inspect_stdout"
grep -Fq 'asset_family: program' "$inspect_stdout"
grep -Fq 'status: ok' "$inspect_stdout"
if [[ $(shasum -a 256 "$external_fixture" | awk '{print $1}') != "$external_fixture_sha256" ]]; then
    printf 'Installed-product smoke changed the external fixture.\n' >&2
    exit 1
fi

printf 'Removing exact package payload and receipts from the disposable runner.\n'
remove_installed_payload
package_installed=0

for receipt in "${component_receipts[@]}"; do
    if pkgutil --pkg-info "$receipt" >/dev/null 2>&1; then
        printf 'Package receipt remains after bounded runner cleanup: %s\n' "$receipt" >&2
        exit 1
    fi
done
while IFS= read -r relative_path; do
    installed_path="/$relative_path"
    if [[ -f "$installed_path" || -L "$installed_path" ]]; then
        printf 'Package payload file remains after bounded runner cleanup: %s\n' "$installed_path" >&2
        exit 1
    fi
done <"$payload_paths"
if [[ -e /Applications/share/copperfin || -L /Applications/share/copperfin ]]; then
    printf 'Package-specific directory remains after bounded runner cleanup.\n' >&2
    exit 1
fi
while IFS= read -r installed_directory; do
    if [[ -e "$installed_directory" || -L "$installed_directory" ]]; then
        printf 'Package-created directory remains after bounded runner cleanup: %s\n' \
            "$installed_directory" >&2
        exit 1
    fi
done <"$created_directories"
if [[ $(shasum -a 256 "$external_fixture" | awk '{print $1}') != "$external_fixture_sha256" ]]; then
    printf 'Runner cleanup changed the external fixture.\n' >&2
    exit 1
fi

python3 - \
    "$evidence_directory/macos-installer-lifecycle.json" \
    "$package_sha256" \
    "$package_version" \
    "$product_identifier" \
    "$runner_architecture" \
    "$installed_file_count" \
    "$inspect_stdout" <<'PY'
import json
import pathlib
import sys

destination, package_sha256, package_version, product_identifier, architecture, count, stdout_path = sys.argv[1:]
evidence = {
    "schema_version": 1,
    "kind": "copperfin-macos-installer-lifecycle-result",
    "package_sha256": package_sha256,
    "package_version": package_version,
    "product_identifier": product_identifier,
    "component_receipts": [
        product_identifier + ".Documentation",
        product_identifier + ".Unspecified",
    ],
    "runner_architecture": architecture,
    "fresh_install": "PASS",
    "component_receipt_contract": "PASS",
    "installed_tree_contract": "PASS",
    "english_locale_catalog": "PASS",
    "installed_cli_smoke": "PASS",
    "installed_cli_stdout": pathlib.Path(stdout_path).read_text(encoding="utf-8"),
    "external_artifact_survived": "PASS",
    "bounded_runner_cleanup": "PASS",
    "package_receipt_residue": "PASS",
    "filesystem_residue": "PASS",
    "installed_file_count": int(count),
    "developer_id_and_notarization": "NOT_RUN",
    "automated_gui": "NOT_RUN",
    "human_gui": "NOT_RUN",
}
pathlib.Path(destination).write_text(json.dumps(evidence, indent=2, sort_keys=True) + "\n", encoding="utf-8")
PY

printf 'macOS productbuild installer lifecycle validation passed.\n'
