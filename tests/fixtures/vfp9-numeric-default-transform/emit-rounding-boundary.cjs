// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Native stimulus only. Centers come from qualified native payload readbacks,
// never Copperfin parsing, a formatter candidate, or formatted expectations.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib');
const a=require('node:assert/strict');
const actual=JSON.parse(zlib.gunzipSync(fs.readFileSync(path.join(__dirname,'intervention-captures/audit.json.gz'))));
const prior=JSON.parse(fs.readFileSync(path.join(__dirname,'intervention-cases.json')));
a.equal(actual.firstSha256,'45d1e318ba670f980f191c574a83d7d14206069468f35a53827537469018bea2');
a.equal(actual.repeatSha256,actual.firstSha256);a.equal(actual.rawByteEqual,true);
const requests=[['0.5',0,-512,32],['1.005',2,-32,32],['2.675',2,-32,32],
    ['9.995',2,-32,32],['0.00000000000000050',15,-32,32],
    ['1.2345678901234565',15,-32,32],['999999999.123456789',7,-32,32],
    ['12345678901234.5625',2,-32,32]];
const centers=[],inputs=[];
for(const [expression,setting,low,high]of requests){
    const item=prior.rows.find(r=>r.kind==='rounding'&&r.expression===expression);a(item);
    const row=actual.observations.find(r=>r.family==='GRID'&&r.index===item.index&&r.setting===0&&r.fixed==='OFF');a(row);
    const center={id:centers.length+1,expression,setting,low,high,bits:row.bits,first:inputs.length+1};
    for(let offset=low;offset<=high;++offset){
        const bits=(BigInt('0x'+row.bits)+BigInt(offset)).toString(16).padStart(16,'0').toUpperCase();
        a(Number.isFinite(Buffer.from(bits,'hex').readDoubleBE()));
        inputs.push({index:inputs.length+1,center:center.id,offset,bits,setting});
    }
    center.last=inputs.length;centers.push(center);
}
const inv={requirement:'RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001',
    nativeCenterSha256:actual.firstSha256,centers,inputCount:inputs.length,
    recordsPerRun:inputs.length*2,width:40,scale:18,
    limitation:'Adjacent finite positive binary64 inputs around eight observed native payloads. CTOBIN(BS) identity and public-ABI width40/scale18 clone must be read back exactly before acceptance. Both FIXED modes at each declared critical DECIMALS setting; no general tolerance/algorithm or compiled-route claim. No callbacks, division/product change, VM, FLL rebuild or decompilation.'};
const value='CFNDEC(CFNWIDTH(payload,40),18)';
const source=`* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: native adjacent-payload stimulus.
LOCAL ARRAY inputHex[${inputs.length}]
${inputs.map(r=>`inputHex[${r.index}]='${r.bits}'`).join('\n')}
LOCAL outputPath,libraryPath,index,fixedNo,fixedMode,decimalCount,payload,line,caught,total
outputPath='Z:\\home\\rich\\.codex\\worktrees\\numeric-literal-display-5611\\Project-Copperfin\\build\\default-transform-rounding\\results.txt'
libraryPath='Z:\\home\\rich\\dev\\Project-Copperfin\\build\\pr7203-completed\\native-evidence\\diagnostic\\cause-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
total=0
FOR index=1 TO ALEN(inputHex)
 DO CASE
${centers.map(c=>` CASE index<=${c.last}\n  decimalCount=${c.setting}`).join('\n')}
 ENDCASE
 SET DECIMALS TO decimalCount
 payload=CTOBIN(Unhex(inputHex[index]),'BS')
 FOR fixedNo=1 TO 2
  IF fixedNo=1
   SET FIXED OFF
   fixedMode='OFF'
  ELSE
   SET FIXED ON
   fixedMode='ON'
  ENDIF
  TRY
   line=fixedMode+'|'+ALLTRIM(STR(index,6,0))+'|'+inputHex[index]+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+CFNMETA(${value})+'|'+HexBytes(BINTOC(${value},'BRS'))+'|'+HexBytes(TRANSFORM(${value}))+'|'+HexBytes(STR(${value},40,decimalCount))+'|'+VARTYPE(TRANSFORM(${value}))
  CATCH TO caught
   line='ERROR-'+fixedMode+'|'+ALLTRIM(STR(index,6,0))+'|'+inputHex[index]+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
  ENDTRY
  =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
  total=total+1
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(total,6,0))+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO

FUNCTION Unhex
 LPARAMETERS value
 LOCAL result,position,high,low
 result=''
 FOR position=1 TO LEN(value) STEP 2
  high=AT(SUBSTR(value,position,1),'0123456789ABCDEF')-1
  low=AT(SUBSTR(value,position+1,1),'0123456789ABCDEF')-1
  result=result+CHR(high*16+low)
 ENDFOR
 RETURN result

FUNCTION HexBytes
 LPARAMETERS value
 LOCAL result,position,byteValue
 result=''
 FOR position=1 TO LEN(value)
  byteValue=ASC(SUBSTR(value,position,1))
  result=result+SUBSTR('0123456789ABCDEF',INT(byteValue/16)+1,1)+SUBSTR('0123456789ABCDEF',MOD(byteValue,16)+1,1)
 ENDFOR
 RETURN result
`;
if(process.argv[2]==='source')process.stdout.write(source);
else if(process.argv[2]==='cases')process.stdout.write(JSON.stringify(inv,null,2)+'\n');
else console.log(JSON.stringify({centers:centers.length,inputs:inputs.length,recordsPerRun:inv.recordsPerRun}));
