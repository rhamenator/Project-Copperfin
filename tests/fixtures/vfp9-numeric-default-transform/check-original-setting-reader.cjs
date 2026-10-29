// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: independent original-setting reader tests.
// SYNTHETIC reader tests, NOT native observations/compatibility expected strings.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),cp=require('node:child_process'),a=require('node:assert/strict'),c=require('node:crypto');
const root=fs.mkdtempSync(path.join(__dirname,'../../../build/original-setting-reader-synthetic-')),inv=JSON.parse(fs.readFileSync(path.join(__dirname,'original-setting-consumer-cases.json'))),sha=b=>c.createHash('sha256').update(b).digest('hex');
const parent=JSON.parse(z.gunzipSync(fs.readFileSync(path.join(__dirname,'round-inline-captures1/audit.json.gz'))));
const raw=z.gunzipSync(fs.readFileSync(path.join(__dirname,'round-inline-captures1/first-results.txt.gz'))).toString('ascii').split('\r\n').slice(1,-2).map(l=>l.split('|'));
const meta=(bits,scale,width=40)=>`type=4E;width=${width.toString(16).toUpperCase().padStart(4,'0')};decimals=${scale.toString(16).toUpperCase().padStart(8,'0')};integer=00000000;real=${bits};cw=027F`;
const rows=[];for(const r of inv.inputs)for(const fixed of ['OFF','ON']){
 const p=parent.observations.find(p=>p.index===r.parentIndex&&p.fixed===fixed),pf=raw.find(f=>f[0]===fixed&&f[1]===String(r.parentIndex));a(p&&pf);
 const original=[pf[4],p.bits,'32',Buffer.from(' '.repeat(39)+'2').toString('hex').toUpperCase(),pf[12]],round=[meta('0000000000000000',r.setting), '0000000000000000','33',Buffer.from(' '.repeat(39)+'3').toString('hex').toUpperCase()];
 // Deliberately arbitrary valid target text/digits: only reader structure/control is tested.
 const target=['N',meta(r.bits,r.scale,r.width),r.bits,'31',Buffer.from(' '.repeat(39)+'1').toString('hex').toUpperCase(),pf[12].replace(/real=[0-9A-F]{16}/,'real='+r.bits).replace(/digits=[0-9A-F]*$/,'digits=31')];
 rows.push([fixed,String(r.index),String(r.parentIndex),r.originalBits,r.bits,String(r.setting),String(r.scale),r.kind,...original,...round,'N',Buffer.from(String(r.setting)).toString('hex').toUpperCase(),'C',Buffer.from(fixed).toString('hex').toUpperCase(),...target,...original,...round,...target]);
}
const report=rows=>Buffer.from('VERSION|Visual FoxPro 09.00.0000.7423 for Windows\r\n'+rows.map(f=>f.join('|')).join('\r\n')+'\r\nDONE|32\r\n','ascii');
const baseline=report(rows);a.equal(rows.length,32);a(rows.every(f=>f.length===42));
function writeRun(n,b){fs.writeFileSync(path.join(root,n+'-results.txt'),b);fs.writeFileSync(path.join(root,n+'.out'),b);fs.writeFileSync(path.join(root,n+'.status'),'0\n');fs.writeFileSync(path.join(root,n+'.stderr'),'');}
function reset(){writeRun('first',baseline);writeRun('repeat',baseline);fs.writeFileSync(path.join(root,'supplied-runtime-env.txt'),inv.runtimeEnvironment+'\n');fs.writeFileSync(path.join(root,'inherited-dll-overrides.txt'),'UNSET\n');fs.writeFileSync(path.join(root,'first-structure.status'),'0\n');fs.writeFileSync(path.join(root,'first-structure.stderr'),'');fs.writeFileSync(path.join(root,'first-structure.json'),JSON.stringify({firstComplete:true,pairedQualified:false,firstSha256:sha(baseline)}));}
let checks=0;function run(expected,single=false){const r=cp.spawnSync(process.execPath,[path.join(__dirname,'audit-original-setting-consumer.cjs'),root,...(single?['--single']:[])],{encoding:'utf8',maxBuffer:2*1024*1024});a.equal(r.signal,null);if(expected===0){a.equal(r.status,0,r.stderr);a.equal(r.stderr,'');}else a.notEqual(r.status,0,'mutation unexpectedly accepted');++checks;return r;}
try{
 reset();run(0,true);run(0);
 for(const [col,value]of [[0,'ON'],[1,'999'],[2,'205'],[3,'0000000000000000'],[4,'0000000000000000'],[5,'15'],[6,'15'],[7,'other'],[8,meta(inv.inputs[0].originalBits,17)],[9,'0000000000000000'],[10,'31'],[11,'30'],[12,rows[0][12].replace('cw0=027F','cw0=037F')],[13,meta(rows[0][14],17)],[14,'3FF0000000000000'],[15,'31'],[16,'30'],[17,'C'],[18,'30'],[19,'N'],[20,'4F4E'],[21,'C'],[22,meta(inv.inputs[0].bits,17,inv.inputs[0].width)],[23,'0000000000000000'],[24,'00'],[25,'31'],[26,rows[0][26].replace('fileSample=ONCE','fileSample=OTHER')],[27,meta(inv.inputs[0].originalBits,17)],[32,meta(rows[0][14],17)],[36,'C'],[37,meta(inv.inputs[0].bits,17,inv.inputs[0].width)],[38,'0000000000000000'],[39,'30'],[40,'30'],[41,rows[0][41].replace(/real=[0-9A-F]{16}/,'real=0000000000000000')]]){
  reset();a.notEqual(value,rows[0][col],'mutation must change column '+col);const changed=rows.map(f=>[...f]);changed[0][col]=value;writeRun('first',report(changed));try{run(1,true);}catch(e){e.message='column '+col+': '+e.message;throw e;}
 }
 // Both target-scale fields must reject the other valid scale independently.
 for(const row of [0,2,4,6])for(const col of [22,37]){reset();const changed=rows.map(f=>[...f]),r=inv.inputs[row/2];changed[row][col]=meta(r.bits,r.scale===17?14:r.scale+1,40);a.notEqual(changed[row][col],rows[row][col]);writeRun('first',report(changed));run(1,true);}
 for(const col of [22,37]){reset();const changed=rows.map(f=>[...f]);changed[0][col]=meta(inv.inputs[0].bits,inv.inputs[0].scale,32);writeRun('first',report(changed));run(1,true);}
 reset();const wrongCount=Buffer.from(baseline.toString().replace('DONE|32','DONE|31'));a.notDeepEqual(wrongCount,baseline);writeRun('first',wrongCount);run(1,true);
 reset();fs.writeFileSync(path.join(root,'first.status'),'124\n');run(1,true);
 reset();fs.writeFileSync(path.join(root,'first.stderr'),'error\n');run(1,true);
 reset();fs.writeFileSync(path.join(root,'supplied-runtime-env.txt'),'WINEDLLOVERRIDES=msvcr71=b\n');run(1,true);
 reset();fs.writeFileSync(path.join(root,'inherited-dll-overrides.txt'),'msvcr71=n\n');run(1,true);
 reset();const changed=rows.map(f=>[...f]);changed[0][24]='32';changed[0][39]='32';writeRun('repeat',report(changed));run(1);
 reset();const inactive=rows.map(f=>[...f]);inactive[0][22]=inactive[0][22].replace('integer=00000000','integer=FFFFFFFF');inactive[0][37]=inactive[0][22];writeRun('repeat',report(inactive));const accepted=JSON.parse(run(0).stdout);a.equal(accepted.inactiveSlotDifferences,2);a.equal(accepted.rawByteEqual,false);

 // Synchronized before/after changes still must reject cross-metadata control drift.
 for(const cols of [[10,29],[15,34]]){reset();const changed=rows.map(f=>[...f]);for(const col of cols)changed[0][col]='34';writeRun('first',report(changed));run(1,true);}
 for(const cols of [[13,32]]){reset();const changed=rows.map(f=>[...f]);for(const col of cols)changed[0][col]=meta('0000000000000000',inv.inputs[0].setting,32);writeRun('first',report(changed));run(1,true);}
 // Coherent arbitrary finite ROUND tuples are observations, not D13 expectations.
 reset();const arbitrary=rows.map(f=>[...f]);for(const row of [0,2])for(const start of [13,32]){arbitrary[row][start]=meta('3FF0000000000000',7,32);arbitrary[row][start+1]='3FF0000000000000';arbitrary[row][start+2]='34';arbitrary[row][start+3]=Buffer.from(' '.repeat(39)+'4').toString('hex').toUpperCase();}writeRun('first',report(arbitrary));writeRun('repeat',report(arbitrary));fs.writeFileSync(path.join(root,'first-structure.json'),JSON.stringify({firstComplete:true,pairedQualified:false,firstSha256:sha(report(arbitrary))}));run(0,true);run(0);
 // Reject nonfinite ROUND payload even when synchronized across controls/metadata routes.
 for(const row of [0,2])for(const start of [13,32]){arbitrary[row][start]=meta('7FF0000000000000',7,32);arbitrary[row][start+1]='7FF0000000000000';}writeRun('first',report(arbitrary));run(1,true);
 reset();const fractional=rows.map(f=>[...f]);fractional[0][18]=Buffer.from('12.5').toString('hex').toUpperCase();writeRun('first',report(fractional));run(1,true);
 console.log(JSON.stringify({syntheticReaderSelfTests:checks,nativeInvoked:false,compatibilityExpectedStrings:false,baselineAndRejectedMutations:true,inactiveNumericIntegerSlotOnly:true,newSettingControlInvariance:true,arbitraryCoherentRoundTupleAccepted:true}));
}finally{for(const name of fs.readdirSync(root))fs.unlinkSync(path.join(root,name));fs.rmdirSync(root);}
