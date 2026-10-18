// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Forensic guard/build receipt check ONLY: never native/pair qualification.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict');
const root=process.argv[2];a(root);
const exists=n=>fs.existsSync(path.join(root,n))||fs.existsSync(path.join(root,n+'.gz'));
const read=n=>{const p=path.join(root,n);return (fs.existsSync(p)?fs.readFileSync(p):z.gunzipSync(fs.readFileSync(p+'.gz'))).toString();};
const pressure=s=>{const line=s.split('\n').find(r=>r.startsWith('full '));a(line);return Object.fromEntries(line.split(' ').slice(1).map(p=>p.split('=')));};
for(const stage of ['host-gnu','check-gnu','host-clang','check-clang','import','compile','link']){
 a.equal(read(stage+'.status'),'0\n');a.equal(read(stage+'.guard.status'),'0\n');a.equal(read(stage+'.prefix.status'),'1\n');
 const p=pressure(read(stage+'.pressure-before.txt'));a(Number(p.avg10)<1&&Number(p.avg60)<1);
 if(stage!=='link')a.equal(read(stage+'.stderr'),'');
}
for(const compiler of ['gnu','clang']){
 const checks=JSON.parse(read('binding-'+compiler+'.checks.json'));a.equal(checks.length,72);
 for(const row of checks){a.equal(row.status,0);a.equal(row.stderr,'');a.equal(row.prefixStatus,1);const p=pressure(row.pressure);a(Number(p.avg10)<1&&Number(p.avg60)<1);}
 const report=JSON.parse(read('check-'+compiler+'.stdout'));a.equal(report.invocations,72);a.equal(report.knownAnswersAndIndependentDigests,true);a.equal(report.peBounds,true);
}
a(exists('build.finished.utc'));a.equal(read('build-source-status.txt'),'');a.equal(read('source-status.txt'),'');
a.equal(JSON.parse(read('build-shell-completion.json')).exit_code,0);a.equal(JSON.parse(read('native-shell-capture.json')).exit_code,75);
a.equal(read('first.guard.status'),'75\n');a.equal(read('first.prefix.status'),'1\n');
const p=pressure(read('first.pressure-before.txt'));a(Number(p.avg10)>=1||Number(p.avg60)>=1);
for(const n of ['first.status','first.out','first.stderr','first-results.txt','first-structure.status','repeat.started.utc','audit.status'])a(!exists(n));
console.log(JSON.stringify({forensicGuardReceipt:true,buildCompleted:true,directChecksPerCompiler:72,guard:75,fullAvg10:Number(p.avg10),fullAvg60:Number(p.avg60),nativeInvoked:false,pairedQualified:false,limitation:'Plain-shell worker guard flow/receipts only; no VFP observation, strict acceptance, REA evidence or descendant-cleanup attestation.'}));
