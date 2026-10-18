// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Forensic framing/build receipt check ONLY; never qualifies this failed run.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),crypto=require('node:crypto');
const root=process.argv[2];a(root);
const exists=n=>fs.existsSync(path.join(root,n))||fs.existsSync(path.join(root,n+'.gz'));
const bytes=n=>fs.existsSync(path.join(root,n))?fs.readFileSync(path.join(root,n)):z.gunzipSync(fs.readFileSync(path.join(root,n+'.gz')));
const raw=bytes('first-results.txt'),lines=raw.toString('ascii').split('\r\n');a.equal(lines.pop(),'');
a.equal(lines.shift(),'VERSION|Visual FoxPro 09.00.0000.7423 for Windows');a.equal(lines.length,1626);
for(const line of lines){const fields=line.split('|');a.equal(fields.length,20);a.equal(fields[16],'ERROR:MODULEBINDING');}
a.equal(bytes('first.status').toString(),'124\n');a.equal(bytes('first.out').length,0);
a.equal(bytes('first.stderr').toString(),"vfp9-probe: timed out after 25 seconds\nvfp9-probe: partial output: ''\n");
for(const n of ['repeat.started.utc','first-structure.status','audit.status'])a(!exists(n));
for(const name of ['host-gnu','check-gnu','host-clang','check-clang','import','compile','link']){
 a.equal(bytes(name+'.status').toString(),'0\n');a.equal(bytes(name+'.guard.status').toString(),'0\n');a.equal(bytes(name+'.prefix.status').toString(),'1\n');
}
for(const comp of ['gnu','clang']){
 const checks=JSON.parse(bytes('binding-'+comp+'.checks.json'));a.equal(checks.length,72);
 for(const r of checks){a.equal(r.status,0);a.equal(r.stderr,'');a.equal(r.prefixStatus,1);const full=r.pressure.split('\n').find(s=>s.startsWith('full '));const v=Object.fromEntries(full.split(' ').slice(1).map(s=>s.split('=')));a(+v.avg10<1&&+v.avg60<1);}
}
console.log(JSON.stringify({qualified:false,pairedQualified:false,firstNativeStatus:124,partialRows:1626,allRowsModuleBindingError:true,footer:false,repeatInvoked:false,
 firstRawSha256:crypto.createHash('sha256').update(raw).digest('hex'),directChecksPerCompiler:72,buildStages0:true,
 limitation:'Forensic build/framing verification only. Generic binding error does not identify which internal check failed. No ABI/conversion/formatter or loaded-module/backing-file qualification.'}));
