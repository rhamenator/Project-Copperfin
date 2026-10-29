// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Archive/verify qualified public-observation evidence; proprietary build products remain host-only.
'use strict';
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),crypto=require('node:crypto'),a=require('node:assert/strict');
const fixture=__dirname,source=path.resolve(fixture,'../../../build/default-transform-sprintf'),target=path.join(fixture,'sprintf-captures1'),sha=b=>crypto.createHash('sha256').update(b).digest('hex');
const excluded=new Set(['binding-gnu','binding-clang','sprintf-gnu','sprintf-clang','crt-kernel32.lib','sprintf-probe.fll','sprintf-probe.lib','sprintf-probe.obj']);
function sourceNames(){return fs.readdirSync(source).filter(name=>{const p=path.join(source,name);return fs.statSync(p).isFile()&&!excluded.has(name);}).sort();}
function manifestFor(names,write){return names.map(name=>{const raw=fs.readFileSync(path.join(source,name)),gzip=z.gzipSync(raw,{level:9});if(write)fs.writeFileSync(path.join(target,name+'.gz'),gzip);return{name:name+'.gz',bytes:gzip.length,sha256:sha(gzip),decodedBytes:raw.length,decodedSha256:sha(raw)};});}
function verify(manifest){const names=sourceNames();a.deepEqual(manifest.map(e=>e.name.slice(0,-3)),names);for(const e of manifest){const gzip=fs.readFileSync(path.join(target,e.name)),raw=z.gunzipSync(gzip),host=fs.readFileSync(path.join(source,e.name.slice(0,-3)));a.equal(gzip.length,e.bytes);a.equal(sha(gzip),e.sha256);a.equal(raw.length,e.decodedBytes);a.equal(sha(raw),e.decodedSha256);a(raw.equals(host));}return names;}
if(process.argv[2]==='create'){
 a(fs.existsSync(source));a(!fs.existsSync(target));fs.mkdirSync(target);const manifest=manifestFor(sourceNames(),true);fs.writeFileSync(path.join(target,'manifest.json'),JSON.stringify(manifest,null,2)+'\n');fs.writeFileSync(path.join(target,'SHA256SUMS'),manifest.map(e=>`${e.sha256}  ${e.name}`).join('\n')+'\n');
 const names=verify(manifest),audit=JSON.parse(fs.readFileSync(path.join(source,'audit.json'))),first=JSON.parse(fs.readFileSync(path.join(source,'first-structure.json')));
 a.equal(audit.mismatchCount,1732);a.equal(first.mismatchCount,1732);a.deepEqual(audit.mismatches,first.mismatches);a.equal(audit.firstSha256,audit.repeatSha256);a.equal(audit.rawByteEqual,true);
 console.log(JSON.stringify({archiveFiles:names.length,manifestSha256:sha(fs.readFileSync(path.join(target,'manifest.json'))),firstSha256:audit.firstSha256,recordsPerRun:audit.recordsPerRun,mismatchCount:audit.mismatchCount,allMismatchesPreserved:true,excludedHostOnly:[...excluded].sort()}));
}else if(process.argv[2]==='verify'){
 const manifest=JSON.parse(fs.readFileSync(path.join(target,'manifest.json'))),names=verify(manifest);for(const line of fs.readFileSync(path.join(target,'SHA256SUMS'),'utf8').trim().split('\n')){const [digest,name]=line.split('  ');a.equal(digest,manifest.find(e=>e.name===name).sha256);}
 console.log(JSON.stringify({verifiedArchiveFiles:names.length,manifestSha256:sha(fs.readFileSync(path.join(target,'manifest.json'))),exactHostBytes:true}));
}else throw new Error('usage: archive-sprintf-evidence.cjs create|verify');
