// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Exact worker-source assertions only; never invokes Wine, VFP, compiler or VM.
'use strict';
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict'),worker=fs.readFileSync(path.join(__dirname,'sprintf-capture.sh'),'utf8');
const build=fs.readFileSync(path.join(__dirname,'sprintf-build.sh'),'utf8');
const before=(x,y)=>a(worker.indexOf(x)>=0&&worker.indexOf(x)<worker.indexOf(y),`${x} before ${y}`);let checks=0,has=x=>{a(worker.includes(x),x);++checks;};
const buildBefore=(x,y)=>a(build.indexOf(x)>=0&&build.indexOf(x)<build.indexOf(y),`${x} before ${y}`);
has('test "${1-}" = --fresh-resource-clear || exit 93');has('flock -n 9 || exit 74');before('flock -n 9','test ! -e "$capture_root/started.utc"');++checks;
has('test -z "${WINEDLLOVERRIDES-}" || exit 94');has('cmp "$capture_root/build-source-commit.txt" "$capture_root/source-commit.txt"');
has("if(a[2]>=1||b[2]>=1)exit 75");has('if($2<8388608)exit 75');has('nice -n 10 /usr/bin/env WINEDLLOVERRIDES=msvcr71=n /home/rich/bin/vfp9-probe');
has('if [ -f "$capture_root/results.txt" ]; then cp "$capture_root/results.txt" "$capture_root/$name-results.txt"; fi');has('test "$prefix_status" -eq 1 || exit 76');
has('sha256sum -c "$capture_root/inputs.sha256" > "$capture_root/$name.inputs-before.txt"');has('sha256sum -c "$capture_root/inputs.sha256" > "$capture_root/$name.inputs-after.txt"');
before('native_run first','audit-sprintf.cjs" "$capture_root" --single');++checks;before('audit-sprintf.cjs" "$capture_root" --single','native_run repeat');++checks;
has('test ! -s "$capture_root/first-structure.stderr" || exit 98');has('test ! -s "$capture_root/audit.stderr" || exit 98');
for(const forbidden of ['virsh','VBoxManage','qemu-system','rm -','wineserver -k','killall']){a.equal(worker.includes(forbidden),false,forbidden);++checks;}
a.equal((worker.match(/native_run repeat/g)||[]).length,1);++checks;
for(const token of ['test ! -e "$capture_root" || exit 96','source-audit node "$fixture/test-audit-sprintf.cjs"','boundary-clang-check','--target=i686-pc-windows-msvc','/entry:CrtEntry@12','/out:$capture_root/sprintf-probe.fll','sha256sum -c "$capture_root/build-inputs.sha256"']){a(build.includes(token),token);++checks;}
buildBefore('test "$initial_io_status" -eq 0 || exit 75','test ! -e "$capture_root" || exit 96');++checks;
buildBefore('test "$initial_memory_status" -eq 0 || exit 75','test ! -e "$capture_root" || exit 96');++checks;
for(const forbidden of ['vfp9-probe','WINEDLLOVERRIDES=msvcr71=n','virsh','VBoxManage','qemu-system']){a.equal(build.includes(forbidden),false,forbidden);++checks;}
console.log(JSON.stringify({exactWorkerSourceChecks:checks,nativeInvoked:false,compilerInvoked:false,vmInvoked:false,resourceLockAndFirstGate:true}));
