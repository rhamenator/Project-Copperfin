#!/usr/bin/env sh
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

set -eu
umask 077

usage() {
    printf '%s\n' \
        'usage: generate-launcher-signing-key.sh --key-id <approved-id> --output-dir <outside-checkout-directory>' >&2
    exit 2
}

key_id=''
output_dir=''

while [ "$#" -gt 0 ]; do
    case "$1" in
        --key-id)
            [ "$#" -ge 2 ] || usage
            key_id=$2
            shift 2
            ;;
        --output-dir)
            [ "$#" -ge 2 ] || usage
            output_dir=$2
            shift 2
            ;;
        --help|-h)
            usage
            ;;
        *)
            usage
            ;;
    esac
done

[ -n "$key_id" ] && [ -n "$output_dir" ] || usage
case "$key_id" in
    *[!A-Za-z0-9._-]*|'')
        printf 'launcher signer key ID must contain only ASCII letters, digits, dot, underscore, or hyphen\n' >&2
        exit 1
        ;;
esac

for required_command in openssl od tail tr sed awk wc cmp mktemp stat id; do
    command -v "$required_command" >/dev/null 2>&1 || {
        printf 'required command is unavailable: %s\n' "$required_command" >&2
        exit 1
    }
done

script_dir=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd -P)
repo_root=$(CDPATH='' cd -- "$script_dir/../.." && pwd -P)

if [ -e "$output_dir" ]; then
    if [ ! -d "$output_dir" ]; then
        printf 'launcher key output path exists but is not a directory: %s\n' "$output_dir" >&2
        exit 1
    fi
    resolved_output_dir=$(CDPATH='' cd -- "$output_dir" && pwd -P)
else
    output_parent=$(dirname -- "$output_dir")
    output_leaf=$(basename -- "$output_dir")
    if [ ! -d "$output_parent" ]; then
        printf 'launcher key output parent directory does not exist: %s\n' "$output_parent" >&2
        exit 1
    fi
    resolved_output_parent=$(CDPATH='' cd -- "$output_parent" && pwd -P)
    resolved_output_dir="$resolved_output_parent/$output_leaf"
fi
case "$resolved_output_dir" in
    "$repo_root"|"$repo_root"/*)
        printf 'refusing to generate launcher signing material inside the repository checkout: %s\n' "$output_dir" >&2
        exit 1
        ;;
esac
if [ ! -d "$resolved_output_dir" ]; then
    mkdir -m 700 -- "$resolved_output_dir"
fi

output_owner=$(stat -c '%u' "$resolved_output_dir")
current_owner=$(id -u)
if [ "$output_owner" != "$current_owner" ]; then
    printf 'launcher key output directory must be owned by the current user\n' >&2
    exit 1
fi
output_mode=$(stat -c '%a' "$resolved_output_dir")
if [ $((0$output_mode & 077)) -ne 0 ]; then
    printf 'launcher key output directory must not grant group or other permissions: %s\n' "$output_mode" >&2
    exit 1
fi

file_prefix="${key_id}_launcher"
private_key="$resolved_output_dir/${file_prefix}_private.pem"
public_key="$resolved_output_dir/${file_prefix}_public.pem"
registry_header="$resolved_output_dir/${file_prefix}_registry.h"
metadata_file="$resolved_output_dir/${file_prefix}_metadata.json"

for target in "$private_key" "$public_key" "$registry_header" "$metadata_file"; do
    if [ -e "$target" ]; then
        printf 'refusing to overwrite existing launcher identity output: %s\n' "$target" >&2
        exit 1
    fi
done

temporary_private=$(mktemp "$resolved_output_dir/.launcher-private.XXXXXX")
temporary_public=$(mktemp "$resolved_output_dir/.launcher-public.XXXXXX")
temporary_public_der=$(mktemp "$resolved_output_dir/.launcher-public-der.XXXXXX")
temporary_derived_der=$(mktemp "$resolved_output_dir/.launcher-derived-der.XXXXXX")
temporary_registry=$(mktemp "$resolved_output_dir/.launcher-registry.XXXXXX")
temporary_metadata=$(mktemp "$resolved_output_dir/.launcher-metadata.XXXXXX")
cleanup() {
    rm -f \
        "$temporary_private" \
        "$temporary_public" \
        "$temporary_public_der" \
        "$temporary_derived_der" \
        "$temporary_registry" \
        "$temporary_metadata"
}
trap cleanup EXIT HUP INT TERM

openssl genpkey -algorithm ED25519 -out "$temporary_private"
openssl pkey -in "$temporary_private" -noout -check >/dev/null
openssl pkey -in "$temporary_private" -pubout -out "$temporary_public"
openssl pkey -pubin -in "$temporary_public" -outform DER -out "$temporary_public_der"
openssl pkey -in "$temporary_private" -pubout -outform DER -out "$temporary_derived_der"
cmp -s "$temporary_public_der" "$temporary_derived_der" || {
    printf 'generated launcher public key does not match the private key\n' >&2
    exit 1
}

der_size=$(wc -c < "$temporary_public_der" | tr -d '[:space:]')
[ "$der_size" = '44' ] || {
    printf 'generated public key is not a canonical Ed25519 SubjectPublicKeyInfo value\n' >&2
    exit 1
}
der_prefix=$(od -An -N 12 -v -t x1 "$temporary_public_der" | tr -d '[:space:]')
[ "$der_prefix" = '302a300506032b6570032100' ] || {
    printf 'generated public key does not use the Ed25519 algorithm identifier\n' >&2
    exit 1
}
raw_public_hex=$(tail -c 32 "$temporary_public_der" | od -An -v -t x1 | tr -d '[:space:]')
[ "${#raw_public_hex}" -eq 64 ] || {
    printf 'generated Ed25519 public key is not exactly 32 bytes\n' >&2
    exit 1
}
public_key_bytes=$(printf '%s' "$raw_public_hex" |
    sed 's/../0x&, /g; s/, $//')
public_der_sha256=$(openssl dgst -sha256 -r "$temporary_public_der" |
    awk '{print $1}')

cat > "$temporary_registry" <<EOF
// Generated public launcher-trust registry for signer "$key_id".
// Public material only. The matching private PEM must never enter source control.
#pragma once

#include "copperfin/package/launcher_inventory_trust.h"

#include <array>

namespace copperfin::package_trust {

inline constexpr std::array<LauncherInventoryTrustedKey, 1>
    kKnownLauncherInventoryTrustedKeys{{
        {"$key_id", {{$public_key_bytes}}}
    }};

}  // namespace copperfin::package_trust
EOF

cat > "$temporary_metadata" <<EOF
{
  "schema_version": 1,
  "kind": "copperfin-launcher-release-signing-identity",
  "signature_algorithm": "ed25519",
  "signer_key_id": "$key_id",
  "public_key_der_sha256": "$public_der_sha256",
  "private_key_file": "$(basename -- "$private_key")",
  "public_key_file": "$(basename -- "$public_key")",
  "registry_header_file": "$(basename -- "$registry_header")"
}
EOF

chmod 600 "$temporary_private"
chmod 644 "$temporary_public" "$temporary_registry" "$temporary_metadata"
mv "$temporary_public" "$public_key"
mv "$temporary_registry" "$registry_header"
mv "$temporary_metadata" "$metadata_file"
mv "$temporary_private" "$private_key"
trap - EXIT HUP INT TERM
rm -f "$temporary_public_der" "$temporary_derived_der"

printf '%s\n' \
    "Generated dedicated Copperfin launcher-release identity \"$key_id\"." \
    "Private key (mode 0600; never commit or share): $private_key" \
    "Public key PEM: $public_key" \
    "Launcher registry header: $registry_header" \
    "Non-secret metadata: $metadata_file" \
    "GitHub release environment secret COPPERFIN_LAUNCHER_TRUST_SIGNING_KEY_PEM <- $private_key" \
    "GitHub release environment secret COPPERFIN_LAUNCHER_TRUST_REGISTRY_HEADER <- $registry_header" \
    "Windows workflow signer_key_id input <- $key_id" \
    "This identity authenticates the Windows generated-launcher inventory; it does not sign macOS or Linux artifacts."
