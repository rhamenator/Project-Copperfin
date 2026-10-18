// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Structural hypothesis search, not expected-value generation/implementation.
const fs=require('node:fs'),a=require('node:assert/strict');
const inputs=JSON.parse(fs.readFileSync(process.argv[2])).inputRecords;
a.equal(inputs.length,245);
const bits=n=>{const b=Buffer.alloc(8);b.writeDoubleBE(n);return b.toString('hex').toUpperCase();};
const models={};
function grouped(digits,k,remainderFirst) {
    let value=0;
    for(let start=0;start<digits.length;) {
        const count=start===0&&remainderFirst?(digits.length%k||k):Math.min(k,digits.length-start);
        value=value*Number('1e'+count)+Number(digits.slice(start,start+count));start+=count;
    }
    return value;
}
// Added only after the initial108-model counterexamples were retained: test
// separate rounded whole/fraction accumulation and a repeatedly scaled divisor.
// These are complete structural algorithms, not per-spelling corrections.
for(const denominator of ['decimalPower','repeatedMultiply']) {
    models[`twoLoops/${denominator}`]=p=>{
        const whole=grouped(p.whole,1,false),fraction=grouped(p.fraction,1,false);
        let divisor=1;
        if(denominator==='decimalPower')divisor=Number('1e'+p.scale);
        else for(let i=0;i<p.scale;++i)divisor*=10;
        return whole+fraction/divisor;
    };
}
for(let k=1;k<=9;++k) {
    for(const remainder of [false,true])for(const reciprocal of [false,true]) {
        models[`coefficient/k${k}/remainderFirst${remainder}/reciprocal${reciprocal}`]=p=>{
            const coefficient=grouped(p.whole+p.fraction,k,remainder);
            return reciprocal?coefficient*Number('1e-'+p.scale):coefficient/Number('1e'+p.scale);
        };
        models[`split/k${k}/remainderFirst${remainder}/reciprocal${reciprocal}`]=p=>{
            const fraction=grouped(p.fraction,k,remainder);
            return Number(p.whole)+(reciprocal?fraction*Number('1e-'+p.scale):fraction/Number('1e'+p.scale));
        };
    }
    for(const reverse of [false,true])for(const reciprocal of [false,true]) {
        models[`fractionTerms/k${k}/reverse${reverse}/reciprocal${reciprocal}`]=p=>{
            const terms=[];
            for(let start=0;start<p.fraction.length;start+=k) {
                const text=p.fraction.slice(start,start+k),end=start+text.length;
                terms.push(reciprocal?Number(text)*Number('1e-'+end):Number(text)/Number('1e'+end));
            }
            if(reverse)terms.reverse();
            return Number(p.whole)+terms.reduce((sum,term)=>sum+term,0);
        };
    }
}
const results=[];
for(const [name,model] of Object.entries(models)) {
    const counterexamples=[];
    for(const input of inputs) {
        const [whole,fraction]=input.literal.split('.');
        const predicted=bits(model({whole,fraction,scale:fraction.length}));
        if(predicted!==input.native)counterexamples.push({...input,predicted});
    }
    results.push({name,compared:inputs.length,mismatches:counterexamples.length,counterexamples});
}
const rationalGroups=new Map();
for(const input of inputs) {
    const [whole,fraction]=input.literal.split('.');
    const canonical=(whole.replace(/^0+/,'')||'0')+'.'+fraction.replace(/0+$/,'');
    if(!rationalGroups.has(canonical))rationalGroups.set(canonical,[]);
    rationalGroups.get(canonical).push(input);
}
const sameRationalDifferences=[...rationalGroups.entries()].filter(([,rows])=>new Set(rows.map(r=>r.native)).size>1)
    .map(([canonical,rows])=>({canonical,rows}));
console.log(JSON.stringify({scope:`${results.length} structural chunk/order hypotheses over245 captured bare literals; no semantic rule accepted.`,
    results:results.sort((x,y)=>x.mismatches-y.mismatches),sameRationalDifferences,
    limitation:'Finite structural model comparison only; no metadata/display/other conversion API or product parity claim.'},null,2));
