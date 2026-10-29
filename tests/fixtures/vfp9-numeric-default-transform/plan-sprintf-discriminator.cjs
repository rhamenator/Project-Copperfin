// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: select an exact old-CRT fixed-format discriminator.
'use strict';
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),crypto=require('node:crypto'),a=require('node:assert/strict');
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');
function plan(observations,expectedInputs=264){
 a.equal(observations.length,expectedInputs*2);const byKey=new Map(),inputs=[];
 for(const row of observations){a(['OFF','ON'].includes(row.fixed));const key=row.index+'|'+row.fixed;a(!byKey.has(key));byKey.set(key,row);}
 for(let index=1;index<=expectedInputs;++index){const off=byKey.get(index+'|OFF'),on=byKey.get(index+'|ON');a(off&&on);a.equal(off.bits,on.bits);a.equal(off.setting,on.setting);
  const expected={};for(const count of [15,16,17,18]){const field='str'+count+'Hex';a.equal(off[field],on[field]);expected[count]=off[field];}
  inputs.push({index,bits:off.bits,setting:off.setting,expectedStrHex:expected});
 }
 return {inputs:inputs.length,counts:[15,16,17,18],vfpControlRows:observations.length,vfpComparisons:observations.length*4,
  crtCalls:inputs.length*4,format:'%.*f',bufferBytes:2048,exports:['sprintf','_snprintf'],fixedControlsPreserved:['OFF','ON'],cases:inputs,
  limitation:'Public MSVCR71 fixed-format comparison only; no VFP internal-call attribution, formatter acceptance, Windows parity or production algorithm.'};
}
module.exports={plan};
if(require.main===module){
 const root=__dirname,archive=path.join(root,'str-middle-captures1'),gzip=fs.readFileSync(path.join(archive,'audit.json.gz'));
 const sums=new Map(fs.readFileSync(path.join(archive,'SHA256SUMS'),'utf8').trim().split('\n').map(line=>{const i=line.indexOf('  ');return[line.slice(i+2),line.slice(0,i)];}));
 a.equal(sha(gzip),sums.get('audit.json.gz'));const raw=z.gunzipSync(gzip),manifest=JSON.parse(fs.readFileSync(path.join(archive,'manifest.json')));
 const entry=manifest.files.filter(item=>item.name==='audit.json');a.equal(entry.length,1);a.equal(raw.length,entry[0].bytes);a.equal(sha(raw),entry[0].sha256);
 const audit=JSON.parse(raw);a.equal(audit.firstSha256,'13fba5c3663d1d44f0cf514d481abfc66e16063e3e300d1c8e33502b10282c6c');
 const report={sourceAuditDecodedSha256:sha(raw),moduleSha256:'8094af5ee310714caebccaeee7769ffb08048503ba478b879edfef5f1a24fefe',...plan(audit.observations)};
 process.stdout.write(JSON.stringify(report,null,2)+'\n');
}
