// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Record setting types and text without passing unknown return types to ALLTRIM.
const fs=require('node:fs'),path=require('node:path'),a=require('node:assert/strict');
const prior=JSON.parse(fs.readFileSync(path.join(__dirname,'rounding-decimals2-cases.json')));
const inv={...prior,settingTypesRecorded:true,
    limitation:'Type-aware native-feedback DECIMALS0..18/both signs. Same explicit input bits and bounds as prior refinement; previous readback ERROR rows remain unqualified. VARTYPE and hex TRANSFORM text record both native SET return types, without assuming character DECIMALS. Exact applied numeric count/mode, constructor/path/metadata/BRS/adjacent endpoints/paired-repeat required. No guessed tolerance, formatting model or general conversion algorithm.'};
let source=fs.readFileSync(path.join(__dirname,'rounding-decimals2.prg'),'utf8');
const old="+'|'+ALLTRIM(SET('DECIMALS'))+'|'+SET('FIXED')";
a.equal(source.split(old).length,2);
source=source.replace(old,"+'|'+VARTYPE(SET('DECIMALS'))+'|'+HexBytes(TRANSFORM(SET('DECIMALS')))+'|'+VARTYPE(SET('FIXED'))+'|'+HexBytes(TRANSFORM(SET('FIXED')))");
source=source.replace('default-transform-rounding-decimals2\\','default-transform-rounding-decimals3\\');
source=source.replace('refined native-feedback DECIMALS0..18 with settings readback.','type-aware native-feedback DECIMALS0..18 settings readback.');
if(process.argv[2]==='source')process.stdout.write(source);
else if(process.argv[2]==='cases')process.stdout.write(JSON.stringify(inv,null,2)+'\n');
else console.log(JSON.stringify({groups:38,maxQueriesPerGroup:54,maxRecords:4104}));
