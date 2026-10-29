// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: first-run gate and paired qualification.
'use strict';
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),crypto=require('node:crypto'),cp=require('node:child_process'),a=require('node:assert/strict');
const {readRun,pair}=require('./read-str-ambient-run.cjs');
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');
function audit(root,single=false,io=fs){
 const read=n=>io.readFileSync(path.join(root,n));
 const selectionRaw=z.gunzipSync(fs.readFileSync(path.join(__dirname,'str-ambient-selection-captures1/selection.out.gz')));
 a.equal(sha(selectionRaw),'00bc1d0cf922c77c3847cb9b74cddde090a976723e886dfcc4ee20abcb76ea14');
 const selection=JSON.parse(selectionRaw);
 const generated=cp.spawnSync(process.execPath,[path.join(__dirname,'emit-str-ambient.cjs'),'source']);a.equal(generated.status,0);a.equal(generated.stderr.length,0);
 a(read('supplied-source.prg').equals(generated.stdout),'supplied source bytes differ');
 a.equal(read('supplied-runtime-env.txt').toString(),'WINEDLLOVERRIDES=msvcr71=n\n');
 a(['UNSET\n','\n'].includes(read('inherited-dll-overrides.txt').toString()));
 const run=name=>{a.equal(read(name+'.status').toString(),'0\n');a.equal(read(name+'.stderr').length,0);
  const raw=read(name+'-results.txt');a(raw.equals(read(name+'.out')),name+' stdout bytes differ');readRun(raw,selection);return raw;};
 const first=run('first'),base={sourceSha256:sha(generated.stdout),selectionSha256:sha(selectionRaw),firstSha256:sha(first),firstComplete:true,records:36};
 if(single)return {...base,pairedQualified:false};
 a.equal(read('first-structure.status').toString(),'0\n');a.equal(read('first-structure.stderr').length,0);
 a.deepEqual(JSON.parse(read('first-structure.json')),{...base,pairedQualified:false});
 const repeat=run('repeat');
 return {...base,...pair(first,repeat,selection),repeatSha256:sha(repeat),pairedQualified:true,
  limitation:'Finite public STR observations with exact parent and sampled runtime controls; not atomic mapped-image proof, Windows parity, internal calls or formatter acceptance.'};
}
module.exports={audit};
if(require.main===module){a(process.argv[2]);a([undefined,'--single'].includes(process.argv[3]));console.log(JSON.stringify(audit(process.argv[2],process.argv[3]==='--single'),null,2));}
