// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Discriminate a rejected arithmetic hypothesis using native public operations.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib'),a=require('node:assert/strict');
const read=name=>JSON.parse(zlib.gunzipSync(fs.readFileSync(path.join(__dirname,name+'/audit.json.gz'))));
const decimals=read('rounding-decimals3-captures'),edges=read('rounding-edge-captures');
a.equal(decimals.firstSha256,'6972cb22232ddd776ec25491bb461b051e88c297d5e98d1bf9eea5bd519e1a3f');
a.equal(edges.firstSha256,'b6e43abea8d3290d46e231a72e0ec6807c25551b266cfa156d91f393f4baacc2');
for(const r of [decimals,edges]){a.equal(r.repeatSha256,r.firstSha256);a.equal(r.rawByteEqual,true);}
const boundaries=[...decimals.boundaries,...edges.boundaries.filter(g=>g.center>=2&&g.center<=4)],inputs=[];
a.equal(boundaries.length,44);
for(const boundary of boundaries)for(let offset=-2;offset<=3;offset++){
    const bits=(BigInt('0x'+boundary.lowBits)+BigInt(offset)).toString(16).padStart(16,'0').toUpperCase();
    a(Number.isFinite(Buffer.from(bits,'hex').readDoubleBE()));
    inputs.push({index:inputs.length+1,group:boundaries.indexOf(boundary)+1,setting:boundary.setting,bits,offset});
}
const inv={requirement:decimals.requirement,parentNativeSha256:[decimals.firstSha256,edges.firstSha256],
    inputs,groups:44,inputCount:264,recordsPerRun:528,width:40,scale:18,
    arithmeticFields:['powDown','bias','shifted','effectiveCount','powUp','scaled','raised','quantized'],
    limitation:'Native-only arithmetic discrimination around44 qualified signed local edges. Six explicit input payloads/group, both FIXED modes, unchanged source descriptors/applied settings. A provisional public-language operation graph is observed, NOT accepted as formatter or requirement: powDown=10^(-D), bias=.0005*powDown, shifted=ABS(input)+bias, q=MIN(D,15), powUp=10^q, scaled=shifted*powUp, raised=scaled+.5, quantized=INT(raised). Scalar assignments are part of this experiment. Actual TRANSFORM/STR strings remain independent observations; no tolerance, cap, operation-order, all-input or implementation parity claim.'};
let source=fs.readFileSync(path.join(__dirname,'rounding-ladder.prg'),'utf8');
const array=/LOCAL ARRAY inputHex\[\d+\]\n[\s\S]+?(?=LOCAL outputPath,)/g;a.equal([...source.matchAll(array)].length,1);
source=source.replace(array,`LOCAL ARRAY inputHex[264],targetDecimals[264]\n${inputs.map(r=>`inputHex[${r.index}]='${r.bits}'\ntargetDecimals[${r.index}]=${r.setting}`).join('\n')}\n`);
const cases=/ DO CASE\n[\s\S]+? ENDCASE\n SET DECIMALS TO decimalCount/g;a.equal([...source.matchAll(cases)].length,1);
source=source.replace(cases,' decimalCount=targetDecimals[index]\n SET DECIMALS TO decimalCount');
source=source.replace('default-transform-rounding-ladder\\','default-transform-rounding-arithmetic\\');
source=source.replace('native wide signed rounding ladder.','native public-arithmetic discrimination (not a formatter).');
source=source.replace('LOCAL outputPath,','LOCAL powDown,bias,shifted,effectiveCount,powUp,scaled,raised,quantized\nLOCAL outputPath,');
source=source.replace('  TRY\n',`  TRY\n   powDown=10^(-decimalCount)\n   bias=0.0005*powDown\n   shifted=ABS(payload)+bias\n   effectiveCount=MIN(decimalCount,15)\n   powUp=10^effectiveCount\n   scaled=shifted*powUp\n   raised=scaled+0.5\n   quantized=INT(raised)\n`);
const tail="+'|'+VARTYPE(TRANSFORM(CFNDEC(CFNWIDTH(payload,40),18)))";a.equal(source.split(tail).length,2);
source=source.replace(tail,tail+"+'|'+VARTYPE(SET('DECIMALS'))+'|'+HexBytes(TRANSFORM(SET('DECIMALS')))+'|'+VARTYPE(SET('FIXED'))+'|'+HexBytes(TRANSFORM(SET('FIXED')))"+
    inv.arithmeticFields.map(f=>`+'|'+HexBytes(BINTOC(${f},'BRS'))`).join('')+"+'|'"+inv.arithmeticFields.map(f=>`+VARTYPE(${f})`).join(''));
if(process.argv[2]==='source')process.stdout.write(source);
else if(process.argv[2]==='cases')process.stdout.write(JSON.stringify(inv,null,2)+'\n');
else console.log(JSON.stringify({groups:44,inputs:264,records:528}));
