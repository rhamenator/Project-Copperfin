// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Forensic pre-launch guard receipt ONLY; never native qualification.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict');
const root=process.argv[2];a(root);
const exists=n=>fs.existsSync(path.join(root,n))||fs.existsSync(path.join(root,n+'.gz'));
const read=n=>{const p=path.join(root,n);return (fs.existsSync(p)?fs.readFileSync(p):z.gunzipSync(fs.readFileSync(p+'.gz'))).toString();};
a.equal(read('source-commit.txt'),'ec4e26cff57cee88254a4f75b1365a6287f6615c\n');a.equal(read('source-status.txt'),'');
a.equal(JSON.parse(read('native-shell-capture.json')).exit_code,75);
a.equal(read('first.guard.status'),'75\n');a.equal(read('first.prefix.status'),'1\n');a.equal(read('first.prefix.txt'),'');
a.equal(read('supplied-runtime-env.txt'),'WINEDLLOVERRIDES=msvcr71=n\n');a(['UNSET\n','\n'].includes(read('inherited-dll-overrides.txt')));
a.equal(read('reused-binary-check.txt'),'build/default-transform-crt-precision-captures1/crt-probe.fll: OK\n');
const line=read('first.pressure-before.txt').split('\n').find(r=>r.startsWith('full '));a(line);
const p=Object.fromEntries(line.split(' ').slice(1).map(v=>v.split('=')));a(Number(p.avg10)>=1||Number(p.avg60)>=1);
for(const n of ['first.status','first.out','first.stderr','first-results.txt','probe.fxp','results.txt','first-structure.status','repeat.started.utc','audit.status','finished.utc'])a(!exists(n));
console.log(JSON.stringify({forensicGuardReceipt:true,guard:75,fullAvg10:Number(p.avg10),fullAvg60:Number(p.avg60),nativeInvoked:false,pairedQualified:false,limitation:'Ordinary-shell worker pre-launch guard receipts only; no VFP execution, ROUND observation, REA Evidence or descendant-cleanup attestation.'}));
