#!/bin/bash
# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Source-frozen native build; does not invoke Wine/VFP or any VM.
set -euo pipefail
test "${1-}" = --fresh-resource-clear || exit 93
cd /home/rich/.codex/worktrees/numeric-literal-display-5611/Project-Copperfin
capture_root=build/default-transform-sprintf
fixture=tests/fixtures/vfp9-numeric-default-transform
# Fail before creating a partial build directory when the shared native prefix
# or measured host capacity is unavailable. Per-stage guards still revalidate.
set +e
pgrep -a -x 'wine|wine64|wine-preloader|wineserver|wineserver64|vfp9.exe' >/dev/null
initial_prefix_status=$?
awk '$1=="full" {split($2,a,"=");split($3,b,"=");found=1;if(a[2]>=1||b[2]>=1)exit 75} END {if(!found)exit 75}' /proc/pressure/io
initial_io_status=$?
awk '$1=="MemAvailable:" {found=1;if($2<8388608)exit 75} END {if(!found)exit 75}' /proc/meminfo
initial_memory_status=$?
set -e
test "$initial_prefix_status" -eq 1 || exit 76
test "$initial_io_status" -eq 0 || exit 75
test "$initial_memory_status" -eq 0 || exit 75
test ! -e "$capture_root" || exit 96
mkdir "$capture_root"
git status --porcelain > "$capture_root/build-source-status.txt"
test ! -s "$capture_root/build-source-status.txt" || exit 97
git rev-parse HEAD > "$capture_root/build-source-commit.txt"
date -u +%FT%TZ > "$capture_root/build.started.utc"
sdk='/home/rich/copperfin-wine-vfp9/drive_c/Program Files (x86)/Microsoft Visual FoxPro 9/Samples/API/winapims.lib'
legacy=/home/rich/dev/Project-Copperfin/build/pr7203-completed/native-evidence/diagnostic
linker=/home/rich/.rustup/toolchains/stable-x86_64-unknown-linux-gnu/lib/rustlib/x86_64-unknown-linux-gnu/bin/rust-lld
sources='gcvt-probe.c gcvt-boundary.h sprintf-boundary.h sprintf-boundary-check.c sprintf-source-check.cjs emit-sprintf.cjs sprintf.prg sprintf-cases.json test-emit-sprintf.cjs read-sprintf-record.cjs test-read-sprintf-record.cjs audit-sprintf.cjs test-audit-sprintf.cjs sprintf-build.sh sprintf-capture.sh check-sprintf-worker.cjs crt-binding.h crt-binding-check.c crt-binding-check.cjs crt-kernel32.def crt-cases.json crt.prg'
for name in $sources; do cp "$fixture/$name" "$capture_root/source-$name"; done
sha256sum $(for name in $sources; do printf '%s/%s\n' "$fixture" "$name"; done) "$sdk" "$legacy/kernel32-stdcall.lib" "$legacy/msvcrt.lib" /usr/bin/gcc /usr/bin/clang /usr/bin/llvm-dlltool-21 "$linker" /home/rich/copperfin-wine-vfp9/drive_c/windows/syswow64/msvcr71.dll > "$capture_root/build-inputs.sha256"

stage() {
 local label=$1 prefix_status guard_status
 shift
 date -u +%FT%TZ > "$capture_root/$label.started.utc"
 sha256sum -c "$capture_root/build-inputs.sha256" > "$capture_root/$label.inputs-before.txt"
 set +e
 pgrep -a -x 'wine|wine64|wine-preloader|wineserver|wineserver64|vfp9.exe' > "$capture_root/$label.prefix.txt"
 prefix_status=$?
 set -e
 printf '%s\n' "$prefix_status" > "$capture_root/$label.prefix.status"
 test "$prefix_status" -eq 1 || exit 76
 cat /proc/pressure/io > "$capture_root/$label.io-pressure-before.txt"
 set +e
 awk '$1=="full" {split($2,a,"=");split($3,b,"=");found=1;if(a[2]>=1||b[2]>=1)exit 75} END {if(!found)exit 75}' "$capture_root/$label.io-pressure-before.txt"
 guard_status=$?
 set -e
 printf '%s\n' "$guard_status" > "$capture_root/$label.guard.status"
 test "$guard_status" -eq 0 || exit 75
 set +e
 nice -n 10 "$@" > "$capture_root/$label.stdout" 2> "$capture_root/$label.stderr"
 local status=$?
 set -e
 printf '%s\n' "$status" > "$capture_root/$label.status"
 cat /proc/pressure/io > "$capture_root/$label.io-pressure-after.txt"
 date -u +%FT%TZ > "$capture_root/$label.finished.utc"
 test "$status" -eq 0 || exit "$status"
 test ! -s "$capture_root/$label.stderr" || exit 98
 sha256sum -c "$capture_root/build-inputs.sha256" > "$capture_root/$label.inputs-after.txt"
}

stage source-boundary node "$fixture/sprintf-source-check.cjs"
stage source-emitter node "$fixture/test-emit-sprintf.cjs"
stage source-record node "$fixture/test-read-sprintf-record.cjs"
stage source-audit node "$fixture/test-audit-sprintf.cjs"
stage source-worker node "$fixture/check-sprintf-worker.cjs"
stage host-gnu /usr/bin/gcc -std=c11 -O0 -Wall -Wextra -Werror -fsanitize=address,undefined "$fixture/crt-binding-check.c" -o "$capture_root/binding-gnu"
stage check-gnu /usr/bin/env ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 node "$fixture/crt-binding-check.cjs" "$capture_root/binding-gnu" "$capture_root"
stage host-clang /usr/bin/clang -std=c11 -O0 -Wall -Wextra -Werror -fsanitize=address,undefined "$fixture/crt-binding-check.c" -o "$capture_root/binding-clang"
stage check-clang /usr/bin/env ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 node "$fixture/crt-binding-check.cjs" "$capture_root/binding-clang" "$capture_root"
stage boundary-gnu-build /usr/bin/gcc -std=c11 -O0 -Wall -Wextra -Werror -fsanitize=address,undefined "$fixture/sprintf-boundary-check.c" -o "$capture_root/sprintf-gnu"
stage boundary-gnu-check /usr/bin/env ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 "$capture_root/sprintf-gnu"
printf 'sprintf-boundary checks=45 PASS (stub only)\n' > "$capture_root/expected-boundary.stdout"
cmp "$capture_root/expected-boundary.stdout" "$capture_root/boundary-gnu-check.stdout"
stage boundary-clang-build /usr/bin/clang -std=c11 -O0 -Wall -Wextra -Werror -fsanitize=address,undefined "$fixture/sprintf-boundary-check.c" -o "$capture_root/sprintf-clang"
stage boundary-clang-check /usr/bin/env ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 "$capture_root/sprintf-clang"
cmp "$capture_root/expected-boundary.stdout" "$capture_root/boundary-clang-check.stdout"
stage import /usr/bin/llvm-dlltool-21 -m i386 -k -d "$fixture/crt-kernel32.def" -l "$capture_root/crt-kernel32.lib"
stage compile /usr/bin/clang --target=i686-pc-windows-msvc -std=c11 -O0 -ffreestanding -fno-builtin -fno-stack-protector -Wall -Wextra -Werror -c "$fixture/gcvt-probe.c" -o "$capture_root/sprintf-probe.obj"
stage link "$linker" -flavor link /dll /nodefaultlib /machine:x86 /entry:CrtEntry@12 "/out:$capture_root/sprintf-probe.fll" "$capture_root/sprintf-probe.obj" "$capture_root/crt-kernel32.lib" "$sdk" "$legacy/kernel32-stdcall.lib" "$legacy/msvcrt.lib"
sha256sum -c "$capture_root/build-inputs.sha256" > "$capture_root/build-inputs-check.txt"
sha256sum "$capture_root"/{sprintf-probe.fll,sprintf-probe.obj,crt-kernel32.lib,binding-gnu,binding-clang,sprintf-gnu,sprintf-clang} > "$capture_root/built.sha256"
file "$capture_root/sprintf-probe.fll" > "$capture_root/built-file.txt"
objdump -p "$capture_root/sprintf-probe.fll" > "$capture_root/built-pe.txt"
date -u +%FT%TZ > "$capture_root/build.finished.utc"
