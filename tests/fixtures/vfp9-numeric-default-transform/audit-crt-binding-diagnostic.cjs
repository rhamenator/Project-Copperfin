// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// ONE-CALL framing/control reader. Completed diagnosis != successful binding.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),crypto=require('node:crypto');
const inv=JSON.parse(fs.readFileSync(path.join(__dirname,'crt-cases.json'))),root=process.argv[2];a(root);
const bytes=n=>fs.existsSync(path.join(root,n))?fs.readFileSync(path.join(root,n)):z.gunzipSync(fs.readFileSync(path.join(root,n+'.gz')));
a.equal(bytes('first.status').toString(),'0\n');a.equal(bytes('first.stderr').length,0);
const raw=bytes('first-results.txt');a.deepEqual(raw,bytes('first.out'));
const lines=raw.toString('ascii').split('\r\n');a.deepEqual([lines.shift(),lines.pop(),lines.pop()],['VERSION|Visual FoxPro 09.00.0000.7423 for Windows','','DONE|1']);a.equal(lines.length,1);
const f=lines[0].split('|');a.equal(f.length,8);a.equal(f[0],'BIND');
for(const i of [2,5])a.match(f[i],/^type=4E;width=0028;decimals=00000012;integer=[0-9A-F]{8};real=3FE0000000000000;cw=027F$/);
a.equal(f[2].replace(/integer=[0-9A-F]{8}/,'integer=UNUSED'),f[5].replace(/integer=[0-9A-F]{8}/,'integer=UNUSED'));
a.equal(f[3],'3FE0000000000000');a.equal(f[6],f[3]);a.match(f[4],/^(?:[0-9A-F]{2})+$/);a.equal(f[7],f[4]);
const keys=['stage','error','bound','pathLength','module','queryBytes','baseMatches','allocationMatches','regionSize','state','protect','memoryType','imageValid','imageMachine','imageMagic','imageStamp','imageSize','imageChecksum','selftest','fileSizeLow','fileSizeHigh','readBytes','readCalls','closeOk','closeError','fileValid','fileMachine','fileMagic','fileStamp','fileImageSize','fileChecksum','hashValid','fileSha256','cw0','cw1'];
const pairs=f[1].split(';').map(s=>s.split('='));a.deepEqual(pairs.map(p=>p[0]),keys);for(const p of pairs)a.equal(p.length,2);
const c=Object.fromEntries(pairs),n=k=>parseInt(c[k],16);
a(['MODULE_API','PATH_API','PATH_BOUND','QUERY_API','QUERY_SIZE','QUERY_PREDICATE','IMAGE_HEADER','CACHE_IMAGE','CACHE_PATH','SHA_SELFTEST','OPEN_API','SIZE_API','SIZE_BOUND','READ_LIMIT','READ_API','READ_BOUND','EOF_API','EOF_BOUND','CLOSE_API','READ_SIZE','FILE_HEADER','HEADER_EQUAL','BOUND'].includes(c.stage));
for(const k of keys.filter(k=>!['stage','module','fileSha256','cw0','cw1'].includes(k)))a.match(c[k],/^[0-9A-F]{8}$/);
for(const k of ['bound','baseMatches','allocationMatches','imageValid','selftest','closeOk','fileValid','hashValid'])a([0,1].includes(n(k)));
a.match(c.module,/^(?:[0-9A-F]{2})*$/);a(c.module.length<=518);a.match(c.fileSha256,/^[0-9A-F]{64}$/);a.equal(c.cw0,'027F');a.equal(c.cw1,'027F');
if(n('bound')){
 a.equal(c.stage,'BOUND');a.equal(n('error'),0);a.equal(Buffer.from(c.module,'hex').toString('ascii').toLowerCase(),inv.modulePath.toLowerCase());a.equal(n('pathLength'),c.module.length/2);
 for(const k of ['baseMatches','allocationMatches','imageValid','selftest','closeOk','fileValid','hashValid'])a.equal(n(k),1);
 a.equal(n('queryBytes'),28);a(n('regionSize')>=4096);a.equal(n('state'),0x1000);a.equal(n('memoryType'),0x1000000);a(!(n('protect')&0x100));a(n('protect')&(0x02|0x04|0x08|0x20|0x40|0x80));
 a.equal(n('fileSizeLow'),inv.moduleFileSize);a.equal(n('fileSizeHigh'),0);a.equal(n('readBytes'),inv.moduleFileSize);a(n('readCalls')>=1&&n('readCalls')<=4096);a.equal(n('closeError'),0);
 for(const prefix of ['image','file'])for(const [suffix,value] of Object.entries({Machine:inv.modulePe.machine,Magic:inv.modulePe.magic,Stamp:inv.modulePe.stamp,Checksum:inv.modulePe.checksum}))a.equal(n(prefix+suffix),value);
 a.equal(n('imageSize'),inv.modulePe.imageSize);a.equal(n('fileImageSize'),inv.modulePe.imageSize);a.equal(c.fileSha256.toLowerCase(),inv.moduleSha256);
}else a.notEqual(c.stage,'BOUND');
console.log(JSON.stringify({requirement:inv.requirement,diagnosticComplete:true,oneCall:true,bindingEstablished:!!n('bound'),pairedQualified:false,controlsUnchanged:true,
 firstSha256:crypto.createHash('sha256').update(raw).digest('hex'),observation:c,limitation:'One diagnostic, no CRT conversion or formatter evidence. Failed identity remains failed. File digest is sampled once, not atomic mapped-image/file proof.'},null,2));
if(process.argv[3]==='--require-bound'&&!n('bound')){console.error('Runtime binding NOT established; preserve diagnosis, do not run full caller.');process.exit(2);}
