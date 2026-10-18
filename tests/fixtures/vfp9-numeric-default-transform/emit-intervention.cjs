// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Stimulus only: no Copperfin, candidate or expected-result imports.
const a=require('node:assert/strict');
const seeds=['0.50','-0.50','0.00','-0.00','1E-14','1E-18','-1E-18','1.234567890123456789'];
const widths=[1,2,3,4,12,20,40],scales=[0,1,2,14,15,18],settings=[0,2,18];
const rows=[];
for(const seed of seeds){
    rows.push({kind:'baseline',seed,expression:seed});
    rows.push({kind:'clone',seed,expression:`CFNCLONE(${seed})`});
    for(const width of widths)for(const scale of scales)
        rows.push({kind:'intervention',seed,width,scale,
            expression:`CFNDEC(CFNWIDTH(${seed},${width}),${scale})`});
}
const rounding=['0.5','-0.5','1.5','-1.5','2.5','-2.5','2.675','-2.675',
    '1.005','-1.005','9.995','-9.995','0.499999999999999','0.500000000000001',
    '1.2345678901234545','1.2345678901234555','1.2345678901234565',
    '1.2345678901234575','1.2345678901234585','9.999999999999999',
    '99.999999999999999','999999999.123456789','-999999999.123456789',
    '12345678901234.5625','-12345678901234.5625','999999999999999.5',
    '999999999999999.9','1000000000000000.5','0.00000000000000049',
    '0.00000000000000050','0.00000000000000051','-0.00000000000000049'];
for(const expression of rounding)rows.push({kind:'rounding',expression});
rows.forEach((r,i)=>{r.index=i+1;a(!r.expression.includes("'"));});
const stored=['0.5','2.50','1.00','-0.00','1E-14','-1E-18',
    '1.234567890123456789','(0.5)','(2.50)','(1E-14)','(1E-18)','(1E+20)']
    .map((expression,i)=>({index:i+1,expression,name:`frozen${i+1}`}));
const inv={requirement:'RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001',
    baseline:'240ac51cbbaa3114a08a687142e67fe4969ea212',settings,seeds,widths,scales,rows,stored,
    recordsPerRun:rows.length*settings.length*2+stored.length*settings.length*2*settings.length*2,
    limitation:'EVALUATE-only pure controlled width/scale interventions with independent typed/BRS readback and unchanged clone controls; not a compiled-route equivalence claim. Stored private values are explicitly assigned compiled expressions once at each birth setting/FIXED state, then read under all display settings without reassignment. STR(value,40,current DECIMALS) is an observed diagnostic, not the expected TRANSFORM model. No callbacks, pictures, Currency, locale, division change, decompilation or VM.'};
const observation=x=>`CFNMETA(${x})+'|'+HexBytes(BINTOC(${x},'BRS'))+'|'+HexBytes(TRANSFORM(${x}))+'|'+HexBytes(STR(${x},40,decimalCount))+'|'+VARTYPE(TRANSFORM(${x}))`;
const source=`* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: native intervention/source freeze.
LOCAL ARRAY expressions[${rows.length}],settings[${settings.length}]
${rows.map(r=>`expressions[${r.index}]='${r.expression}'`).join('\n')}
${settings.map((s,i)=>`settings[${i+1}]=${s}`).join('\n')}
PRIVATE ${stored.map(r=>r.name).join(',')}
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,expression
LOCAL observation,line,total,caught,birthNo,birthSetting,birthFixedNo,birthFixed
outputPath='Z:\\home\\rich\\.codex\\worktrees\\numeric-literal-display-5611\\Project-Copperfin\\build\\default-transform-intervention\\results.txt'
libraryPath='Z:\\home\\rich\\dev\\Project-Copperfin\\build\\pr7203-completed\\native-evidence\\diagnostic\\cause-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
total=0
FOR settingNo=1 TO ALEN(settings)
 decimalCount=settings[settingNo]
 SET DECIMALS TO decimalCount
 FOR fixedNo=1 TO 2
  fixedMode=SetFixedMode(fixedNo)
  FOR index=1 TO ALEN(expressions)
   expression=expressions[index]
   TRY
    observation=${observation('EVALUATE(expression)')}
    line='GRID|-|-|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+observation
   CATCH TO caught
    line='ERROR-GRID|-|-|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
   ENDTRY
   =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
   total=total+1
  ENDFOR
 ENDFOR
ENDFOR
FOR birthNo=1 TO ALEN(settings)
 birthSetting=settings[birthNo]
 FOR birthFixedNo=1 TO 2
  SET DECIMALS TO birthSetting
  birthFixed=SetFixedMode(birthFixedNo)
${stored.map(r=>`  ${r.name}=${r.expression}`).join('\n')}
  FOR settingNo=1 TO ALEN(settings)
   decimalCount=settings[settingNo]
   SET DECIMALS TO decimalCount
   FOR fixedNo=1 TO 2
    fixedMode=SetFixedMode(fixedNo)
    FOR index=1 TO ${stored.length}
     TRY
      observation=FrozenObservation(index,decimalCount)
      line='STORED|'+ALLTRIM(STR(birthSetting,4,0))+'|'+birthFixed+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(FrozenSource(index))+'|'+observation
     CATCH TO caught
      line='ERROR-STORED|'+ALLTRIM(STR(birthSetting,4,0))+'|'+birthFixed+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
     ENDTRY
     =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
     total=total+1
    ENDFOR
   ENDFOR
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(total,6,0))+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO

FUNCTION SetFixedMode
 LPARAMETERS fixedNo
 IF fixedNo=1
  SET FIXED OFF
  RETURN 'OFF'
 ENDIF
 SET FIXED ON
 RETURN 'ON'

FUNCTION FrozenObservation
 LPARAMETERS index,decimalCount
 DO CASE
${stored.map(r=>` CASE index=${r.index}\n  RETURN ${observation(r.name)}`).join('\n')}
 ENDCASE
 RETURN ''

FUNCTION FrozenSource
 LPARAMETERS index
 DO CASE
${stored.map(r=>` CASE index=${r.index}\n  RETURN '${r.expression}'`).join('\n')}
 ENDCASE
 RETURN ''

FUNCTION HexBytes
 LPARAMETERS value
 LOCAL result,index,byteValue
 result=''
 FOR index=1 TO LEN(value)
  byteValue=ASC(SUBSTR(value,index,1))
  result=result+SUBSTR('0123456789ABCDEF',INT(byteValue/16)+1,1)+SUBSTR('0123456789ABCDEF',MOD(byteValue,16)+1,1)
 ENDFOR
 RETURN result
`;
if(process.argv[2]==='source')process.stdout.write(source);
else if(process.argv[2]==='cases')process.stdout.write(JSON.stringify(inv,null,2)+'\n');
else console.log(JSON.stringify({grid:rows.length,stored:stored.length,recordsPerRun:inv.recordsPerRun}));
