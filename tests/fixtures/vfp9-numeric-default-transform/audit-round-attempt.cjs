// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Forensic first-only receipt check; does NOT relax the frozen strict reader.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),c=require('node:crypto');
const root=process.argv[2];a(root);const exists=n=>fs.existsSync(path.join(root,n))||fs.existsSync(path.join(root,n+'.gz'));
const bytes=n=>{const p=path.join(root,n);return fs.existsSync(p)?fs.readFileSync(p):z.gunzipSync(fs.readFileSync(p+'.gz'));},read=n=>bytes(n).toString();
a.equal(read('source-commit.txt'),'35bd9b24bbecfb4f9503170c5909375463393425\n');a.equal(read('source-status.txt'),'');
a.equal(JSON.parse(read('native-shell-completion.json')).exit_code,1);
a.equal(read('first.status'),'0\n');a.equal(bytes('first.stderr').length,0);a.equal(read('first-structure.status'),'1\n');a.equal(bytes('first-structure.json').length,0);
a(read('first-structure.stderr').includes("width=001D;decimals=00000012"));
a.equal(read('first.guard.status'),'0\n');a.equal(read('first.prefix.status'),'1\n');a.equal(read('first.prefix.txt'),'');
const pressure=Object.fromEntries(read('first.pressure-before.txt').split('\n').find(v=>v.startsWith('full ')).split(' ').slice(1).map(v=>v.split('=')));
a(Number(pressure.avg10)<1&&Number(pressure.avg60)<1);
for(const n of ['repeat.started.utc','repeat.status','repeat-results.txt','audit.status','finished.utc'])a(!exists(n));
const raw=bytes('first-results.txt');a.deepEqual(raw,bytes('first.out'));a.equal(c.createHash('sha256').update(raw).digest('hex'),'e86d9b43a656896a2dd9ed3965b5d4fe77273aeca765dfe89d3977511e492468');
const lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.pop(),'DONE|528');a.equal(lines.length,528);
const seen=new Set(),widths={};for(const line of lines){const f=line.split('|');a.equal(f.length,26);a(['OFF','ON'].includes(f[0]));a(Number(f[1])>=1&&Number(f[1])<=264);const key=f[0]+':'+f[1];a(!seen.has(key));seen.add(key);
 a.match(f[4],/^type=4E;width=001[DE];decimals=00000012;integer=[0-9A-F]{8};real=[0-9A-F]{16};cw=027F$/);const width=f[4].split(';')[1];widths[width]=(widths[width]||0)+1;}
a.deepEqual(widths,{'width=001D':264,'width=001E':264});
console.log(JSON.stringify({forensicFirstOnly:true,records:528,nativeExit:0,strictReaderExit:1,pairedQualified:false,argumentControlQualified:false,argumentWidths:widths,limitation:'Complete first-only transport/footer evidence, NOT strict metadata/semantic acceptance or a qualified ROUND pair. Stored argument assignment did not preserve required width40; repeat correctly prevented. No REA Evidence or descendant-cleanup attestation.'}));
