// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Native stimulus/inventory only. No product, model, or expected-result imports.
const a = require('node:assert/strict');
const literals = [
    '2','3','7','5','1','0','0.5','0.25','2.50','0.50','0.3',
    '0.000000000000000001','1.234567890123456789','00.50','0.125',
    '0.0','0.00','-0.00','-0.5','-2.50','1.00','123.4500','00123.4500',
    '1.234567','1.23456789','1.23456789012345','1.234567890123456',
    '1.2345678901234567','1.23456789012345678',
    '0.00001','0.0000000001','0.00000000000001',
    '0.000000000000001','0.0000000000000001','0.00000000000000001',
    '0.0000000000000000010','-0.000000000000000001',
    '9.99999999999999','9.999999999999999','99.99999999999999',
    '999999999.5','9999999999.5','999999999999999.5',
    '2147483647','2147483648','-2147483648','-2147483649',
    '9999999999999999','99999999999999999','999999999999999999',
    '1E-1','1E-14','1E-18','1E-20','1.25E-2','1E+20','1E+100','1E+308',
];
a.equal(new Set(literals).size,literals.length);
const rows=literals.map(expression=>({kind:'literal',expression}));
for(const literal of ['0.5','2.50','0.00','0.000000000000000001',
    '1.234567890123456789','2147483648','1E-18','1E+20'])
    for(const wrapper of ['group','minus','iif'])rows.push({kind:wrapper,
        expression:wrapper==='group'?`(${literal})`:wrapper==='minus'?`-(${literal})`:
            `IIF(.T.,${literal},0)`});
rows.forEach((row,i)=>{row.index=i+1;a(!row.expression.includes("'"));});
const settings=[0,1,2,6,14,18];
const routes=['EVAL','COMPILED','STORE-EVAL','STORE-COMPILED'];
const inventory={requirement:'RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001',
    parent:'RQ-CF-PRG-NUMERIC-METADATA-001',baseline:'240ac51cbbaa3114a08a687142e67fe4969ea212',
    settings,routes,literals,rows,recordsPerRun:rows.length*settings.length*2*routes.length,
    limitation:'Finite literal/default TRANSFORM recovery, not producer or arithmetic policy. Pure roots replay for metadata/BRS/display. Store routes assign to a local at the current settings before observing; they are not evidence for values assigned under another setting. No callbacks, fields, Currency, pictures, locale changes, VM, or implementation imports.'};
const observation=expression=>`CFNMETA(${expression})+'|'+HexBytes(BINTOC(${expression},'BRS'))+'|'+HexBytes(TRANSFORM(${expression}))+'|'+VARTYPE(TRANSFORM(${expression}))`;
const source=`* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: immutable native stimulus, not an oracle model.
LOCAL ARRAY expressions[${rows.length}],settings[${settings.length}]
${rows.map(r=>`expressions[${r.index}]='${r.expression}'`).join('\n')}
${settings.map((s,i)=>`settings[${i+1}]=${s}`).join('\n')}
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,expression
LOCAL observation,line,total,caught,routeNo,route
outputPath='Z:\\home\\rich\\.codex\\worktrees\\numeric-literal-display-5611\\Project-Copperfin\\build\\default-transform-native\\results.txt'
libraryPath='Z:\\home\\rich\\dev\\Project-Copperfin\\build\\pr7203-completed\\native-evidence\\diagnostic\\cause-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
total=0
FOR settingNo=1 TO ALEN(settings)
 decimalCount=settings[settingNo]
 SET DECIMALS TO decimalCount
 FOR fixedNo=1 TO 2
  IF fixedNo=1
   SET FIXED OFF
   fixedMode='OFF'
  ELSE
   SET FIXED ON
   fixedMode='ON'
  ENDIF
  FOR index=1 TO ALEN(expressions)
   expression=expressions[index]
   FOR routeNo=1 TO 4
    route=GETWORDNUM('EVAL COMPILED STORE-EVAL STORE-COMPILED',routeNo)
    TRY
     DO CASE
     CASE routeNo=1
      observation=${observation('EVALUATE(expression)')}
     CASE routeNo=2
      observation=CompiledObservation(index,.F.)
     CASE routeNo=3
      observation=StoredEvalObservation(expression)
     CASE routeNo=4
      observation=CompiledObservation(index,.T.)
     ENDCASE
     line=route+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+observation
    CATCH TO caught
     line='ERROR-'+route+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
    ENDTRY
    =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
    total=total+1
   ENDFOR
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(total,6,0))+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO

FUNCTION StoredEvalObservation
 LPARAMETERS expression
 LOCAL stored
 stored=EVALUATE(expression)
 RETURN ${observation('stored')}

FUNCTION CompiledObservation
 LPARAMETERS index,storeFirst
 LOCAL stored
 DO CASE
${rows.map(r=>` CASE index=${r.index}
  IF storeFirst
   stored=${r.expression}
   RETURN ${observation('stored')}
  ENDIF
  RETURN ${observation(r.expression)}`).join('\n')}
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
else if(process.argv[2]==='cases')process.stdout.write(JSON.stringify(inventory,null,2)+'\n');
else console.log(JSON.stringify({expressions:rows.length,literals:literals.length,recordsPerRun:inventory.recordsPerRun}));
