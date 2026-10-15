// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Native recovery audit, not a Copperfin parser or acceptance oracle.
const fs = require('node:fs'), path = require('node:path'), assert = require('node:assert/strict');
const crypto = require('node:crypto');
const root = process.argv[2];
const expressions = ['1.', '0.', '0001.', '1.0', ...[17,18,19,38,39,40,41,100,253].map(n=>'1.'+'0'.repeat(n)),
    ...[10,38,39,40,41,100,253].map(n=>'0'.repeat(n)+'.1'),
    ...[38,39,40,100,252].map(n=>'1'+'0'.repeat(n)+'.0'), '(1.00)', '  1.00  ',
    '1.'+'0'.repeat(254), '1.'+'0'.repeat(255), '0'.repeat(254)+'.1', '1'+'0'.repeat(253)+'.0'];
assert.equal(expressions.length,31);
function metadata(text) {
    assert.match(text,/^type=[0-9A-F]{2};width=[0-9A-F]{4};decimals=[0-9A-F]{8};integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=[0-9A-F]{4}$/);
    const m=Object.fromEntries(text.split(';').map(s=>s.split('=')));
    assert(['4E','49'].includes(m.type)); assert.equal(m.cw,'027F');
    return m.type==='4E' ? {type:m.type,width:m.width,decimals:m.decimals,real:m.real,cw:m.cw}
        : {type:m.type,width:m.width,integer:m.integer,cw:m.cw};
}
function capture(name) {
    assert.equal(fs.readFileSync(path.join(root,name+'.status'),'utf8'),'0\n');
    assert.equal(fs.statSync(path.join(root,name+'.stderr')).size,0);
    const raw=fs.readFileSync(path.join(root,name+'-results.txt'));
    assert.deepEqual(raw,fs.readFileSync(path.join(root,name+'.out')));
    const lines=raw.toString('ascii').split('\r\n'); assert.equal(lines.pop(),'');
    assert.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');
    assert.equal(lines.pop(),'DONE|31'); assert.equal(lines.length,31);
    const rows=lines.map((line,index)=>{
        const f=line.split('|'); assert.equal(Number(f[1]),index+1);
        const expression=Buffer.from(f[2],'hex').toString('ascii'); assert.equal(expression,expressions[index]);
        if(index===28) {
            assert.equal(f.length,5); assert.equal(f[0],'ERROR'); assert.equal(f[3],'36');
            assert.equal(Buffer.from(f[4],'hex').toString('ascii'),'Command contains unrecognized phrase/keyword.');
            return {index:index+1,expression,error:36};
        }
        assert.equal(f.length,7); assert.equal(f[0],'PLAIN');
        const direct=metadata(f[3]),assigned=metadata(f[4]);
        assert.match(f[5],/^[0-9A-F]{16}$/); assert.equal(f[5],f[6]);
        for(const m of [direct,assigned]) {
            if(m.type==='4E') assert.equal(m.real,f[5]);
            else assert.equal(Buffer.from(f[5],'hex').readDoubleBE(),parseInt(m.integer,16)|0);
        }
        assert.equal(direct.type,index<3?'49':'4E');
        if(index>=3) {
            const lexeme=expression.trim().replace(/^\((.*)\)$/,'$1');
            assert.equal(parseInt(direct.width,16),Math.min(40,lexeme.length));
            assert.equal(parseInt(direct.decimals,16),Math.min(18,lexeme.length-lexeme.indexOf('.')-1));
        }
        return {index:index+1,expression,direct,assigned,bits:f[5]};
    });
    return {raw,rows};
}
const first=capture('plain-v2-first'),repeat=capture('plain-v2-repeat');
assert.deepEqual(first.rows,repeat.rows);
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
process.stdout.write(JSON.stringify({casesPerRun:31,successfulValues:30,recordedNativeErrors:1,
    documentedFieldsRepeatEqual:true,rawByteEqual:first.raw.equals(repeat.raw),
    firstSha256:hash(first.raw),repeatSha256:hash(repeat.raw)},null,2)+'\n');
