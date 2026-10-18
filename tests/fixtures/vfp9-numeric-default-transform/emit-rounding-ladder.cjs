// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Stimulus only: widen unbracketed native intervals, including both signs.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib');
const a=require('node:assert/strict');
const prior=JSON.parse(fs.readFileSync(path.join(__dirname,'rounding-cases.json')));
const observed=JSON.parse(zlib.gunzipSync(fs.readFileSync(path.join(__dirname,'rounding-captures/audit.json.gz'))));
a.equal(observed.firstSha256,'3c6601c1d769cd9bf662adf3068da1486a4f48e9993fb2b13524dfabcf627980');
a.equal(observed.repeatSha256,observed.firstSha256);a.equal(observed.metadataAndBRSIdentityExact,true);
const offsetsSet=new Set([0,1,-1]);
for(let exponent=0;exponent<=40;++exponent)for(const neighbor of [-1,0,1])
    offsetsSet.add(-(2**exponent)+neighbor);
const offsets=[...offsetsSet].sort((x,y)=>x-y);
const centers=prior.centers.map(({id,expression,setting,bits})=>({id,expression,setting,bits}));
const groups=[],inputs=[];
for(const center of centers)for(const negative of [false,true]){
    const group={id:groups.length+1,center:center.id,negative,first:inputs.length+1,setting:center.setting};
    for(const offset of offsets){
        const magnitude=BigInt('0x'+center.bits)+BigInt(offset);a(magnitude>0n&&magnitude<0x7ff0000000000000n);
        const bits=(magnitude|(negative?0x8000000000000000n:0n)).toString(16).padStart(16,'0').toUpperCase();
        a(Number.isFinite(Buffer.from(bits,'hex').readDoubleBE()));
        inputs.push({index:inputs.length+1,center:center.id,negative,offset,bits,setting:center.setting});
    }
    group.last=inputs.length;groups.push(group);
}
const inv={requirement:prior.requirement,parentNativeSha256:observed.firstSha256,
    centers,groups,offsets,inputCount:inputs.length,recordsPerRun:inputs.length*2,width:40,scale:18,
    limitation:'Finite signed magnitude-offset ladder around eight native payload centers, offsets at negative powers of two plus immediate integer-bit neighbors through2^40. This is a wider bracketing experiment, NOT an epsilon/tolerance/rounding algorithm. Exact native CTOBIN/BRS and width40/scale18 readbacks required. Both FIXED modes; no compiled-route, producer, all-input or implementation parity claim. No division/product/VM/FLL changes.'};
let source=fs.readFileSync(path.join(__dirname,'rounding.prg'),'utf8');
const array=/LOCAL ARRAY inputHex\[\d+\]\n[\s\S]+?(?=LOCAL outputPath,)/g;
a.equal([...source.matchAll(array)].length,1);
source=source.replace(array,`LOCAL ARRAY inputHex[${inputs.length}]\n${inputs.map(r=>`inputHex[${r.index}]='${r.bits}'`).join('\n')}\n`);
const targets=/ DO CASE\n[\s\S]+? ENDCASE\n SET DECIMALS TO decimalCount/g;
a.equal([...source.matchAll(targets)].length,1);
source=source.replace(targets,` DO CASE\n${groups.map(g=>` CASE index<=${g.last}\n  decimalCount=${g.setting}`).join('\n')}\n ENDCASE\n SET DECIMALS TO decimalCount`);
source=source.replace('default-transform-rounding\\','default-transform-rounding-ladder\\');
a(source.includes('default-transform-rounding-ladder\\results.txt'));
source=source.replace('native adjacent-payload stimulus.','native wide signed rounding ladder.');
if(process.argv[2]==='source')process.stdout.write(source);
else if(process.argv[2]==='cases')process.stdout.write(JSON.stringify(inv,null,2)+'\n');
else console.log(JSON.stringify({centers:centers.length,offsets:offsets.length,inputs:inputs.length,recordsPerRun:inv.recordsPerRun}));
