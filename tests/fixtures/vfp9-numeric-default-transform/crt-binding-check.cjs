// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Compare EXACT header compiled by each host compiler against Node crypto.
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),crypto=require('node:crypto'),a=require('node:assert/strict');
const binary=process.argv[2],root=process.argv[3],log=[];
a(binary&&root);
function run(args){
 const prefix=cp.spawnSync('pgrep',['-a','-x','wine|wine64|wine-preloader|wineserver|wineserver64|vfp9.exe'],{encoding:'utf8'});
 const pressure=fs.readFileSync('/proc/pressure/io','utf8'),full=pressure.split('\n').find(s=>s.startsWith('full '));
 const values=full&&Object.fromEntries(full.split(' ').slice(1).map(s=>s.split('=')));
 const entry={args,prefixStatus:prefix.status,prefixOutput:prefix.stdout,pressure,started:new Date().toISOString()};log.push(entry);
 fs.writeFileSync(path.join(root,path.basename(binary)+'.checks.json'),JSON.stringify(log,null,2)+'\n');
 if(prefix.status!==1 || !values || !(Number(values.avg10)<1&&Number(values.avg60)<1))process.exit(prefix.status!==1?76:75);
 const result=cp.spawnSync(binary,args,{encoding:'utf8'});Object.assign(entry,{status:result.status,stdout:result.stdout,stderr:result.stderr,finished:new Date().toISOString()});
 fs.writeFileSync(path.join(root,path.basename(binary)+'.checks.json'),JSON.stringify(log,null,2)+'\n');
 a.equal(result.status,0);a.equal(result.stderr,'');return result.stdout;
}
a.equal(run(['--pe']),'sha-known-answers/pe-bounds PASS\n');
// Padding/multiblock boundaries, deterministic binary bytes, differing chunks.
for(const n of [0,1,3,55,56,63,64,65,119,120,127,128,129,4095,4096,4097,1000000]) {
 const bytes=Buffer.alloc(n);for(let i=0;i<n;++i)bytes[i]=n===1000000?97:(i*37+11)&255;
 const file=path.join(root,`sha-input-${n}.bin`);if(fs.existsSync(file))a.deepEqual(fs.readFileSync(file),bytes);else fs.writeFileSync(file,bytes,{flag:'wx'});
 const digest=crypto.createHash('sha256').update(bytes).digest('hex');
 for(const chunk of [1,55,64,4096])a.equal(run([file,String(chunk)]),digest+'\n');
 if(n===1000000)a.equal(digest,'cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0');
}
const crt='/home/rich/copperfin-wine-vfp9/drive_c/windows/syswow64/msvcr71.dll';
const digest=crypto.createHash('sha256').update(fs.readFileSync(crt)).digest('hex');
a.equal(digest,'8094af5ee310714caebccaeee7769ffb08048503ba478b879edfef5f1a24fefe');
for(const chunk of [1,63,4096])a.equal(run([crt,String(chunk)]),digest+'\n');
console.log(JSON.stringify({compilerBinary:binary,invocations:log.length,knownAnswersAndIndependentDigests:true,peBounds:true,crtSha256:digest}));
