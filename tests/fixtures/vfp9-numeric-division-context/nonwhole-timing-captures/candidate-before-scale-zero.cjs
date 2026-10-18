// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// POST-CAPTURE hypothesis check only. Never emits public expected values.
const fs=require('node:fs'),z=require('node:zlib'),a=require('node:assert/strict');
const inventory=JSON.parse(fs.readFileSync(__dirname+'/nonwhole-timing-cases.json'));
const observed=JSON.parse(fs.readFileSync(process.argv[2]));
const oldInventory=JSON.parse(fs.readFileSync(__dirname+'/nonwhole-grid-cases.json'));
const oldNative=JSON.parse(z.gunzipSync(fs.readFileSync(__dirname+'/nonwhole-captures/audit-qualified.json.gz')));
const literals=new Map();
for(const row of oldInventory.rows.filter(r=>r.kind==='literal-control')){
    const samples=oldNative.observations.filter(o=>o.index===row.index);a.equal(samples.length,20);
    for(const s of samples)a.deepEqual(s.result,samples[0].result);
    literals.set(row.literal,{...samples[0].result,number:Buffer.from(samples[0].result.bits,'hex').readDoubleBE()});
}
const bits=n=>{const b=Buffer.alloc(8);b.writeDoubleBE(n);return b.toString('hex').toUpperCase();};
const neg=v=>({...v,width:v.width+1,number:-v.number,bits:bits(-v.number)});
function intrinsic(l,r){
    const n=l.number/r.number,integer=l.type==='I'&&r.type==='I'&&Number.isInteger(n)&&Math.abs(n)<=2147483647;
    const dl=l.type==='I'?0:l.decimals,dr=r.type==='I'?0:r.decimals,d=integer?0:Math.min(18,dl+dr);
    const whole=Math.abs(n)>=1?String(Math.trunc(Math.abs(n))).length:0;
    return {type:integer?'I':'N',width:Math.max(l.width-dl-(dl>0?1:0),whole+(n<0?1:0))+d+(d>0?1:0),
        ...(!integer?{decimals:d}:{}),number:n,bits:bits(n),cw:'027F'};
}
function parse(text){
    let i=0;
    function atom(){
        if(text[i]==='-'){++i;return neg(atom());}
        if(text[i]==='('){++i;const v=expression();a.equal(text[i++],')');return v;}
        const m=/^\d+(?:\.\d+)?/.exec(text.slice(i));a(m);i+=m[0].length;a(literals.has(m[0]));return literals.get(m[0]);
    }
    function expression(){let v=atom();while(text[i]==='/'){++i;v=intrinsic(v,atom());}return v;}
    const v=expression();a.equal(i,text.length);return v;
}
function materialize(v,s){if(v.type==='I')return v;const d=Math.max(v.decimals,s);return {...v,decimals:d,width:v.width+d-v.decimals+(v.decimals===0&&d>0?1:0)};}
function parameter(v){return {...v,width:11+v.decimals+(v.number<0?1:0)};}
function runtime(l,r,s){
    const dl=l.type==='I'?0:l.decimals,dr=r.type==='I'?0:r.decimals,d=Math.min(18,Math.max(s+2,dl,dr));
    const n=l.number/r.number;
    return {type:'N',width:Math.min(40,l.width+r.width-dl-dr+d-(dl>0||dr>0?1:0)),decimals:d,number:n,bits:bits(n),cw:'027F'};
}
const plain=v=>{const {number,...out}=v;return out;};
let compared=0,retained=0;const differences=[];
for(const o of observed.observations){
    const row=inventory.rows[o.index-1];a(row);
    // Keep all zero/nonfinite controls in the raw audit. No new zero contract
    // is inferred from the existing nonzero timing/LPARAMETERS hypothesis.
    if(row.core&&row.core.includes('0.00')){++retained;continue;}
    let predicted,seen=null;
    if(row.oldUnknownControl)predicted=materialize(parse(row.expression),o.setting);
    else{
        const v=parse(row.core),m=materialize(v,o.setting),target=row.changesTo;
        const marker={...literals.get('2.50'),width:4,decimals:2,number:2,bits:bits(2)};
        // Marker 2.00 has independently observed source metadata W4/D2 and
        // exact payload2; equal payload alone does not establish its descriptor.
        switch((row.index-1)%9){
        case 0:seen=parameter(m);predicted=seen;break;
        case 1:seen=parameter(m);predicted=neg(seen);break;
        case 2:seen=parameter(neg(v));predicted=seen;break;
        case 3:case 7:case 8:predicted=runtime(m,marker,target);break;
        case 4:predicted=runtime(neg(v),marker,target);break;
        case 5:predicted=neg(runtime(m,marker,target));break;
        case 6:predicted=runtime(marker,materialize(v,target),target);break;
        default:a.fail();
        }
    }
    ++compared;
    const actual={result:o.result,seenParameter:o.seenParameter};
    const expected={result:plain(predicted),seenParameter:seen&&plain(seen)};
    if(JSON.stringify(actual)!==JSON.stringify(expected)){
        try{a.deepEqual(actual,expected);}catch{differences.push({expression:row.expression,setting:o.setting,fixed:o.fixed,mode:o.mode,actual,predicted:expected});}
    }
}
a.equal(compared,2220);a.equal(retained,720);
console.log(JSON.stringify({compared,retainedZeroBoundaryObservations:retained,differences:differences.length,
    limitation:'Post-capture nonzero hypothesis only. Zero boundaries stay retained; not a test oracle or product acceptance.',examples:differences},null,2));
