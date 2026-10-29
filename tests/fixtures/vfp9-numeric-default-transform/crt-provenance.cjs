// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Read-only installed PE-header/digest comparison, never code/disassembly.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),crypto=require('node:crypto');
const root=process.argv[2];a(root);const file=path.join(root,'diagnostic.json');
const diagnostic=JSON.parse(fs.existsSync(file)?fs.readFileSync(file):z.gunzipSync(fs.readFileSync(file+'.gz')));
a.equal(diagnostic.bindingEstablished,false);a.equal(diagnostic.observation.stage,'HEADER_EQUAL');
function pe(file){
 const b=fs.readFileSync(file);a(b.length>=64);a.equal(b.toString('ascii',0,2),'MZ');const offset=b.readUInt32LE(60);a(offset>=64&&offset<=4096-92&&offset+92<=b.length);a.equal(b.readUInt32LE(offset),0x4550);
 return {path:file,fileSize:b.length,sha256:crypto.createHash('sha256').update(b).digest('hex'),machine:b.readUInt16LE(offset+4),magic:b.readUInt16LE(offset+24),stamp:b.readUInt32LE(offset+8),imageSize:b.readUInt32LE(offset+80),checksum:b.readUInt32LE(offset+88)};
}
const artifacts=[pe('/usr/lib/x86_64-linux-gnu/wine/i386-windows/msvcr71.dll'),pe('/usr/lib/x86_64-linux-gnu/wine/x86_64-windows/msvcr71.dll'),pe('/home/rich/copperfin-wine-vfp9/drive_c/windows/syswow64/msvcr71.dll')];
const observed=diagnostic.observation;
for(const [key,suffix] of Object.entries({machine:'Machine',magic:'Magic',stamp:'Stamp',imageSize:'Size',checksum:'Checksum'}))a.equal(parseInt(observed['image'+suffix],16),artifacts[0][key]);
a.equal(observed.fileSha256.toLowerCase(),artifacts[2].sha256);a.notEqual(artifacts[0].sha256,artifacts[2].sha256);
console.log(JSON.stringify({requirement:diagnostic.requirement,firstNativeSha256:diagnostic.firstSha256,artifacts,loadedHeadersMatchWine32:true,
 runtimeOpenedFileDigestMatchesMicrosoft32:true,loadedMicrosoftIdentityEstablished:false,
 limitation:'Offline file samples plus matching loaded PE header facts identify a Wine-builtin provenance candidate, not atomic image/file binding or complete instruction identity. Prior same-environment paired captures are observations, not Microsoft-CRT or Windows formatter identity proof.'},null,2));
