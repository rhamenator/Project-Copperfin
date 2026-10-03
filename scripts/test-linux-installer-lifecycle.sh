#!/usr/bin/env bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# Traceability: RQ-CF-REL-006; DQ-linux-installer-lifecycle-scope;
# DV-linux-installer-lifecycle-contract; HZ-system-failure-01;
# HZ-data-corruption-01; HZ-doc-command-01.

set -euo pipefail

usage() {
    printf 'Usage: %s --package <deb> --evidence-directory <directory> --allow-system-mutation [--process-timeout-seconds <seconds>]\n' "$0" >&2
}

package_path=""
evidence_directory=""
process_timeout_seconds=180
allow_system_mutation=0
while (($# > 0)); do
    case "$1" in
        --package)
            package_path=${2-}
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

if [[ -z "$package_path" || -z "$evidence_directory" || ! "$process_timeout_seconds" =~ ^[1-9][0-9]*$ \
    || "$allow_system_mutation" != 1 ]]; then
    usage
    exit 2
fi
if [[ ${GITHUB_ACTIONS:-} != "true" || ${RUNNER_ENVIRONMENT:-} != "github-hosted" ]]; then
    printf 'Linux installer lifecycle validation is restricted to a GitHub-hosted Actions runner.\n' >&2
    exit 1
fi
if ((EUID != 0)); then
    printf 'Linux installer lifecycle validation must run as root.\n' >&2
    exit 1
fi

for command_name in dpkg dpkg-deb dpkg-query find jq realpath sha256sum timeout; do
    if ! command -v "$command_name" >/dev/null 2>&1; then
        printf 'Required command is unavailable: %s\n' "$command_name" >&2
        exit 1
    fi
done

if [[ -L "$package_path" || ! -f "$package_path" ]]; then
    printf 'Package must be an existing regular file: %s\n' "$package_path" >&2
    exit 1
fi
if [[ -L "$evidence_directory" || ! -d "$evidence_directory" ]]; then
    printf 'Evidence directory must be an existing regular directory: %s\n' "$evidence_directory" >&2
    exit 1
fi
package_path=$(realpath -e -- "$package_path")
evidence_directory=$(realpath -e -- "$evidence_directory")

runner_temp=${RUNNER_TEMP:-}
if [[ -z "$runner_temp" ]]; then
    printf 'RUNNER_TEMP is required for bounded fixture cleanup.\n' >&2
    exit 1
fi
if [[ -L "$runner_temp" || ! -d "$runner_temp" ]]; then
    printf 'RUNNER_TEMP must be an existing regular directory: %s\n' "$runner_temp" >&2
    exit 1
fi
runner_temp=$(realpath -e -- "$runner_temp")

package_name=$(dpkg-deb --field "$package_path" Package)
package_version=$(dpkg-deb --field "$package_path" Version)
package_architecture=$(dpkg-deb --field "$package_path" Architecture)
if [[ "$package_name" != "copperfin" || -z "$package_version" || "$package_architecture" != "amd64" ]]; then
    printf 'Unexpected Debian package identity: name=%s version=%s architecture=%s\n' \
        "$package_name" "$package_version" "$package_architecture" >&2
    exit 1
fi
if dpkg-query --show "$package_name" >/dev/null 2>&1; then
    printf 'Refusing to disturb a pre-existing %s package database record.\n' "$package_name" >&2
    exit 1
fi

expected_installed_files=(
    /usr/bin/copperfin_build_host
    /usr/bin/copperfin_inspect
    /usr/bin/copperfin_mcp_host
    /usr/bin/copperfin_runtime_host
    /usr/bin/copperfin_studio_host
    /usr/share/copperfin/locales/en-US/strings.json
)
for expected_path in "${expected_installed_files[@]}" /usr/share/copperfin /usr/share/doc/copperfin; do
    if [[ -e "$expected_path" || -L "$expected_path" ]]; then
        printf 'Refusing to overwrite a pre-existing package path: %s\n' "$expected_path" >&2
        exit 1
    fi
done

fixture_root=$(mktemp -d "$runner_temp/copperfin-linux-installer-lifecycle.XXXXXX")
fixture_root=$(realpath -e -- "$fixture_root")
case "$fixture_root" in
    "$runner_temp"/copperfin-linux-installer-lifecycle.*) ;;
    *)
        printf 'Fixture directory escaped RUNNER_TEMP: %s\n' "$fixture_root" >&2
        exit 1
        ;;
esac

package_installed=0
cleanup() {
    local original_status=$?
    trap - EXIT
    if ((package_installed)); then
        timeout --signal=KILL "$process_timeout_seconds" dpkg --purge "$package_name" >/dev/null 2>&1 || true
    fi
    if [[ -n "${fixture_root:-}" && -d "$fixture_root" ]]; then
        find "$fixture_root" -depth -delete
    fi
    exit "$original_status"
}
trap cleanup EXIT

run_bounded() {
    timeout --signal=KILL "$process_timeout_seconds" "$@"
}

package_sha256=$(sha256sum -- "$package_path" | awk '{print $1}')
installed_paths="$fixture_root/installed-paths.txt"
installed_directories="$fixture_root/installed-directories.txt"
installed_inventory_before="$fixture_root/installed-inventory-before.txt"
installed_inventory_after="$fixture_root/installed-inventory-after.txt"
external_fixture="$fixture_root/external-fixture.prg"
inspect_stdout="$fixture_root/copperfin-inspect.stdout"

printf 'PROCEDURE lifecycle_fixture\nRETURN .T.\n' >"$external_fixture"
external_fixture_sha256=$(sha256sum -- "$external_fixture" | awk '{print $1}')

printf 'Installing %s %s from %s\n' "$package_name" "$package_version" "$package_path"
package_installed=1
run_bounded dpkg --install "$package_path"

installed_status=$(dpkg-query --show --showformat='${Status}' "$package_name")
installed_version=$(dpkg-query --show --showformat='${Version}' "$package_name")
if [[ "$installed_status" != "install ok installed" || "$installed_version" != "$package_version" ]]; then
    printf 'Fresh install did not establish the expected package state.\n' >&2
    exit 1
fi

for expected_path in "${expected_installed_files[@]}"; do
    if [[ ! -f "$expected_path" || -L "$expected_path" ]]; then
        printf 'Installed package is missing expected regular file: %s\n' "$expected_path" >&2
        exit 1
    fi
done

dpkg-query --listfiles "$package_name" \
    | while IFS= read -r installed_path; do
        if [[ -f "$installed_path" || -L "$installed_path" ]]; then
            printf '%s\n' "$installed_path"
        fi
    done \
    | LC_ALL=C sort -u >"$installed_paths"
dpkg-query --listfiles "$package_name" \
    | while IFS= read -r installed_path; do
        if [[ -d "$installed_path" && "$installed_path" == *copperfin* ]]; then
            printf '%s\n' "$installed_path"
        fi
    done \
    | LC_ALL=C sort -u >"$installed_directories"
installed_file_count=$(wc -l <"$installed_paths")
if ((installed_file_count < ${#expected_installed_files[@]})); then
    printf 'Installed file inventory is unexpectedly small: %s\n' "$installed_file_count" >&2
    exit 1
fi
if [[ ! -s "$installed_directories" ]]; then
    printf 'Installed package exposes no package-specific directory inventory.\n' >&2
    exit 1
fi

while IFS= read -r installed_path; do
    sha256sum -- "$installed_path"
done <"$installed_paths" | LC_ALL=C sort >"$installed_inventory_before"

run_bounded /usr/bin/copperfin_inspect --locale en-US "$external_fixture" | tee "$inspect_stdout"
grep -Fq 'asset_family: program' "$inspect_stdout"
grep -Fq 'status: ok' "$inspect_stdout"

printf 'Reinstalling the same package version for maintenance validation.\n'
run_bounded dpkg --install "$package_path"
if [[ $(dpkg-query --show --showformat='${Status}' "$package_name") != "install ok installed" \
    || $(dpkg-query --show --showformat='${Version}' "$package_name") != "$package_version" ]]; then
    printf 'Same-version maintenance reinstall did not preserve package identity.\n' >&2
    exit 1
fi
while IFS= read -r installed_path; do
    if [[ ! -f "$installed_path" && ! -L "$installed_path" ]]; then
        printf 'Same-version reinstall removed an installed file: %s\n' "$installed_path" >&2
        exit 1
    fi
    sha256sum -- "$installed_path"
done <"$installed_paths" | LC_ALL=C sort >"$installed_inventory_after"
if ! cmp -s "$installed_inventory_before" "$installed_inventory_after"; then
    printf 'Same-version reinstall changed the installed file inventory.\n' >&2
    exit 1
fi
while IFS= read -r installed_directory; do
    if [[ ! -d "$installed_directory" || -L "$installed_directory" ]]; then
        printf 'Same-version reinstall removed a package-specific directory: %s\n' "$installed_directory" >&2
        exit 1
    fi
done <"$installed_directories"
if [[ $(sha256sum -- "$external_fixture" | awk '{print $1}') != "$external_fixture_sha256" ]]; then
    printf 'Same-version reinstall changed the external fixture.\n' >&2
    exit 1
fi

printf 'Purging %s and verifying package and filesystem residue.\n' "$package_name"
run_bounded dpkg --purge "$package_name"
package_installed=0
if dpkg-query --show "$package_name" >/dev/null 2>&1; then
    printf 'Package database retains a %s record after purge.\n' "$package_name" >&2
    exit 1
fi
while IFS= read -r installed_path; do
    if [[ -e "$installed_path" || -L "$installed_path" ]]; then
        printf 'Installed package file remains after purge: %s\n' "$installed_path" >&2
        exit 1
    fi
done <"$installed_paths"
while IFS= read -r installed_directory; do
    if [[ -e "$installed_directory" || -L "$installed_directory" ]]; then
        printf 'Package-specific directory remains after purge: %s\n' "$installed_directory" >&2
        exit 1
    fi
done <"$installed_directories"
if [[ $(sha256sum -- "$external_fixture" | awk '{print $1}') != "$external_fixture_sha256" ]]; then
    printf 'Package purge changed the external fixture.\n' >&2
    exit 1
fi

installed_cli_stdout=$(cat "$inspect_stdout")
jq -n \
    --arg package_sha256 "$package_sha256" \
    --arg package_name "$package_name" \
    --arg package_version "$package_version" \
    --arg package_architecture "$package_architecture" \
    --arg installed_cli_stdout "$installed_cli_stdout" \
    --argjson installed_file_count "$installed_file_count" \
    '{
        schema_version: 1,
        kind: "copperfin-linux-installer-lifecycle-result",
        package_sha256: $package_sha256,
        package_name: $package_name,
        package_version: $package_version,
        package_architecture: $package_architecture,
        fresh_install: "PASS",
        installed_tree_contract: "PASS",
        english_locale_catalog: "PASS",
        installed_cli_smoke: "PASS",
        installed_cli_stdout: $installed_cli_stdout,
        same_version_maintenance_reinstall: "PASS",
        external_artifact_survived: "PASS",
        purge_uninstall: "PASS",
        package_database_residue: "PASS",
        filesystem_residue: "PASS",
        installed_file_count: $installed_file_count,
        rpm_lifecycle: "NOT_RUN"
    }' >"$evidence_directory/linux-installer-lifecycle.json"

printf 'Linux Debian installer lifecycle validation passed.\n'
