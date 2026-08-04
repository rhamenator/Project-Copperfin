#!/usr/bin/env sh
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

set -eu

usage() {
    printf '%s\n' \
        'usage: sign-launcher-inventory.sh --input app.cftrust --output app.cftrust.sig --key-ref /outside/checkout/key.pem' >&2
    exit 2
}

input_path=''
output_path=''
key_ref=''

while [ "$#" -gt 0 ]; do
    case "$1" in
        --input)
            [ "$#" -ge 2 ] || usage
            input_path=$2
            shift 2
            ;;
        --output)
            [ "$#" -ge 2 ] || usage
            output_path=$2
            shift 2
            ;;
        --key-ref)
            [ "$#" -ge 2 ] || usage
            key_ref=$2
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

[ -n "$input_path" ] && [ -n "$output_path" ] && [ -n "$key_ref" ] || usage
[ -f "$input_path" ] || { printf 'input envelope not found: %s\n' "$input_path" >&2; exit 1; }
[ -f "$key_ref" ] || { printf 'external signing key not found: %s\n' "$key_ref" >&2; exit 1; }
[ "$input_path" != "$output_path" ] || { printf 'input and output must differ\n' >&2; exit 1; }
command -v openssl >/dev/null 2>&1 || {
    printf 'openssl is required for external Ed25519 signing\n' >&2
    exit 1
}

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd -P)
repo_root=$(CDPATH= cd -- "$script_dir/../.." && pwd -P)
key_dir=$(CDPATH= cd -- "$(dirname -- "$key_ref")" && pwd -P)
case "$key_dir/$(basename -- "$key_ref")" in
    "$repo_root"|"$repo_root"/*)
        printf 'refusing a signing key inside the repository checkout: %s\n' "$key_ref" >&2
        exit 1
        ;;
esac

first_line=$(sed -n '1p' "$input_path")
second_line=$(sed -n '2p' "$input_path")
third_line=$(sed -n '3p' "$input_path")
signer_line=$(sed -n '4p' "$input_path")
[ "$first_line" = 'launcher_inventory_version=1' ] || {
    printf 'input is not a version 1 launcher inventory envelope\n' >&2
    exit 1
}
[ "$second_line" = 'hash_algorithm=sha256' ] || {
    printf 'input uses an unsupported inventory hash algorithm\n' >&2
    exit 1
}
[ "$third_line" = 'signature_algorithm=ed25519' ] || {
    printf 'input uses an unsupported inventory signature algorithm\n' >&2
    exit 1
}
last_byte=$(tail -c 1 "$input_path" | od -An -t x1 | tr -d '[:space:]')
[ "$last_byte" = '0a' ] || {
    printf 'input envelope must end with a single LF byte\n' >&2
    exit 1
}
case "$signer_line" in
    signer_key_id=*) signer_key_id=${signer_line#signer_key_id=} ;;
    *) printf 'input is missing signer_key_id\n' >&2; exit 1 ;;
esac
case "$signer_key_id" in
    ''|*[!A-Za-z0-9._-]*) printf 'input contains an invalid signer_key_id\n' >&2; exit 1 ;;
esac

output_dir=$(dirname -- "$output_path")
[ -d "$output_dir" ] || {
    printf 'output directory not found: %s\n' "$output_dir" >&2
    exit 1
}
temporary_signature=$(mktemp "$output_dir/.app.cftrust.sig.XXXXXX")
temporary_raw=$(mktemp "$output_dir/.app.cftrust.raw.XXXXXX")
cleanup() {
    rm -f "$temporary_signature" "$temporary_raw"
}
trap cleanup EXIT HUP INT TERM

openssl pkeyutl -sign -rawin -inkey "$key_ref" -in "$input_path" -out "$temporary_raw"
signature_base64=$(openssl base64 -A -in "$temporary_raw" | tr -d '\r\n')
[ "${#signature_base64}" -eq 88 ] || {
    printf 'OpenSSL did not produce a canonical 64-byte Ed25519 signature\n' >&2
    exit 1
}

{
    printf '%s\n' 'launcher_signature_version=1'
    printf '%s\n' 'signature_algorithm=ed25519'
    printf 'signer_key_id=%s\n' "$signer_key_id"
    printf 'signature_base64=%s\n' "$signature_base64"
} > "$temporary_signature"
mv -f "$temporary_signature" "$output_path"
trap - EXIT HUP INT TERM
rm -f "$temporary_raw"
