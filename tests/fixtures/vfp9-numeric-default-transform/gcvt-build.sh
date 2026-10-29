#!/bin/bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
# Native-only build; request NEW explicit audit+DAW CLEAR before invoking.
# Proprietary SDK-containing objects/libraries/FLL remain HOST-ONLY.
set -eu
test "${1-}" = --fresh-both-clear || exit 93
cd /home/rich/.codex/worktrees/numeric-literal-display-5611/Project-Copperfin
capture_root=build/default-transform-gcvt
fixture=tests/fixtures/vfp9-numeric-default-transform
mkdir -p "$capture_root"
test ! -e "$capture_root/build.started.utc" || exit 96
git status --porcelain > "$capture_root/build-source-status.txt"
test ! -s "$capture_root/build-source-status.txt" || exit 97
date -u +%FT%TZ > "$capture_root/build.started.utc"
git rev-parse HEAD > "$capture_root/build-source-commit.txt"
sdk='/home/rich/copperfin-wine-vfp9/drive_c/Program Files (x86)/Microsoft Visual FoxPro 9/Samples/API/winapims.lib'
legacy=/home/rich/dev/Project-Copperfin/build/pr7203-completed/native-evidence/diagnostic
linker=/home/rich/.rustup/toolchains/stable-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/bin/rust-lld
sources='gcvt-probe.c gcvt-boundary.h sprintf-boundary.h gcvt-boundary-check.c gcvt-source-check.cjs emit-gcvt.cjs gcvt.prg gcvt-cases.json audit-gcvt.cjs gcvt-build.sh gcvt-capture.sh crt-probe.c crt-binding.h crt-binding-check.c crt-binding-check.cjs crt-kernel32.def crt-cases.json crt.prg'
for name in $sources; do cp "$fixture/$name" "$capture_root/source-$name"; done
node "$fixture/gcvt-source-check.cjs" > "$capture_root/source-check.json"
node "$fixture/emit-gcvt.cjs" source > "$capture_root/regenerated.prg"
node "$fixture/emit-gcvt.cjs" cases > "$capture_root/regenerated-cases.json"
cmp "$fixture/gcvt.prg" "$capture_root/regenerated.prg"
cmp "$fixture/gcvt-cases.json" "$capture_root/regenerated-cases.json"
sha256sum "$fixture/gcvt-probe.c" "$fixture/gcvt-boundary.h" "$fixture/sprintf-boundary.h" "$fixture/gcvt-boundary-check.c" "$fixture/gcvt-source-check.cjs" "$fixture/emit-gcvt.cjs" "$fixture/gcvt.prg" "$fixture/gcvt-cases.json" "$fixture/audit-gcvt.cjs" "$fixture/gcvt-build.sh" "$fixture/gcvt-capture.sh" "$fixture/crt-probe.c" "$fixture/crt-binding.h" "$fixture/crt-binding-check.c" "$fixture/crt-binding-check.cjs" "$fixture/crt-kernel32.def" "$fixture/crt-cases.json" "$fixture/crt.prg" "$fixture/arithmetic-inline-captures1/"{audit.json,first-results.txt,repeat-results.txt}.gz "$sdk" "$legacy/kernel32-stdcall.lib" "$legacy/msvcrt.lib" /usr/bin/gcc /usr/bin/clang /usr/bin/llvm-dlltool-21 "$linker" /home/rich/copperfin-wine-vfp9/drive_c/windows/syswow64/msvcr71.dll > "$capture_root/build-inputs.sha256"
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
 test "$status" -eq 0 || exit "$status"
}
stage host-gnu /usr/bin/gcc -std=c11 -O0 -Wall -Wextra -Werror -fsanitize=address,undefined "$fixture/crt-binding-check.c" -o "$capture_root/binding-gnu"
stage check-gnu /usr/bin/env ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 node "$fixture/crt-binding-check.cjs" "$capture_root/binding-gnu" "$capture_root"
stage host-clang /usr/bin/clang -std=c11 -O0 -Wall -Wextra -Werror -fsanitize=address,undefined "$fixture/crt-binding-check.c" -o "$capture_root/binding-clang"
stage check-clang /usr/bin/env ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 node "$fixture/crt-binding-check.cjs" "$capture_root/binding-clang" "$capture_root"
stage boundary-gnu-build /usr/bin/gcc -std=c11 -O0 -Wall -Wextra -Werror -fsanitize=address,undefined "$fixture/gcvt-boundary-check.c" -o "$capture_root/gcvt-gnu"
stage boundary-gnu-check /usr/bin/env ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 "$capture_root/gcvt-gnu"
printf 'gcvt-boundary checks=115 PASS (stub only)\n' > "$capture_root/expected-boundary.stdout"
cmp "$capture_root/expected-boundary.stdout" "$capture_root/boundary-gnu-check.stdout"
test ! -s "$capture_root/boundary-gnu-check.stderr"
stage boundary-clang-build /usr/bin/clang -std=c11 -O0 -Wall -Wextra -Werror -fsanitize=address,undefined "$fixture/gcvt-boundary-check.c" -o "$capture_root/gcvt-clang"
stage boundary-clang-check /usr/bin/env ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 "$capture_root/gcvt-clang"
cmp "$capture_root/expected-boundary.stdout" "$capture_root/boundary-clang-check.stdout"
test ! -s "$capture_root/boundary-clang-check.stderr"
stage import /usr/bin/llvm-dlltool-21 -m i386 -k -d "$fixture/crt-kernel32.def" -l "$capture_root/crt-kernel32.lib"
stage compile /usr/bin/clang --target=i686-pc-windows-msvc -std=c11 -O0 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Werror -c "$fixture/gcvt-probe.c" -o "$capture_root/gcvt-probe.obj"
stage link "$linker" -flavor link /dll /nodefaultlib /machine:x86 /entry:CrtEntry@12 "/out:$capture_root/gcvt-probe.fll" "$capture_root/gcvt-probe.obj" "$capture_root/crt-kernel32.lib" "$sdk" "$legacy/kernel32-stdcall.lib" "$legacy/msvcrt.lib"
sha256sum -c "$capture_root/build-inputs.sha256" > "$capture_root/build-inputs-check.txt"
sha256sum "$capture_root/gcvt-probe.fll" "$capture_root/gcvt-probe.obj" "$capture_root/crt-kernel32.lib" "$capture_root/binding-gnu" "$capture_root/binding-clang" "$capture_root/gcvt-gnu" "$capture_root/gcvt-clang" > "$capture_root/built.sha256"
file "$capture_root/gcvt-probe.fll" > "$capture_root/built-file.txt"
objdump -p "$capture_root/gcvt-probe.fll" > "$capture_root/built-pe.txt"
date -u +%FT%TZ > "$capture_root/build.finished.utc"
