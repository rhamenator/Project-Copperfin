// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Probe/inventory only: no product, candidate or expected-result import.
const a = require('node:assert/strict');
const pairs = [
    ['5','0.5'], ['1','0.5'], ['0','0.5'], ['0.5','0.25'],
    ['2.50','0.50'], ['1','0.3'], ['1','0.25'],
    ['0.000000000000000001','0.5'], ['1.234567890123456789','0.5'],
    ['5','00.50'], ['1','0.125'], ['0','0.000000000000000001'],
];
const cores = [], literals = new Set(['2','3','7']);
const add = (expression, label) => cores.push({id:cores.length+1, expression, label});
for (const [left,right] of pairs) {
    literals.add(left); literals.add(right);
    for (const [ls,rs] of [[false,false],[true,false],[false,true],[true,true]])
        add(`(${ls?`-(${left})`:left}/${rs?`-(${right})`:right})`, 'literal small divisor/sign');
}
for (const expression of ['(1/3)/(1/3)', '5/(1/3)', '(1/7)/(1/7)',
    '(0.5/2)/(0.5/2)', '((1/3)/(1/3))/2', '(1/3)/((1/3)/2)',
    '(5/2)/(1/3)', '((5/2)/(1/3))/2']) add(expression,'nested small divisor');
a.equal(cores.length,56);
const rows = [], addRow = row => rows.push({index:rows.length+1,...row});
for (const core of cores) for (const wrapper of ['direct','minus','iif','minus-iif']) {
    const expression = {direct:core.expression, minus:`-(${core.expression})`,
        iif:`IIF(.T.,${core.expression},0)`, 'minus-iif':`-(IIF(.T.,${core.expression},0))`}[wrapper];
    addRow({kind:'core',core:core.id,wrapper,expression});
}
for (const expression of literals) addRow({kind:'literal-control',expression});
for (const expression of ['(5/0.5)/iTwo','iTwo/(1/3)','(1/3)/nTwo',
    'nTwo/(1/3)','-((5/0.5)/iTwo)','-(iTwo/(1/3))'])
    addRow({kind:'runtime-mixture',expression});
for (const expression of ['1/0','0/0','5/-0.00','1/0.0000000001',
    '1/0.000000000000000001','1E-14/0.5','9999999999/0.5','5/1E-2'])
    addRow({kind:'retained-boundary',expression});
for (const row of rows) { a(!row.expression.includes("'")); a(row.expression.length<240); }
const inventory = {requirement:'RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-SMALL-DIVISOR-001',
    baseline:'240ac51cbbaa3114a08a687142e67fe4969ea212', settings:[0,2,6,14,18],cores,rows,
    setup:{iTwo:'2',nTwo:'2.00'}, literals:[...literals],
    limitation:'Native complete roots only. Literal controls are not folded-child descriptors. Pure roots replay for metadata/BRS/display; no callbacks. Boundary records are retained observations, not authorized zero/nonfinite/conversion policy changes.'};
const prg = `* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-SMALL-DIVISOR-001: native-only source freeze.
LOCAL ARRAY expressions[${rows.length}],settings[5]
${rows.map(r=>`expressions[${r.index}]='${r.expression}'`).join('\n')}
settings[1]=0
settings[2]=2
settings[3]=6
settings[4]=14
settings[5]=18
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,expression
LOCAL observation,line,total,caught,modeNo,mode
PRIVATE iTwo,nTwo
iTwo=2
nTwo=2.00
outputPath='Z:\\home\\rich\\.codex\\worktrees\\numeric-small-divisor-5611\\Project-Copperfin\\build\\small-divisor-native\\grid-results.txt'
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
   FOR modeNo=1 TO 2
    IF modeNo=1
     mode='EVAL'
    ELSE
     mode='COMPILED'
    ENDIF
    TRY
     IF modeNo=1
      observation=CFNMETA(EVALUATE(expression))+'|'+HexBytes(BINTOC(EVALUATE(expression),'BRS'))+'|'+HexBytes(TRANSFORM(EVALUATE(expression)))
     ELSE
      observation=CompiledObservation(index)
     ENDIF
     line=mode+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+observation
    CATCH TO caught
     line='ERROR-'+mode+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+fixedMode+'|'+ALLTRIM(STR(index,4,0))+'|'+HexBytes(expression)+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
    ENDTRY
    =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
    total=total+1
   ENDFOR
  ENDFOR
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(total,6,0))+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO

FUNCTION CompiledObservation
 LPARAMETERS index
 DO CASE
${rows.map(r=>` CASE index=${r.index}\n  RETURN CFNMETA(${r.expression})+'|'+HexBytes(BINTOC(${r.expression},'BRS'))+'|'+HexBytes(TRANSFORM(${r.expression}))`).join('\n')}
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
if (process.argv[2]==='source') process.stdout.write(prg);
else if (process.argv[2]==='cases') process.stdout.write(JSON.stringify(inventory,null,2)+'\n');
else console.log(JSON.stringify({cores:cores.length,expressions:rows.length,rowsPerRun:rows.length*20,literals:literals.size}));
