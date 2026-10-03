#!/usr/bin/env bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# Traceability: RQ-CF-REL-006; DQ-linux-rpm-installer-lifecycle-scope;
# DV-linux-rpm-installer-lifecycle-contract; HZ-system-failure-01;
# HZ-data-corruption-01; HZ-doc-command-01.

set -euo pipefail

usage() {
    printf 'Usage: %s --package <rpm> --evidence-directory <directory> --allow-container-mutation [--process-timeout-seconds <seconds>]\n' "$0" >&2
}

package_path=""
evidence_directory=""
process_timeout_seconds=180
allow_container_mutation=0
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
        --allow-container-mutation)
            allow_container_mutation=1
            shift
            ;;
        *)
            usage
            exit 2
            ;;
    esac
done

if [[ -z "$package_path" || -z "$evidence_directory" || ! "$process_timeout_seconds" =~ ^[1-9][0-9]*$ \
    || "$allow_container_mutation" != 1 ]]; then
    usage
    exit 2
fi
if [[ ${GITHUB_ACTIONS:-} != "true" || ${RUNNER_ENVIRONMENT:-} != "github-hosted" \
    || ${COPPERFIN_RPM_CONTAINER:-} != "true" ]]; then
    printf 'Linux RPM lifecycle validation is restricted to its disposable GitHub-hosted container.\n' >&2
    exit 1
fi
if [[ ! -f /.dockerenv || $EUID != 0 ]]; then
    printf 'Linux RPM lifecycle validation must run as root inside a Docker container.\n' >&2
    exit 1
fi
if [[ ! ${CONTAINER_IMAGE_REFERENCE:-} =~ ^docker\.io/library/fedora@sha256:[0-9a-f]{64}$ \
    || ! ${CONTAINER_IMAGE_ID:-} =~ ^sha256:[0-9a-f]{64}$ ]]; then
    printf 'Pinned Fedora container identity is missing or malformed.\n' >&2
    exit 1
fi

for command_name in awk cat cmp find grep jq mktemp realpath rpm sha256sum sort tee timeout wc; do
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
if [[ -z "$runner_temp" || -L "$runner_temp" || ! -d "$runner_temp" ]]; then
    printf 'RUNNER_TEMP must identify an existing regular container directory.\n' >&2
    exit 1
fi
runner_temp=$(realpath -e -- "$runner_temp")

package_name=$(rpm --query --package --queryformat '%{NAME}' "$package_path")
package_version=$(rpm --query --package --queryformat '%{VERSION}-%{RELEASE}' "$package_path")
package_architecture=$(rpm --query --package --queryformat '%{ARCH}' "$package_path")
if [[ "$package_name" != "copperfin" || -z "$package_version" || "$package_architecture" != "x86_64" ]]; then
    printf 'Unexpected RPM package identity: name=%s version=%s architecture=%s\n' \
        "$package_name" "$package_version" "$package_architecture" >&2
    exit 1
fi
if rpm --query "$package_name" >/dev/null 2>&1; then
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

fixture_root=$(mktemp -d "$runner_temp/copperfin-linux-rpm-lifecycle.XXXXXX")
fixture_root=$(realpath -e -- "$fixture_root")
case "$fixture_root" in
    "$runner_temp"/copperfin-linux-rpm-lifecycle.*) ;;
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
        timeout --signal=KILL "$process_timeout_seconds" rpm --erase "$package_name" >/dev/null 2>&1 || true
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

printf 'Installing %s %s from %s in %s\n' \
    "$package_name" "$package_version" "$package_path" "$CONTAINER_IMAGE_REFERENCE"
package_installed=1
run_bounded rpm --install "$package_path"

installed_identity=$(rpm --query --queryformat '%{NAME} %{VERSION}-%{RELEASE} %{ARCH}' "$package_name")
if [[ "$installed_identity" != "$package_name $package_version $package_architecture" ]]; then
    printf 'Fresh RPM install did not establish the expected package identity.\n' >&2
    exit 1
fi

for expected_path in "${expected_installed_files[@]}"; do
    if [[ ! -f "$expected_path" || -L "$expected_path" ]]; then
        printf 'Installed package is missing expected regular file: %s\n' "$expected_path" >&2
        exit 1
    fi
done

rpm --query --list "$package_name" \
    | while IFS= read -r installed_path; do
        if [[ -f "$installed_path" || -L "$installed_path" ]]; then
            printf '%s\n' "$installed_path"
        fi
    done \
    | LC_ALL=C sort -u >"$installed_paths"
rpm --query --list "$package_name" \
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
if ! run_bounded rpm --verify "$package_name" >"$fixture_root/rpm-verify-before.txt"; then
    printf 'RPM verification failed immediately after installation.\n' >&2
    cat "$fixture_root/rpm-verify-before.txt" >&2
    exit 1
fi
if [[ -s "$fixture_root/rpm-verify-before.txt" ]]; then
    printf 'RPM verification reported changed installed files.\n' >&2
    cat "$fixture_root/rpm-verify-before.txt" >&2
    exit 1
fi

while IFS= read -r installed_path; do
    sha256sum -- "$installed_path"
done <"$installed_paths" | LC_ALL=C sort >"$installed_inventory_before"

run_bounded /usr/bin/copperfin_inspect --locale en-US "$external_fixture" | tee "$inspect_stdout"
grep -Fq 'asset_family: program' "$inspect_stdout"
grep -Fq 'status: ok' "$inspect_stdout"

printf 'Reinstalling the same RPM version for maintenance validation.\n'
run_bounded rpm --replacepkgs --install "$package_path"
if [[ $(rpm --query --queryformat '%{NAME} %{VERSION}-%{RELEASE} %{ARCH}' "$package_name") \
    != "$package_name $package_version $package_architecture" ]]; then
    printf 'Same-version RPM reinstall did not preserve package identity.\n' >&2
    exit 1
fi
while IFS= read -r installed_path; do
    if [[ ! -f "$installed_path" && ! -L "$installed_path" ]]; then
        printf 'Same-version RPM reinstall removed an installed file: %s\n' "$installed_path" >&2
        exit 1
    fi
    sha256sum -- "$installed_path"
done <"$installed_paths" | LC_ALL=C sort >"$installed_inventory_after"
if ! cmp -s "$installed_inventory_before" "$installed_inventory_after"; then
    printf 'Same-version RPM reinstall changed the installed file inventory.\n' >&2
    exit 1
fi
if ! run_bounded rpm --verify "$package_name" >"$fixture_root/rpm-verify-after.txt" \
    || [[ -s "$fixture_root/rpm-verify-after.txt" ]]; then
    printf 'RPM verification failed after the same-version reinstall.\n' >&2
    cat "$fixture_root/rpm-verify-after.txt" >&2
    exit 1
fi
if [[ $(sha256sum -- "$external_fixture" | awk '{print $1}') != "$external_fixture_sha256" ]]; then
    printf 'Same-version RPM reinstall changed the external fixture.\n' >&2
    exit 1
fi

printf 'Erasing %s and verifying RPM database and filesystem residue.\n' "$package_name"
run_bounded rpm --erase "$package_name"
package_installed=0
if rpm --query "$package_name" >/dev/null 2>&1; then
    printf 'RPM database retains a %s record after erase.\n' "$package_name" >&2
    exit 1
fi
while IFS= read -r installed_path; do
    if [[ -e "$installed_path" || -L "$installed_path" ]]; then
        printf 'Installed RPM file remains after erase: %s\n' "$installed_path" >&2
        exit 1
    fi
done <"$installed_paths"
while IFS= read -r installed_directory; do
    if [[ -e "$installed_directory" || -L "$installed_directory" ]]; then
        printf 'Package-specific directory remains after RPM erase: %s\n' "$installed_directory" >&2
        exit 1
    fi
done <"$installed_directories"
if [[ $(sha256sum -- "$external_fixture" | awk '{print $1}') != "$external_fixture_sha256" ]]; then
    printf 'RPM erase changed the external fixture.\n' >&2
    exit 1
fi

installed_cli_stdout=$(cat "$inspect_stdout")
jq -n \
    --arg package_sha256 "$package_sha256" \
    --arg container_image_reference "$CONTAINER_IMAGE_REFERENCE" \
    --arg container_image_id "$CONTAINER_IMAGE_ID" \
    --arg package_name "$package_name" \
    --arg package_version "$package_version" \
    --arg package_architecture "$package_architecture" \
    --arg installed_cli_stdout "$installed_cli_stdout" \
    --argjson installed_file_count "$installed_file_count" \
    '{
        schema_version: 1,
        kind: "copperfin-linux-rpm-installer-lifecycle-result",
        package_sha256: $package_sha256,
        container_image_reference: $container_image_reference,
        container_image_id: $container_image_id,
        package_name: $package_name,
        package_version: $package_version,
        package_architecture: $package_architecture,
        fresh_install: "PASS",
        installed_tree_contract: "PASS",
        english_locale_catalog: "PASS",
        installed_cli_smoke: "PASS",
        installed_cli_stdout: $installed_cli_stdout,
        package_verify: "PASS",
        same_version_maintenance_reinstall: "PASS",
        external_artifact_survived: "PASS",
        erase_uninstall: "PASS",
        package_database_residue: "PASS",
        filesystem_residue: "PASS",
        installed_file_count: $installed_file_count
    }' >"$evidence_directory/linux-rpm-installer-lifecycle.json"

printf 'Linux RPM installer lifecycle validation passed.\n'
