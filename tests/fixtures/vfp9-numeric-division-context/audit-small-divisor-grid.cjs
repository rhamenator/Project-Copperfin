// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Independent actual-observation reader; no product/candidate/expected import.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib');
const a=require('node:assert/strict'),crypto=require('node:crypto');
const root=process.argv[2], source=fs.readFileSync(path.join(__dirname,'small-divisor-grid.prg'),'utf8');
const inv=JSON.parse(fs.readFileSync(path.join(__dirname,'small-divisor-grid-cases.json')));
a.equal(inv.rows.length,253);a.equal(inv.cores.length,56);a.equal(inv.literals.length,15);
a.deepEqual(inv.settings,[0,2,6,14,18]);
a.equal([...source.matchAll(/^expressions\[\d+\]='/gm)].length,253);
a.equal([...source.matchAll(/^ CASE index=\d+$/gm)].length,253);
for(let i=0;i<inv.settings.length;++i)a(source.includes(`settings[${i+1}]=${inv.settings[i]}`));
const declared=new Map(inv.rows.map(r=>[r.index,r]));
a.deepEqual([...declared.keys()],Array.from({length:253},(_,i)=>i+1));
for(const r of inv.rows){
    a(source.includes(`expressions[${r.index}]='${r.expression}'`));
    a(source.includes(` CASE index=${r.index}\n  RETURN CFNMETA(${r.expression})+'|'+HexBytes(BINTOC(${r.expression},'BRS'))+'|'+HexBytes(TRANSFORM(${r.expression}))`));
}
const hex=x=>a.match(x,/^(?:[0-9A-F]{2})*$/);
function descriptor(text){
    a.match(text,/^type=(?:49|4E);width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=[0-9A-F]{4}$/);
    const raw=Object.fromEntries(text.split(';').map(p=>p.split('=')));
    const typed={type:raw.type==='49'?'I':'N',width:parseInt(raw.width,16),cw:raw.cw};
    if(typed.type==='I'){
        typed.integer=parseInt(raw.integer,16)|0;
        const b=Buffer.alloc(8);b.writeDoubleBE(typed.integer);typed.bits=b.toString('hex').toUpperCase();
    }else{typed.decimals=parseInt(raw.decimals,16);typed.bits=raw.real;}
    typed.finite=Number.isFinite(Buffer.from(typed.bits,'hex').readDoubleBE());
    return {raw,typed};
}
function bytes(name){const f=path.join(root,name);return fs.existsSync(f)?fs.readFileSync(f):zlib.gunzipSync(fs.readFileSync(f+'.gz'));}
function capture(name){
    a.equal(bytes(name+'.status').toString(),'0\n');a.equal(bytes(name+'.stderr').length,0);
    const raw=bytes(name+'-results.txt');a.deepEqual(raw,bytes(name+'.out'));
    const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');
    a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
    a.equal(lines.pop(),'DONE|5060');a.equal(lines.length,5060);
    const rows=new Map();let errors=0,nonfinite=0,emptyDisplays=0;
    for(const line of lines){
        const f=line.split('|'),error=f[0].startsWith('ERROR-'),mode=f[0].replace(/^ERROR-/,'');
        a.equal(f.length,error?7:8);a(['EVAL','COMPILED'].includes(mode));
        const setting=Number(f[1]),fixed=f[2],index=Number(f[3]),input=declared.get(index);
        a(inv.settings.includes(setting));a(['OFF','ON'].includes(fixed));a(input);
        hex(f[4]);a.equal(Buffer.from(f[4],'hex').toString('ascii'),input.expression);
        let result;
        if(error){a.match(f[5],/^\d+$/);hex(f[6]);result={error:Number(f[5]),messageHex:f[6]};++errors;}
        else{
            const d=descriptor(f[5]);a.equal(f[6],d.typed.bits);hex(f[7]);
            result={...d.typed,displayHex:f[7]};if(!d.typed.finite)++nonfinite;if(f[7]==='')++emptyDisplays;
        }
        const key=JSON.stringify([setting,fixed,index,mode]);a(!rows.has(key));
        rows.set(key,{fields:f,error,typed:{setting,fixed,index,mode,kind:input.kind,result}});
    }
    for(const setting of inv.settings)for(const fixed of ['OFF','ON'])for(const input of inv.rows){
        const x=rows.get(JSON.stringify([setting,fixed,input.index,'EVAL']));
        const y=rows.get(JSON.stringify([setting,fixed,input.index,'COMPILED']));a(x&&y);
        a.deepEqual({...x.typed,mode:null},{...y.typed,mode:null});
    }
    return {raw,rows,errors,nonfinite,emptyDisplays};
}
const first=capture('first'),repeat=capture('repeat'),unused={};let rawDifferentRows=0;
for(const [key,x]of first.rows){
    const y=repeat.rows.get(key);a(y);a.deepEqual(x.typed,y.typed);
    if(x.fields.join('|')!==y.fields.join('|'))++rawDifferentRows;
    if(x.error){a.deepEqual(x.fields,y.fields);continue;}
    for(let i=0;i<8;++i)if(i!==5)a.equal(x.fields[i],y.fields[i]);
    const p=descriptor(x.fields[5]).raw,q=descriptor(y.fields[5]).raw;
    for(const field of Object.keys(p))if(p[field]!==q[field]){
        a(p.type==='49'?['real','decimals'].includes(field):field==='integer');
        const k=p.type+':'+field;unused[k]=(unused[k]||0)+1;
    }
}
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');
console.log(JSON.stringify({requirement:inv.requirement,casesPerRun:5060,expressions:253,
    successfulValues:5060-first.errors,errors:first.errors,nonfiniteValues:first.nonfinite,
    emptyDisplays:first.emptyDisplays,documentedFieldsRepeatEqual:true,compiledVersusEvaluateDifferences:0,
    rawByteEqual:first.raw.equals(repeat.raw),rawDifferentRows,rawDifferencesConfinedToUnusedSlots:unused,
    firstSha256:sha(first.raw),repeatSha256:sha(repeat.raw),
    limitation:'Complete roots only; native errors/nonfinite/retained boundaries are observations, not changed checked-policy authorization or helper acceptance.',
    observations:[...first.rows.values()].map(r=>r.typed)},null,2));
