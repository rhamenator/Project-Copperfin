// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Synthetic harness records only; never native experiment evidence.
'use strict';
const a=require('node:assert/strict'),fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),cp=require('node:child_process');
const {audit}=require('./audit-str-ambient.cjs'),hex=s=>Buffer.from(s).toString('hex').toUpperCase();
const native=z.gunzipSync(fs.readFileSync(path.join(__dirname,'str-middle-captures1/first-results.txt.gz'))).toString('ascii').split('\r\n').slice(1,-2).map(l=>l.split('|'));
const lines=[];for(const [i,index]of [184,185,186,190,191,192].entries())for(const fixed of ['OFF','ON'])for(const ambientD of [0,13,18]){
 const original=native.find(f=>f[0]===fixed&&f[1]===String(index));a(original);const f=[...original];f[1]=String(i+1);
 f.push(String(index),String(ambientD),'N',hex(String(ambientD)),'C',hex(fixed),f[4],f[5],f[18],f[19],hex(String(ambientD)),hex(fixed));lines.push(f.join('|'));
}
const raw=Buffer.from(['VERSION|Visual FoxPro 09.00.0000.7423 for Windows',...lines,'DONE|36',''].join('\r\n'));
const generated=cp.spawnSync(process.execPath,[path.join(__dirname,'emit-str-ambient.cjs'),'source']);a.equal(generated.status,0);
function fixture(){const files=new Map([['supplied-source.prg',generated.stdout],['supplied-runtime-env.txt',Buffer.from('WINEDLLOVERRIDES=msvcr71=n\n')],['inherited-dll-overrides.txt',Buffer.from('UNSET\n')]]);
 for(const name of ['first','repeat'])for(const [suffix,b]of [['.status',Buffer.from('0\n')],['.stderr',Buffer.alloc(0)],['-results.txt',raw],['.out',raw]])files.set(name+suffix,b);
 const io={readFileSync:n=>{const key=path.basename(n);a(files.has(key),'missing '+key);return files.get(key);}};
 const first=audit('/synthetic',true,io);files.set('first-structure.status',Buffer.from('0\n'));files.set('first-structure.stderr',Buffer.alloc(0));files.set('first-structure.json',Buffer.from(JSON.stringify(first)));return {files,io};}
let checks=0;const good=fixture();a.equal(audit('/synthetic',false,good.io).pairedQualified,true);++checks;
for(const [name,value]of [['supplied-source.prg','wrong'],['supplied-runtime-env.txt',''],['inherited-dll-overrides.txt','msvcr71=b\n'],['first.status','1\n'],['first.stderr','error'],['first.out','wrong'],['repeat.status','1\n'],['repeat.stderr','error'],['repeat.out','wrong'],['first-structure.status','1\n'],['first-structure.stderr','error'],['first-structure.json','{}']]){
 const {files,io}=fixture();files.set(name,Buffer.from(value));a.throws(()=>audit('/synthetic',false,io));++checks;}
console.log(JSON.stringify({checks,passed:true,syntheticOnly:true,nativeInvoked:false}));
