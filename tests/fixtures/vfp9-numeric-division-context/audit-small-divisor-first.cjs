// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// First-run framing/typed audit only. Does NOT simulate or qualify a repeat.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib');
const a=require('node:assert/strict'),crypto=require('node:crypto');
const root=process.argv[2], inv=JSON.parse(fs.readFileSync(path.join(__dirname,'small-divisor-grid-cases.json')));
const source=fs.readFileSync(path.join(__dirname,'small-divisor-grid.prg'),'utf8');
a.equal(inv.rows.length,253);a.equal(inv.cores.length,56);a.equal(inv.literals.length,15);
a.deepEqual(inv.settings,[0,2,6,14,18]);
const declared=new Map(inv.rows.map(r=>[r.index,r]));
a.deepEqual([...declared.keys()],Array.from({length:253},(_,i)=>i+1));
a.equal([...source.matchAll(/^expressions\[\d+\]='/gm)].length,253);
a.equal([...source.matchAll(/^ CASE index=\d+$/gm)].length,253);
for(let i=0;i<inv.settings.length;++i)a(source.includes(`settings[${i+1}]=${inv.settings[i]}`));
for(const r of inv.rows){
    a(source.includes(`expressions[${r.index}]='${r.expression}'`));
    a(source.includes(` CASE index=${r.index}\n  RETURN CFNMETA(${r.expression})+'|'+HexBytes(BINTOC(${r.expression},'BRS'))+'|'+HexBytes(TRANSFORM(${r.expression}))`));
}
const bytes=name=>{const f=path.join(root,name);return fs.existsSync(f)?fs.readFileSync(f):zlib.gunzipSync(fs.readFileSync(f+'.gz'));};
a.equal(bytes('first.status').toString(),'0\n');a.equal(bytes('first.stderr').length,0);
const raw=bytes('first-results.txt');a.deepEqual(raw,bytes('first.out'));
const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');
a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
a.equal(lines.pop(),'DONE|5060');a.equal(lines.length,5060);
const rows=new Map(),byKind={},cw=new Set();let errors=0,nonfinite=0,emptyDisplays=0;
const hex=x=>a.match(x,/^(?:[0-9A-F]{2})*$/);
for(const line of lines){
    const f=line.split('|'),error=f[0].startsWith('ERROR-'),mode=f[0].replace(/^ERROR-/,'');
    a.equal(f.length,error?7:8);a(['EVAL','COMPILED'].includes(mode));
    const setting=Number(f[1]),fixed=f[2],index=Number(f[3]),input=declared.get(index);
    a(inv.settings.includes(setting));a(['OFF','ON'].includes(fixed));a(input);
    hex(f[4]);a.equal(Buffer.from(f[4],'hex').toString('ascii'),input.expression);
    let result;
    if(error){a.match(f[5],/^\d+$/);hex(f[6]);result={error:Number(f[5]),messageHex:f[6]};++errors;}
    else{
        a.match(f[5],/^type=(?:49|4E);width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=[0-9A-F]{4}$/);
        const d=Object.fromEntries(f[5].split(';').map(p=>p.split('=')));
        result={type:d.type==='49'?'I':'N',width:parseInt(d.width,16),cw:d.cw};cw.add(d.cw);
        if(result.type==='I'){
            result.integer=parseInt(d.integer,16)|0;const b=Buffer.alloc(8);b.writeDoubleBE(result.integer);
            result.bits=b.toString('hex').toUpperCase();
        }else{result.decimals=parseInt(d.decimals,16);result.bits=d.real;}
        a.equal(f[6],result.bits);hex(f[7]);result.displayHex=f[7];
        result.finite=Number.isFinite(Buffer.from(result.bits,'hex').readDoubleBE());
        if(!result.finite)++nonfinite;if(f[7]==='')++emptyDisplays;
    }
    const key=JSON.stringify([setting,fixed,index,mode]);a(!rows.has(key));
    rows.set(key,{setting,fixed,index,mode,kind:input.kind,result});
    const k=input.kind;byKind[k]??={values:0,errors:0};++byKind[k][error?'errors':'values'];
}
for(const setting of inv.settings)for(const fixed of ['OFF','ON'])for(const input of inv.rows){
    const x=rows.get(JSON.stringify([setting,fixed,input.index,'EVAL']));
    const y=rows.get(JSON.stringify([setting,fixed,input.index,'COMPILED']));a(x&&y);
    a.deepEqual({...x,mode:null},{...y,mode:null});
}
console.log(JSON.stringify({requirement:inv.requirement,actualRun:'first only',actualStatus:0,
    observations:5060,expressions:253,successfulValues:5060-errors,errors,nonfiniteValues:nonfinite,
    emptyDisplays,byKind,controlWords:[...cw],compiledVersusEvaluateDifferences:0,
    firstSha256:crypto.createHash('sha256').update(raw).digest('hex'),repeatQualified:false,
    limitation:'One actual invocation only; no repeat/paired-model/helper/public compatibility qualification. Boundary errors/nonfinite facts are not authorization to change checked policies.',
    records:[...rows.values()]},null,2));
