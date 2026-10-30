// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// POST-CAPTURE hypothesis only; no product import or expected-value emission.
const fs=require('node:fs'),z=require('node:zlib'),a=require('node:assert/strict');
const inventory=JSON.parse(fs.readFileSync(__dirname+'/small-divisor-timing-cases.json'));
const observed=JSON.parse(fs.readFileSync(process.argv[2]));
const controlInventory=JSON.parse(fs.readFileSync(__dirname+'/small-divisor-grid-cases.json'));
const controlNative=JSON.parse(z.gunzipSync(fs.readFileSync(__dirname+'/small-divisor-captures/audit-qualified.json.gz')));
const oldInventory=JSON.parse(fs.readFileSync(__dirname+'/nonwhole-grid-cases.json'));
const oldNative=JSON.parse(z.gunzipSync(fs.readFileSync(__dirname+'/nonwhole-captures/audit-qualified.json.gz')));
const literals=new Map();
function addLiteral(text,samples){
    a.equal(samples.length,20);
    const typed=s=>{const {displayHex,...v}=s.result;return v;};
    const first=typed(samples[0]);for(const s of samples)a.deepEqual(typed(s),first);
    const {displayHex,finite,...v}=first;
    literals.set(text,{...v,number:Buffer.from(v.bits,'hex').readDoubleBE()});
}
for(const row of controlInventory.rows.filter(r=>r.kind==='literal-control'))
    addLiteral(row.expression,controlNative.observations.filter(o=>o.index===row.index));
const markerControl=oldInventory.rows.find(r=>r.kind==='literal-control'&&r.literal==='2.00');a(markerControl);
addLiteral('2.00',oldNative.observations.filter(o=>o.index===markerControl.index));
const bits=n=>{const b=Buffer.alloc(8);b.writeDoubleBE(n);return b.toString('hex').toUpperCase();};
function neg(v){
    const n=v.number===0?(v.type==='I'?0:-0):-v.number;
    return {...v,width:v.width+1,number:n,bits:bits(n)};
}
function runtimeNeg(v){const n=v.number===0?0:-v.number;return {...v,width:v.width+1,number:n,bits:bits(n)};}
function intrinsic(l,r){
    const quotient=l.number/r.number,n=quotient===0?0:quotient;
    const integer=l.type==='I'&&r.type==='I'&&Number.isInteger(n)&&Math.abs(n)<=2147483647;
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
function materialize(v,s){
    if(v.type==='I')return v;
    const d=Math.max(v.decimals,s),n=v.number===0?0:v.number;
    return {...v,number:n,bits:bits(n),decimals:d,width:v.width+d-v.decimals+(v.decimals===0&&d>0?1:0)};
}
function parameter(v){return {...v,width:10+v.decimals+(v.decimals>0?1:0)+(v.number<0?1:0)};}
function runtime(l,r,s){
    const dl=l.type==='I'?0:l.decimals,dr=r.type==='I'?0:r.decimals,d=Math.min(18,Math.max(s+2,dl,dr));
    const n=l.number/r.number;
    return {type:'N',width:Math.min(40,l.width+r.width-dl-dr+d-(dl>0||dr>0?1:0)),decimals:d,number:n,bits:bits(n),cw:'027F'};
}
function rootIsUnary(text){
    if(!text.startsWith('-'))return false;
    let depth=0;
    for(const c of text){if(c==='(')++depth;else if(c===')')--depth;else if(c==='/'&&depth===0)return false;}
    return true;
}
const plain=v=>{const {number,...out}=v;return out;};
let compared=0;const differences=[],retainedNonfinite=[];
for(const o of observed.observations){
    const row=inventory.rows[o.index-1];a(row);
    if(!Number.isFinite(Buffer.from(o.result.bits,'hex').readDoubleBE())){
        retainedNonfinite.push({expression:row.expression,setting:o.setting,fixed:o.fixed,mode:o.mode,result:o.result});continue;
    }
    let predicted,seen=null;
    if(row.pureControl)predicted=row.expression.startsWith('-(IIF')?runtimeNeg(materialize(parse('0/0.5'),o.setting)):materialize(parse(row.expression),o.setting);
    else{
        const v=parse(row.core),m=materialize(v,o.setting),target=row.changesTo,marker=literals.get('2.00');
        switch((row.index-1)%9){
        case 0:seen=parameter(m);predicted=seen;break;
        case 1:seen=parameter(m);predicted=runtimeNeg(seen);break;
        case 2:seen=parameter(neg(v));predicted=seen;break;
        case 3:case 8:predicted=runtime(m,marker,target);break;
        // An ungrouped unary prefix consumes intrinsic precision; the explicit
        // grouped argument in case3 crosses the prepared materialization boundary.
        case 7:predicted=runtime(rootIsUnary(row.core)?v:m,marker,target);break;
        case 4:predicted=runtime(neg(v),marker,target);break;
        case 5:predicted=runtimeNeg(runtime(m,marker,target));break;
        case 6:predicted=runtime(marker,materialize(v,target),target);break;
        default:a.fail();
        }
    }
    ++compared;
    const actual={result:o.result,seenParameter:o.seenParameter};
    const predictedFields={result:plain(predicted),seenParameter:seen&&plain(seen)};
    try{a.deepEqual(actual,predictedFields);}catch{differences.push({expression:row.expression,setting:o.setting,fixed:o.fixed,mode:o.mode,actual,predicted:predictedFields});}
}
a.equal(compared+retainedNonfinite.length,4380);
console.log(JSON.stringify({compared,retainedNonfiniteObservations:retainedNonfinite.length,differences:differences.length,
    limitation:'Post-capture finite timing hypothesis only, using actual literal controls; retained nonfinite rows are not new checked-zero policy. No test oracle, product pass or universal Numeric claim.',examples:differences,retainedNonfinite},null,2));
