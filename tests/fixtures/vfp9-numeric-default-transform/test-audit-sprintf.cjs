// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Synthetic complete-run reader tests only; no native observations.
'use strict';
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),crypto=require('node:crypto'),a=require('node:assert/strict');
const fixture=__dirname,inv=JSON.parse(fs.readFileSync(path.join(fixture,'sprintf-cases.json'))),root=fs.mkdtempSync(path.join(fixture,'../../../build/sprintf-reader-synthetic-'));
const sha=b=>crypto.createHash('sha256').update(b).digest('hex'),hex=s=>Buffer.from(s,'ascii').toString('hex').toUpperCase(),h=(n,w)=>n.toString(16).toUpperCase().padStart(w,'0');
const moduleHex=hex(inv.modulePath),identity=(bits)=>`real=${bits};cw0=027F;cw1=027F;module=${moduleHex};fileSample=ONCE;fileSize=${h(inv.moduleFileSize,8)};fileSha256=${inv.moduleSha256.toUpperCase()};machine=${h(inv.modulePe.machine,4)};magic=${h(inv.modulePe.magic,4)};stamp=${h(inv.modulePe.stamp,8)};imageSize=${h(inv.modulePe.imageSize,8)};checksum=${h(inv.modulePe.checksum,8)}`;
const metadata=bits=>`type=4E;width=0028;decimals=00000012;integer=00000000;real=${bits};cw=027F`;
const control=bits=>`kind=F;count=00000011;real=${bits};decimal=00000000;sign=00000000;cw0=027F;cw1=027F;module=${moduleHex};fileSample=ONCE;fileSize=${h(inv.moduleFileSize,8)};fileSha256=${inv.moduleSha256.toUpperCase()};machine=${h(inv.modulePe.machine,4)};magic=${h(inv.modulePe.magic,4)};stamp=${h(inv.modulePe.stamp,8)};imageSize=${h(inv.modulePe.imageSize,8)};checksum=${h(inv.modulePe.checksum,8)};digits=30`;
function crt(bits,count,s,n=s){const grammar=x=>Number(new RegExp(`^[+-]?[0-9]+\\.[0-9]{${count}}$`).test(x));return `kind=S;count=${h(count,8)};${identity(bits)};capacity=00000800;canary=PASS;sprintfGrammar=${grammar(s)};snprintfGrammar=${grammar(n)};equal=${s===n?1:0};sprintf=${hex(s)};snprintf=${hex(n)}`;}
const rows=[];for(const row of inv.inputs)for(const count of inv.counts){const expected=row.expectedStrHex[String(count)],text=Buffer.from(expected,'hex').toString('ascii').trimStart(),m=metadata(row.bits),ctl=control(row.bits);rows.push([String(row.index),row.bits,String(row.setting),String(count),m,row.bits,expected,ctl,crt(row.bits,count,text),m,row.bits,expected,ctl,'N',hex(String(row.setting)),'C',hex('OFF'),'OFF']);}
a.equal(rows.length,1056);a(rows.every(f=>f.length===18));const report=rs=>Buffer.from('VERSION|Visual FoxPro 09.00.0000.7423 for Windows\r\n'+rs.map(f=>f.join('|')).join('\r\n')+'\r\nDONE|1056\r\n','ascii'),baseline=report(rows);
function writeRun(name,raw){fs.writeFileSync(path.join(root,name+'-results.txt'),raw);fs.writeFileSync(path.join(root,name+'.out'),raw);fs.writeFileSync(path.join(root,name+'.status'),'0\n');fs.writeFileSync(path.join(root,name+'.stderr'),'');}
function reset(){writeRun('first',baseline);writeRun('repeat',baseline);fs.writeFileSync(path.join(root,'supplied-runtime-env.txt'),inv.runtimeEnvironment+'\n');fs.writeFileSync(path.join(root,'inherited-dll-overrides.txt'),'UNSET\n');fs.writeFileSync(path.join(root,'first-structure.status'),'0\n');fs.writeFileSync(path.join(root,'first-structure.stderr'),'');fs.writeFileSync(path.join(root,'first-structure.json'),JSON.stringify({firstComplete:true,records:1056,firstSha256:sha(baseline),mismatchCount:0,mismatches:[],pairedQualified:false}));}
let checks=0;function run(ok,single=false){const r=cp.spawnSync(process.execPath,[path.join(fixture,'audit-sprintf.cjs'),root,...(single?['--single']:[])],{encoding:'utf8',maxBuffer:16*1024*1024});a.equal(r.signal,null);if(ok){a.equal(r.status,0,r.stderr);a.equal(r.stderr,'');}else a.notEqual(r.status,0,'mutation unexpectedly accepted');++checks;return r;}
try{
 reset();run(true,true);run(true);
 reset();fs.writeFileSync(path.join(root,'first.status'),'124\n');run(false,true);
 reset();fs.writeFileSync(path.join(root,'first.stderr'),'error\n');run(false,true);
 reset();fs.writeFileSync(path.join(root,'supplied-runtime-env.txt'),'WINEDLLOVERRIDES=msvcr71=b\n');run(false,true);
 reset();writeRun('first',Buffer.from(baseline.toString().replace('DONE|1056','DONE|1055')));run(false,true);
 reset();{const changed=rows.map(f=>[...f]);changed[0][5]='0000000000000000';writeRun('first',report(changed));run(false,true);}
 reset();fs.writeFileSync(path.join(root,'first-structure.json'),JSON.stringify({firstComplete:true,records:1056,firstSha256:'0'.repeat(64),mismatchCount:0,mismatches:[],pairedQualified:false}));run(false);
 reset();{const changed=rows.map(f=>[...f]);changed[0][8]=crt(changed[0][1],15,'1.000000000000001',Buffer.from(changed[0][6],'hex').toString('ascii').trimStart());writeRun('repeat',report(changed));run(false);}
 reset();{const changed=rows.map(f=>[...f]);changed[0][8]=crt(changed[0][1],15,'1.000000000000001',Buffer.from(changed[0][6],'hex').toString('ascii').trimStart());writeRun('first',report(changed));const reportObject=JSON.parse(run(true,true).stdout);a.equal(reportObject.mismatchCount,2);a(reportObject.mismatches.every(m=>m.rawText==='1.000000000000001'));}
 reset();{const changed=rows.map(f=>[...f]);changed[0][4]=changed[0][4].replace('integer=00000000','integer=FFFFFFFF');changed[0][9]=changed[0][4];writeRun('repeat',report(changed));const reportObject=JSON.parse(run(true).stdout);a.equal(reportObject.inactiveSlotDifferences,2);a.equal(reportObject.rawByteEqual,false);}
 console.log(JSON.stringify({syntheticCompleteReaderChecks:checks,nativeInvoked:false,firstGateRequired:true,mismatchesPreserved:true,inactiveIntegerSlotsOnly:true}));
}finally{for(const name of fs.readdirSync(root))fs.unlinkSync(path.join(root,name));fs.rmdirSync(root);}
