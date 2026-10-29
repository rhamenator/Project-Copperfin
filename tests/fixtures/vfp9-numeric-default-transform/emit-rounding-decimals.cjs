// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Broaden exact native-edge observations across DECIMALS; stimuli are not answers.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib');
const a=require('node:assert/strict');
const prior=JSON.parse(fs.readFileSync(path.join(__dirname,'rounding-edge-cases.json')));
const observed=JSON.parse(zlib.gunzipSync(fs.readFileSync(path.join(__dirname,'rounding-edge-captures/audit.json.gz'))));
a.equal(observed.firstSha256,'b6e43abea8d3290d46e231a72e0ec6807c25551b266cfa156d91f393f4baacc2');
a.equal(observed.repeatSha256,observed.firstSha256);a.equal(observed.rawByteEqual,true);
const groups=[];
for(let setting=0;setting<=18;setting++){
    const expression='5e-'+(setting+1),number=Number(expression),buffer=Buffer.alloc(8);
    a(Number.isFinite(number)&&number>0);buffer.writeDoubleBE(number);
    const bits=buffer.toString('hex').toUpperCase();
    for(const negative of [false,true])groups.push({id:groups.length+1,center:setting+1,expression,setting,bits,negative});
}
const inv={...prior,parentNativeSha256:observed.firstSha256,groups,
    limitation:'Exact native-feedback edges at 19 binary64 near-half stimuli, DECIMALS0..18/both signs, width40/scale18, both FIXED observations per query. Host Number only selects explicit finite input bits, never a native expected string. Reader requires every bit/readback/path and adjacent different-output endpoints. No monotonicity, uniqueness, magnitude-wide, descriptor-wide or conversion-algorithm claim.'};
let source=fs.readFileSync(path.join(__dirname,'rounding-edge.prg'),'utf8');
const start=source.indexOf('LOCAL ARRAY baseHex'),end=source.indexOf('LOCAL outputPath');a(start>0&&end>start);
const decl=`LOCAL ARRAY baseHex[38],negative[38],setting[38]\n`+groups.map(g=>`baseHex[${g.id}]='${g.bits}'\nnegative[${g.id}]=${g.negative?'.T.':'.F.'}\nsetting[${g.id}]=${g.setting}`).join('\n')+'\n';
source=source.slice(0,start)+decl+source.slice(end);
source=source.replace('FOR groupNo=1 TO 10','FOR groupNo=1 TO 38');
source=source.replace('default-transform-rounding-edge\\','default-transform-rounding-decimals\\');
source=source.replace('native-feedback local boundary recovery.','native-feedback DECIMALS0..18 boundary stimuli.');
a(source.includes('FOR groupNo=1 TO 38'));a(source.includes('default-transform-rounding-decimals\\results.txt'));
if(process.argv[2]==='source')process.stdout.write(source);
else if(process.argv[2]==='cases')process.stdout.write(JSON.stringify(inv,null,2)+'\n');
else console.log(JSON.stringify({groups:groups.length,maxQueriesPerGroup:54,maxRecords:4104}));
