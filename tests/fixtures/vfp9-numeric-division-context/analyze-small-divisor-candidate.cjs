// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// POST-CAPTURE hypothesis only. No product imports or expected-value emission.
const fs=require('node:fs'),a=require('node:assert/strict');
const inv=JSON.parse(fs.readFileSync(__dirname+'/small-divisor-grid-cases.json'));
const native=JSON.parse(fs.readFileSync(process.argv[2]));
a.equal(native.documentedFieldsRepeatEqual,true);a.equal(native.casesPerRun,5060);
const bits=n=>{const b=Buffer.alloc(8);b.writeDoubleBE(n);return b.toString('hex').toUpperCase();};
const literals=new Map(),literalParseDifferences=[];
for(const row of inv.rows.filter(r=>r.kind==='literal-control')){
    const samples=native.observations.filter(o=>o.index===row.index);a.equal(samples.length,20);
    // TRANSFORM is setting-sensitive even for an unchanged literal descriptor.
    // Keep display in native evidence; this metadata/payload hypothesis does not model it.
    for(const s of samples){const {displayHex,...typed}=s.result;const {displayHex:firstDisplay,...first}=samples[0].result;a.deepEqual(typed,first);}
    const {displayHex,finite,...v}=samples[0].result;a(finite);
    literals.set(row.expression,{...v,number:Buffer.from(v.bits,'hex').readDoubleBE()});
    if(bits(Number(row.expression))!==v.bits)
        literalParseDifferences.push({literal:row.expression,native:v.bits,binary64:bits(Number(row.expression))});
}
function negate(v){
    const n=v.type==='I'&&v.number===0?0:-v.number;
    return {...v,width:v.width+1,number:n,bits:bits(n)};
}
function divide(l,r){
    a(Number.isFinite(l.number)&&Number.isFinite(r.number)&&r.number!==0);
    const quotient=l.number/r.number,n=quotient===0?0:quotient;
    const integer=l.type==='I'&&r.type==='I'&&Number.isInteger(n)&&Math.abs(n)<=2147483647;
    const dl=l.type==='I'?0:l.decimals,dr=r.type==='I'?0:r.decimals;
    const d=integer?0:Math.min(18,dl+dr);
    const whole=Math.abs(n)>=1?String(Math.trunc(Math.abs(n))).length:0;
    return {type:integer?'I':'N',width:Math.max(l.width-dl-(dl>0?1:0),whole+(n<0?1:0))+d+(d>0?1:0),
        ...(!integer?{decimals:d}:{}),number:n,bits:bits(n),cw:'027F'};
}
function parse(text){
    let i=0;
    function atom(){
        if(text[i]==='-'){++i;return negate(atom());}
        if(text[i]==='('){++i;const v=expression();a.equal(text[i++],')');return v;}
        const m=/^\d+(?:\.\d+)?/.exec(text.slice(i));a(m);i+=m[0].length;
        a(literals.has(m[0]));return literals.get(m[0]);
    }
    function expression(){let v=atom();while(text[i]==='/'){++i;v=divide(v,atom());}return v;}
    const v=expression();a.equal(i,text.length);return v;
}
function materialize(v,s){
    if(v.type==='I')return v;
    const d=Math.max(v.decimals,s);
    return {...v,decimals:d,width:v.width+d-v.decimals+(v.decimals===0&&d>0?1:0)};
}
function runtimeNegate(v){
    // Separate post-capture boundary hypothesis: runtime unary zero is +0,
    // unlike grouped constant unary zero. Do not alter the constant operator.
    const n=v.number===0?0:-v.number;
    return {...v,width:v.width+1,number:n,bits:bits(n)};
}
const cores=new Map(inv.cores.map(c=>[c.id,c]));
let compared=0,descriptorDifferences=0,payloadDifferences=0;const differences=[];
for(const o of native.observations){
    const row=inv.rows[o.index-1];a(row);if(row.kind!=='core')continue;
    const v=parse(cores.get(row.core).expression);
    const predicted=row.wrapper==='minus'?negate(v):row.wrapper==='minus-iif'?runtimeNegate(materialize(v,o.setting)):materialize(v,o.setting);
    const actual=o.result;
    const descriptor=['type','width','decimals'].some(k=>predicted[k]!==actual[k]);
    const payload=predicted.bits!==actual.bits;
    ++compared;if(descriptor)++descriptorDifferences;if(payload)++payloadDifferences;
    if(descriptor||payload)differences.push({expression:row.expression,setting:o.setting,fixed:o.fixed,mode:o.mode,predicted,actual,descriptor,payload});
}
a.equal(compared,4480);
console.log(JSON.stringify({candidate:'Prior native intrinsic metadata/materialization and sequential binary64 division extended to finite small divisors',
    compared,descriptorDifferences,payloadDifferences,literalParseDifferences,
    runtimeZeroHypothesis:'Unary minus after IIF preserves +0; grouped constant unary produces -0. Native complete-root evidence only, not callback timing qualification.',
    limitations:'Post-capture hypothesis, not an expected-value oracle. Core complete-root descriptors/payloads only; display, runtime mixtures, retained boundaries and callback timing not modeled. No production acceptance or guard relaxation.',differences},null,2));
