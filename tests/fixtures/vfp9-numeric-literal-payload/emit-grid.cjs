// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Native source/inventory only; no expected payload, Copperfin or candidate import.
const a = require('node:assert/strict');
const bases = new Map();
const add = (literal,label) => { a.match(literal,/^[0-9]+\.[0-9]+$/); if(!bases.has(literal)) bases.set(literal,{literal,label}); };
const decimal = (coefficient,scale) => {
    const digits=coefficient.toString().padStart(scale+1,'0');
    return digits.slice(0,-scale)+'.'+digits.slice(-scale);
};
for (const literal of ['0.0','0.00','0.1','0.2','0.29','0.3','0.5','2.50','02.50','1.00',
    '0.000000000000000001','1.234567890123456789','0.9999999999999999',
    '1.0000000000000002','3.9999999999999996','0.123456789012345678']) add(literal,'retained or short control');
for (const center of [9007199254740992n,9999999999999999n,1234567890123456789n,
    999999999999999999n,1000000000000000000n])
    for(let offset=-4n;offset<=4n;++offset)
        for(const scale of [1,8,16,18]) add(decimal(center+offset,scale),'adjacent coefficient/scale intervention');
for(const literal of ['0.9999999999999999','1.234567890123456789','1.0000000000000002',
    '3.9999999999999996','0.1','0.29'])
    for(const spelling of ['0'+literal,'00'+literal,literal+'0',literal+'00']) add(spelling,'same decimal rational/source-zero intervention');
const digits='1234567890123456789012345678901234';
for(let length=1;length<=32;++length) {
    const coefficient=BigInt(digits.slice(0,length));
    add(decimal(coefficient,1),'coefficient length intervention');
    add(decimal(coefficient,length),'fraction scale/length intervention');
}
const rows=[];
for(const base of bases.values()) {
    for(const wrapper of ['literal','lexical-minus','group-minus','iif']) {
        const expression=wrapper==='literal'?base.literal:wrapper==='lexical-minus'?'-'+base.literal:
            wrapper==='group-minus'?`-(${base.literal})`:`IIF(.T.,${base.literal},0)`;
        rows.push({index:rows.length+1,...base,wrapper,expression});
    }
}
a(rows.length<1400);
const inventory={requirement:'RQ-CF-PRG-NUMERIC-DECIMAL-LITERAL-PAYLOAD-001',
    baseline:'ef3ea326f78d640546363abe97bbe27b6208bf45',parents:[5611,6776,7189],trackingIssue:7206,
    settings:[0,18],bases:[...bases.values()],rows,
    limitation:'Plain decimal source literals and pure unary/IIF materialization only. Full expression observations; no VAL/scientific/omitted-leading-zero grammar/consumer algorithm or product parity claim.'};
const prg=`* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DECIMAL-LITERAL-PAYLOAD-001: observed native conversion, no expected values.
LOCAL ARRAY expressions[${rows.length}],settings[2]
${rows.map(r=>`expressions[${r.index}]='${r.expression}'`).join('\n')}
settings[1]=0
settings[2]=18
LOCAL outputPath,libraryPath,settingNo,fixedNo,decimalCount,fixedMode,index,expression
LOCAL observation,line,total,caught,modeNo,mode
outputPath='Z:\\home\\rich\\.codex\\worktrees\\numeric-literal-payload-7206\\Project-Copperfin\\build\\literal-payload-native\\grid-results.txt'
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
if(process.argv[2]==='source') process.stdout.write(prg);
else if(process.argv[2]==='cases') process.stdout.write(JSON.stringify(inventory,null,2)+'\n');
else console.log(JSON.stringify({bases:bases.size,expressions:rows.length,observationsPerRun:rows.length*8}));
