#!/bin/bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# Each mode requires its OWN fresh explicit audit+DAW CLEAR. No full4176/repeat.
# Native-only load-order DISCRIMINATOR, process-local; no registry/host changes.
set -eu
cd /home/rich/.codex/worktrees/numeric-literal-display-5611/Project-Copperfin
fixture=tests/fixtures/vfp9-numeric-default-transform
capture_root=build/default-transform-crt-binding-diagnostic
mode=${1:-}
test "$mode" = --build || test "$mode" = --capture || exit 95
mkdir -p "$capture_root"
if [ "$mode" = --build ]; then
 test ! -e "$capture_root/build.started.utc" || exit 96
else
 test -e "$capture_root/build.finished.utc"
 test ! -e "$capture_root/first.started.utc" || exit 96
fi
test ! -e "$capture_root/source-status-${mode#--}.txt" || exit 96
git status --porcelain > "$capture_root/source-status-${mode#--}.txt"
test ! -s "$capture_root/source-status-${mode#--}.txt" || exit 97
stage() {
 local label=$1
 shift
 date -u +%FT%TZ > "$capture_root/$label.started.utc"
 set +e
 pgrep -a -x 'wine|wine64|wine-preloader|wineserver|wineserver64|vfp9.exe' > "$capture_root/$label.prefix.txt"
 local prefix_status=$?
 set -e
 printf '%s\n' "$prefix_status" > "$capture_root/$label.prefix.status"
 test "$prefix_status" -eq 1 || exit 76
 cat /proc/pressure/io > "$capture_root/$label.pressure-before.txt"
 set +e
 awk '$1=="full" {split($2,a,"=");split($3,b,"=");found=1;if(a[2]>=1||b[2]>=1)exit 75} END {if(!found)exit 75}' "$capture_root/$label.pressure-before.txt"
 local guard_status=$?
 set -e
 printf '%s\n' "$guard_status" > "$capture_root/$label.guard.status"
 test "$guard_status" -eq 0 || exit 75
 set +e
 nice -n 10 "$@" > "$capture_root/$label.stdout" 2> "$capture_root/$label.stderr"
 local status=$?
 set -e
 printf '%s\n' "$status" > "$capture_root/$label.status"
 cat /proc/pressure/io > "$capture_root/$label.pressure-after.txt"
 date -u +%FT%TZ > "$capture_root/$label.finished.utc"
 printf '%s status=%s\n' "$label" "$status"
 return "$status"
}
if [ "$mode" = --build ]; then
 test ! -e "$capture_root/build.started.utc" || exit 96
 date -u +%FT%TZ > "$capture_root/build.started.utc"
 git rev-parse HEAD > "$capture_root/build-source-commit.txt"
 sdk='/home/rich/copperfin-wine-vfp9/drive_c/Program Files (x86)/Microsoft Visual FoxPro 9/Samples/API/winapims.lib'
 legacy=/home/rich/dev/Project-Copperfin/build/pr7203-completed/native-evidence/diagnostic
 linker=/home/rich/.rustup/toolchains/stable-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/bin/rust-lld
 sha256sum "$fixture"/{crt-probe.c,crt-binding.h,crt-kernel32.def,crt-cases.json,crt-binding-diagnostic.prg,audit-crt-binding-diagnostic.cjs,crt-binding-diagnostic.sh} "$sdk" "$legacy/kernel32-stdcall.lib" "$legacy/msvcrt.lib" /usr/bin/clang /usr/bin/llvm-dlltool-21 "$linker" /home/rich/copperfin-wine-vfp9/drive_c/windows/syswow64/msvcr71.dll > "$capture_root/build-inputs.sha256"
 stage import /usr/bin/llvm-dlltool-21 -m i386 -k -d "$fixture/crt-kernel32.def" -l "$capture_root/crt-kernel32.lib"
 stage compile /usr/bin/clang --target=i686-pc-windows-msvc -std=c11 -O0 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Werror -c "$fixture/crt-probe.c" -o "$capture_root/crt-probe.obj"
 stage link "$linker" -flavor link /dll /nodefaultlib /machine:x86 /entry:CrtEntry@12 "/out:$capture_root/crt-probe.fll" "$capture_root/crt-probe.obj" "$capture_root/crt-kernel32.lib" "$sdk" "$legacy/kernel32-stdcall.lib" "$legacy/msvcrt.lib"
 sha256sum -c "$capture_root/build-inputs.sha256" > "$capture_root/build-inputs-check.txt"
 sha256sum "$capture_root/crt-probe.fll" "$capture_root/crt-probe.obj" "$capture_root/crt-kernel32.lib" > "$capture_root/built.sha256"
 file "$capture_root/crt-probe.fll" > "$capture_root/built-file.txt"
 objdump -p "$capture_root/crt-probe.fll" > "$capture_root/built-pe.txt"
 date -u +%FT%TZ > "$capture_root/build.finished.utc"
else
 test -e "$capture_root/build.finished.utc"
 test ! -e "$capture_root/first.started.utc" || exit 96
 git rev-parse HEAD > "$capture_root/native-source-commit.txt"
 sha256sum -c "$capture_root/build-inputs.sha256" > "$capture_root/pre-native-build-check.txt"
 sha256sum -c "$capture_root/built.sha256" > "$capture_root/pre-native-binary-check.txt"
 printf '%s\n' "${WINEDLLOVERRIDES-UNSET}" > "$capture_root/inherited-dll-overrides.txt"
 test -z "${WINEDLLOVERRIDES-}" || exit 94
 printf 'WINEDLLOVERRIDES=msvcr71=n\n' > "$capture_root/supplied-runtime-env.txt"
 sha256sum /home/rich/bin/vfp9-probe /home/rich/copperfin-wine-vfp9/drive_c/hotfix968409/vfp9.exe /home/rich/copperfin-wine-vfp9/drive_c/windows/syswow64/msvcr71.dll /home/rich/dev/Project-Copperfin/build/pr7203-completed/native-evidence/diagnostic/cause-probe.fll > "$capture_root/native-inputs.sha256"
 cp "$fixture/crt-binding-diagnostic.prg" "$capture_root/probe.prg"
 cmp "$fixture/crt-binding-diagnostic.prg" "$capture_root/probe.prg"
 native_status=0
 stage first /usr/bin/env WINEDLLOVERRIDES=msvcr71=n /home/rich/bin/vfp9-probe "$capture_root/probe.prg" --result "$capture_root/results.txt" || native_status=$?
 cp "$capture_root/first.stdout" "$capture_root/first.out"
 if [ -f "$capture_root/results.txt" ]; then cp "$capture_root/results.txt" "$capture_root/first-results.txt"; fi
 sha256sum -c "$capture_root/native-inputs.sha256" > "$capture_root/native-inputs-check.txt"
 test "$native_status" -eq 0 || exit "$native_status"
 diagnostic_status=0
 node "$fixture/audit-crt-binding-diagnostic.cjs" "$capture_root" > "$capture_root/diagnostic.json" 2> "$capture_root/diagnostic.stderr" || diagnostic_status=$?
 printf '%s\n' "$diagnostic_status" > "$capture_root/diagnostic.status"
 test "$diagnostic_status" -eq 0 || exit "$diagnostic_status"
 binding_status=0
 node "$fixture/audit-crt-binding-diagnostic.cjs" "$capture_root" --require-bound > "$capture_root/binding-check.json" 2> "$capture_root/binding-check.stderr" || binding_status=$?
 printf '%s\n' "$binding_status" > "$capture_root/binding-check.status"
 date -u +%FT%TZ > "$capture_root/native.finished.utc"
 exit "$binding_status"
fi
