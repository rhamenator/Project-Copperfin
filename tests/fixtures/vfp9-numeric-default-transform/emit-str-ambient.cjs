// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: public STR ambient-setting stimuli only.
'use strict';
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib'),a=require('node:assert/strict'),crypto=require('node:crypto');
const root=__dirname,read=n=>fs.readFileSync(path.join(root,n));
const bytes=read('str-ambient-selection-captures1/selection.out.gz');
const entry=JSON.parse(read('str-ambient-selection-captures1/manifest.json')).filter(r=>r.name==='selection.out.gz');a.equal(entry.length,1);
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');a.equal(sha(bytes),entry[0].sha256);
const raw=z.gunzipSync(bytes);a.equal(sha(raw),entry[0].decodedSha256);a.equal(raw.length,entry[0].decodedBytes);
const selection=JSON.parse(raw);a.deepEqual(selection.inputs.map(r=>r.index),[184,185,186,190,191,192]);
let source=read('str-middle.prg').toString();
const start=source.indexOf('LOCAL outputPath,');a(start>0);
source=source.slice(start);
const header=['* Copyright © 2026 Richard M. Hamilton.','* SPDX-License-Identifier: GPL-3.0-only',
 '* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: diagnostic ambient settings, not expected strings.',
 'LOCAL ARRAY inputHex[6],targetDecimals[6],sourceIndices[6]'];
for(const [i,r]of selection.inputs.entries())header.push(`inputHex[${i+1}]='${r.bits}'`,`targetDecimals[${i+1}]=15`,`sourceIndices[${i+1}]=${r.index}`);
source=header.join('\n')+'\nLOCAL ambientNo,ambientD,ambientDecimalType,ambientDecimalHex,ambientFixedType,ambientFixedHex\nLOCAL ambientMeta,ambientBits,ambientAfterMeta,ambientAfterBits,ambientAfterDecimalHex,ambientAfterFixedHex\n'+source;
source=source.replaceAll('default-transform-str-middle\\results.txt','default-transform-str-ambient\\results.txt');
const fixedMarker="  failed=.F.";a.equal(source.split(fixedMarker).length,2);
source=source.replace(fixedMarker,"  FOR ambientNo=1 TO 3\n   ambientD=IIF(ambientNo=1,0,IIF(ambientNo=2,13,18))\n   SET DECIMALS TO 15\n"+fixedMarker);
const marker='     str15Type=VARTYPE';a.equal(source.split(marker).length,2);
source=source.replace(marker,"     SET DECIMALS TO ambientD\n     ambientDecimalType=VARTYPE(SET('DECIMALS'))\n     ambientDecimalHex=HexBytes(TRANSFORM(SET('DECIMALS')))\n     ambientFixedType=VARTYPE(SET('FIXED'))\n     ambientFixedHex=HexBytes(TRANSFORM(SET('FIXED')))\n"+marker);
source=source.replace("     str15Type=VARTYPE", "     ambientMeta=CFNMETA(CFNDEC(CFNWIDTH(payload,40),18))\n     ambientBits=HexBytes(BINTOC(CFNDEC(CFNWIDTH(payload,40),18),'BRS'))\n     str15Type=VARTYPE");
source=source.replace('     afterMeta=CFNMETA',"     ambientAfterMeta=CFNMETA(CFNDEC(CFNWIDTH(payload,40),18))\n     ambientAfterBits=HexBytes(BINTOC(CFNDEC(CFNWIDTH(payload,40),18),'BRS'))\n     ambientAfterDecimalHex=HexBytes(TRANSFORM(SET('DECIMALS')))\n     ambientAfterFixedHex=HexBytes(TRANSFORM(SET('FIXED')))\n     SET DECIMALS TO 15\n     afterMeta=CFNMETA");
const lineMarker="+'|'+str18AfterHex";a.equal(source.split(lineMarker).length,2);
source=source.replace(lineMarker,lineMarker+"+'|'+ALLTRIM(STR(sourceIndices[index],6,0))+'|'+ALLTRIM(STR(ambientD,4,0))+'|'+ambientDecimalType+'|'+ambientDecimalHex+'|'+ambientFixedType+'|'+ambientFixedHex+'|'+ambientMeta+'|'+ambientBits+'|'+ambientAfterMeta+'|'+ambientAfterBits+'|'+ambientAfterDecimalHex+'|'+ambientAfterFixedHex");
source=source.replace('  total=total+1\n ENDFOR','  total=total+1\n  ENDFOR\n ENDFOR');
// 36 records, each with four explicit-count observations =144 observations.
if(process.argv[2]==='source')process.stdout.write(source);
else console.log(JSON.stringify({sourceOnly:true,nativeInvoked:false,records:36,observations:144,fields:44,selectionDecodedSha256:sha(raw)}));
