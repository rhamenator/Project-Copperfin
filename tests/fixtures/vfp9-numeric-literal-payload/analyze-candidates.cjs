// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Hypothesis comparison only; never an expected-value emitter/product oracle.
const fs=require('node:fs'),a=require('node:assert/strict');
const inventory=JSON.parse(fs.readFileSync(__dirname+'/cases.json'));
const native=JSON.parse(fs.readFileSync(process.argv[2]));
const bits=n=>{const b=Buffer.alloc(8);b.writeDoubleBE(n);return b.toString('hex').toUpperCase();};
const models={
    directDecimal:p=>Number(p.literal),
    roundedCoefficientDivide:p=>Number(p.digits)/Number('1e'+p.scale),
    roundedCoefficientReciprocal:p=>Number(p.digits)*Number('1e-'+p.scale),
    hornerCoefficientDivide:p=>p.horner/Number('1e'+p.scale),
    hornerCoefficientReciprocal:p=>p.horner*Number('1e-'+p.scale),
    splitIntegerFractionDivide:p=>Number(p.whole)+Number(p.fraction)/Number('1e'+p.scale),
    splitIntegerFractionReciprocal:p=>Number(p.whole)+Number(p.fraction)*Number('1e-'+p.scale),
    chunkNineCoefficientReciprocal:p=>p.chunks*Number('1e-'+p.scale),
};
const results=Object.fromEntries(Object.keys(models).map(name=>[name,{compared:0,mismatches:0,counterexamples:[]}]));
const nativeDirectDifferences=[],inputRecords=[];
for(const row of inventory.rows.filter(r=>r.wrapper==='literal')) {
    const records=native.observations.filter(o=>o.index===row.index);
    a.equal(records.length,8);
    const payloads=new Set(records.map(o=>o.result.bits));a.equal(payloads.size,1);
    const actual=[...payloads][0];
    const [whole,fraction]=row.literal.split('.'),digits=whole+fraction;
    let horner=0;for(const digit of digits)horner=horner*10+Number(digit);
    let chunks=0;const normalized=digits.replace(/^0+/,'')||'0';
    let first=normalized.length%9||9;
    for(let start=0;start<normalized.length;) {
        const size=start===0?first:9;
        chunks=chunks*Number('1e'+size)+Number(normalized.slice(start,start+size));start+=size;
    }
    const input={literal:row.literal,whole,fraction,digits,scale:fraction.length,horner,chunks};
    inputRecords.push({literal:row.literal,native:actual,index:row.index});
    if(bits(Number(row.literal))!==actual)nativeDirectDifferences.push({literal:row.literal,native:actual,direct:bits(Number(row.literal))});
    for(const [name,model] of Object.entries(models)) {
        const result=results[name],predicted=bits(model(input));++result.compared;
        if(predicted!==actual){++result.mismatches;result.counterexamples.push({literal:row.literal,native:actual,predicted});}
    }
}
a.equal(inputRecords.length,245);
console.log(JSON.stringify({
    scope:'Finite245 bare-literal payloads from audited native records; no model is accepted as an implementation requirement.',
    results,nativeDirectDifferences,inputRecords,
    limitations:'No product code, expected test data or conversion API behavior is imported/generated. Unary/materialization metadata and wider producer boundaries require separate recovery. A zero counterexample count on this profile is not universal equivalence.'},null,2));
