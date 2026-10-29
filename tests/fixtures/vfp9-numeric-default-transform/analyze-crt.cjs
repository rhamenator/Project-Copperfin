// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Offline observations/counterexamples ONLY. No expected formatter or production rule.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),crypto=require('node:crypto');
const root=process.argv[2];a(root);
const bytes=(dir,name)=>{const p=path.join(dir,name);return fs.existsSync(p)?fs.readFileSync(p):z.gunzipSync(fs.readFileSync(p+'.gz'));};
const current=JSON.parse(bytes(root,'audit.json'));
a.equal(current.recordsPerRun,4176);a.equal(current.documentedFieldsRepeatEqual,true);a.equal(current.metadataAndBRSExact,true);a.equal(current.settingsAndRuntimeModuleExact,true);
for(const name of ['first','repeat'])a.equal(crypto.createHash('sha256').update(bytes(root,name+'-results.txt')).digest('hex'),current[name+'Sha256']);
const prior=JSON.parse(bytes(path.join(__dirname,'rounding-arithmetic-captures'),'audit.json'));
a.equal(current.observations.length,4176);a.equal(prior.observations.length,528);
a.equal(prior.firstSha256,'2917ff9167e4a2752520614776580d08dda8fa4ba1520615154912bd937bd1c9');
const original=new Map();
for(const r of current.observations.filter(r=>r.route==='ORIGINAL')){
 const key=JSON.stringify([r.index,r.fixed]);
 if(original.has(key))a.equal(original.get(key).displayHex,r.displayHex);else original.set(key,r);
}
let compared=0,changed=0;
a.equal(original.size,528);
for(const r of prior.observations){const n=original.get(JSON.stringify([r.index,r.fixed]));a(n);a.equal(n.bits,r.bits);a.equal(n.setting,r.setting);++compared;if(n.displayHex!==r.displayHex)++changed;}
const collisions=[];
for(const route of ['ORIGINAL','SHIFTED'])for(const kindCount of ['F','E15','E16','E17']){
 const groups=new Map();let rows=0;
 for(const r of current.observations){
  if(r.route!==route||(r.kind==='F'?'F':'E'+r.count)!==kindCount)continue;
  ++rows;const target=original.get(JSON.stringify([r.index,r.fixed]));a(target);
  // Deliberately test sufficiency of ONLY public conversion output, not input bits.
  // Historical SHIFTED stimuli are magnitudes: original sign remains explicit.
  const key=JSON.stringify([r.fixed,r.setting,40,18,BigInt('0x'+r.bits)>>63n===1n,r.kind,r.count,r.crt.decimal,r.crt.sign,r.crt.digits]);
  const group=groups.get(key)||[];group.push({index:r.index,argumentBits:r.argumentBits,originalBits:r.bits,displayHex:target.displayHex});groups.set(key,group);
 }
 const bad=[...groups].filter(([,g])=>new Set(g.map(r=>r.displayHex)).size>1);
 collisions.push({route,kindCount,rows,distinctKeys:groups.size,conflictingKeys:bad.length,counterexamples:bad.map(([key,observations])=>({key:JSON.parse(key),observations}))});
}
console.log(JSON.stringify({analysisOnly:true,sourceSha256:current.firstSha256,previousDisplayComparison:{compared,changed,scope:'ONLY528 original-route contexts at identical explicit payload/settings; not earlier-phase/Windows retroqualification'},collisions,interpretation:'Conflicting keys reject sufficiency of that conversion-output tuple for original display in this finite sample. No VFP internal call attribution or accepted conversion/rounding rule. SHIFTED arguments remain explicit historical stimuli, not newly observed arithmetic.'},null,2));
