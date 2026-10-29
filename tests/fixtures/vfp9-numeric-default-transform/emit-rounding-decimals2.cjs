// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Refine failed stimuli; retain applied settings, never infer expected strings.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib');
const a=require('node:assert/strict');
const prior=JSON.parse(fs.readFileSync(path.join(__dirname,'rounding-decimals-cases.json')));
const edge=JSON.parse(zlib.gunzipSync(fs.readFileSync(path.join(__dirname,'rounding-edge-captures/audit.json.gz'))));
a.equal(edge.firstSha256,'b6e43abea8d3290d46e231a72e0ec6807c25551b266cfa156d91f393f4baacc2');a.equal(edge.repeatSha256,edge.firstSha256);
const witness=edge.observations.find(r=>r.group===9&&r.query===1&&r.fixed==='ON');a(witness);a.equal(witness.bits,'3CC203AF9EE75616');
const groups=prior.groups.map(g=>g.setting<=15?g:{...g,expression:'5e-16 (qualified native payload)',bits:witness.bits});
const inv={...prior,groups,parentNativeSha256:edge.firstSha256,appliedSettingsRequired:true,
    limitation:'Refined native-feedback DECIMALS0..18/both signs. For D16..18 reuse the qualified5e-16 payload instead of unbracketed tiny inputs; this is stimulus selection, NOT an inferred minimum fractional precision. Width40/scale18, both FIXED modes/query, explicit native SET DECIMALS/FIXED readback, exact input/path/metadata/BRS and paired adjacent edges. No output model, tolerance, monotonicity, descriptor-wide or magnitude-wide algorithm.'};
let source=fs.readFileSync(path.join(__dirname,'rounding-decimals.prg'),'utf8');
for(const g of groups)source=source.replace(new RegExp(`baseHex\\[${g.id}\\]='[0-9A-F]{16}'`),`baseHex[${g.id}]='${g.bits}'`);
source=source.replace('default-transform-rounding-decimals\\','default-transform-rounding-decimals2\\');
const suffix="+'|'+VARTYPE(TRANSFORM(CFNDEC(CFNWIDTH(payload,40),18)))";
a.equal(source.split(suffix).length,2);
source=source.replace(suffix,suffix+"+'|'+ALLTRIM(SET('DECIMALS'))+'|'+SET('FIXED')");
source=source.replace('native-feedback DECIMALS0..18 boundary stimuli.','refined native-feedback DECIMALS0..18 with settings readback.');
if(process.argv[2]==='source')process.stdout.write(source);
else if(process.argv[2]==='cases')process.stdout.write(JSON.stringify(inv,null,2)+'\n');
else console.log(JSON.stringify({groups:groups.length,maxQueriesPerGroup:54,maxRecords:4104}));
