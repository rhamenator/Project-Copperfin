// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Native oracle for the exact continuation regression expressions/call count.
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict'),crypto=require('node:crypto');
const root=process.argv[2];
function capture(name){
 a.equal(fs.readFileSync(path.join(root,name+'.status'),'utf8'),'0\n');a.equal(fs.statSync(path.join(root,name+'.stderr')).size,0);
 const raw=fs.readFileSync(path.join(root,name+'-results.txt'));a.deepEqual(raw,fs.readFileSync(path.join(root,name+'.out')));
 const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|4');a.equal(lines.length,4);
 const keys=new Set();for(const l of lines){const f=l.split('|');a.equal(f.length,7);a.equal(f[0],'CACHE');a(['2','14'].includes(f[1]));a(['OFF','ON'].includes(f[2]));const key=f[1]+'/'+f[2];a(!keys.has(key));keys.add(key);a.deepEqual(f.slice(3),['0000000000000000','8000000000000000','8000000000000000','3']);}
 a.equal(keys.size,4);return raw;
}
const first=capture('cache-first'),repeat=capture('cache-repeat');a.deepEqual(first,repeat);
const hash=b=>crypto.createHash('sha256').update(b).digest('hex');
process.stdout.write(JSON.stringify({settingsPerRun:4,actualExit:0,stderrEmpty:true,byteRepeat:true,firstSha256:hash(first),repeatSha256:hash(repeat),nativeBits:['0000000000000000','8000000000000000','8000000000000000'],callbackCountPerSetting:3},null,2)+'\n');
