// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Offline tuple insufficiency check, NOT an expected formatter or native-call attribution.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),crypto=require('node:crypto');
const root=process.argv[2];a(root);
const bytes=(dir,name)=>{const p=path.join(dir,name);return fs.existsSync(p)?fs.readFileSync(p):z.gunzipSync(fs.readFileSync(p+'.gz'));};
const current=JSON.parse(bytes(root,'audit.json')),parentRoot=path.join(__dirname,'crt-nativeonly-captures1'),parent=JSON.parse(bytes(parentRoot,'audit.json'));
a.equal(current.recordsPerRun,1584);a.equal(current.observations.length,1584);
a.equal(current.documentedFieldsRepeatEqual,true);a.equal(current.metadataAndBRSExact,true);a.equal(current.settingsAndRuntimeModuleExact,true);
for(const n of ['first','repeat'])a.equal(crypto.createHash('sha256').update(bytes(root,n+'-results.txt')).digest('hex'),current[n+'Sha256']);
const parentHash='60a1b217bbe305ccb61012cb4324d7bb944d9e413bd920803debd17e36e8c54d';
for(const n of ['first','repeat']){a.equal(parent[n+'Sha256'],parentHash);a.equal(crypto.createHash('sha256').update(bytes(parentRoot,n+'-results.txt')).digest('hex'),parentHash);}
const previous=new Map(parent.observations.filter(r=>r.route==='ORIGINAL'&&r.kind==='E'&&r.count===17).map(r=>[JSON.stringify([r.index,r.fixed]),r]));a.equal(previous.size,528);
const contexts=new Map();let changedDisplays=0,changedStr=0;
for(const r of current.observations){
 a.equal(r.route,'ORIGINAL');a.equal(r.kind,'F');const offset=r.count-r.setting;a([1,2,3].includes(offset));
 const key=JSON.stringify([r.index,r.fixed]),old=previous.get(key);a(old);a.equal(r.bits,old.bits);a.equal(r.setting,old.setting);
 const context=contexts.get(key)||[];a(!context.some(x=>x.count===r.count));context.push(r);contexts.set(key,context);
}
a.equal(contexts.size,528);
for(const rows of contexts.values()){
 a.equal(rows.length,3);rows.sort((x,y)=>x.count-y.count);const r=rows[0],old=previous.get(JSON.stringify([r.index,r.fixed]));
 for(const x of rows){a.equal(x.displayHex,r.displayHex);a.equal(x.strHex,r.strHex);}
 if(r.displayHex!==old.displayHex)++changedDisplays;if(r.strHex!==old.strHex)++changedStr;
}
const tuple=r=>[r.count,r.crt.decimal,r.crt.sign,r.crt.digits];
const results=[];
for(const mode of [1,2,3,'ALL'])for(const fixed of ['OFF','ON']){
 const groups=new Map();let rows=0;
 for(const context of contexts.values()){
  const r=context[0];if(r.fixed!==fixed)continue;++rows;
  const selected=mode==='ALL'?context:context.filter(x=>x.count-x.setting===mode);
  const key=JSON.stringify([r.fixed,r.setting,40,18,...selected.map(tuple)]),g=groups.get(key)||[];
  g.push({index:r.index,bits:r.bits,displayHex:r.displayHex,strHex:r.strHex});groups.set(key,g);
 }
 const conflicts=[...groups].filter(([,g])=>new Set(g.map(r=>r.displayHex)).size>1);
 results.push({offset:mode,fixed,contexts:rows,distinctKeys:groups.size,conflictingKeys:conflicts.length,counterexamples:conflicts.map(([key,observations])=>({key:JSON.parse(key),observations}))});
}
console.log(JSON.stringify({analysisOnly:true,sourceSha256:current.firstSha256,previousContextComparison:{contexts:contexts.size,changedDisplays,changedStr,scope:'ONLY this528-context overlap with the bound4176 pair; not earlier-phase or Windows retroqualification'},results,interpretation:'Conflicting output tuples reject their sufficiency for this finite original-display sample, including the combined three-count tuple. No guessed tolerance, cap, internal VFP API use or accepted formatting rule follows.'},null,2));
