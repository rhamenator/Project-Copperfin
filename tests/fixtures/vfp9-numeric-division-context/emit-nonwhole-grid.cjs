// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Native probe source/inventory only. No Copperfin helper/expected-result import.
const a = require('node:assert/strict');
const lit = text => ({kind: 'literal', text});
const div = (left, right) => ({kind: 'divide', left, right});
const neg = operand => ({kind: 'negate', operand});
const show = n => n.kind === 'literal' ? n.text : n.kind === 'negate' ? `-(${show(n.operand)})`
    : `(${show(n.left)}/${show(n.right)})`;
const cores = [];
const add = (tree, label) => cores.push({id: cores.length + 1, label, tree, expression: show(tree)});
for (const l of ['0.5', '2.50', '0.000000000000000001', '1.234567890123456789'])
    for (const r of ['2', '2.5', '02.50'])
        for (const [ls, rs] of [[false, false], [true, false], [false, true], [true, true]])
            add(div(ls ? neg(lit(l)) : lit(l), rs ? neg(lit(r)) : lit(r)), 'literal pair/sign');
const q = (l, r) => div(lit(l), lit(r));
for (const tree of [
    div(q('5', '2'), q('5', '2')), div(q('1', '7'), lit('3')),
    div(lit('5'), q('5', '2')), div(q('2', '3'), lit('7')),
    div(q('1', '3'), lit('7')), div(q('1', '3'), q('7', '3')),
    div(div(q('1', '7'), lit('3')), lit('3')),
    div(q('5.00', '2.00'), q('7.0', '3.0')), div(q('0.5', '2'), lit('2')),
    div(div(q('5', '2'), q('5', '2')), lit('3')),
    div(div(q('5', '2'), lit('2')), q('7', '3')),
    div(q('2.50', '2.5'), q('5', '2')),
    div(div(neg(lit('5')), lit('2')), q('7', '3')),
    div(div(lit('5'), neg(lit('2'))), q('7', '3')),
    div(q('5', '2'), div(neg(lit('7')), lit('3'))),
    div(q('5', '2'), div(lit('7'), neg(lit('3')))),
    q('1.0000000000000002', '2.5'), q('0.9999999999999999', '2.5'),
    q('3.9999999999999996', '2.5'), q('1.234567890123456789', '3')
]) add(tree, 'nested or mantissa control');
a.equal(cores.length, 68);
const wrappers = ['direct', 'plus', 'minus', 'double-minus', 'iif', 'minus-iif', 'evaluate', 'minus-evaluate'];
const wrap = (e, w) => ({direct:e, plus:`+(${e})`, minus:`-(${e})`, 'double-minus':`-(-(${e}))`,
    iif:`IIF(.T.,${e},0)`, 'minus-iif':`-(IIF(.T.,${e},0))`,
    evaluate:`EVALUATE("${e}")`, 'minus-evaluate':`-(EVALUATE("${e}"))`})[w];
const rows = [];
const row = r => rows.push({index: rows.length + 1, ...r});
for (const core of cores) for (const wrapper of wrappers)
    row({kind:'core', core:core.id, wrapper, expression:wrap(core.expression, wrapper)});
const literals = new Set();
function collect(n) {
    if (n.kind === 'literal') literals.add(n.text);
    else if (n.kind === 'negate') collect(n.operand);
    else { collect(n.left); collect(n.right); }
}
for (const c of cores) collect(c.tree);
for (const text of [...literals]) row({kind:'literal-control', literal:text, expression:text});
for (const expression of ['(1/7)/iTwo', 'iTwo/(5/2)', '((5/2)/(5/2))/iTwo',
    'iTwo/((5/2)/(5/2))', '(2.50/2)/nTwo', 'nTwo/(2.50/2)',
    '-((1/7)/iTwo)', '-(iTwo/(5/2))', '((1/7)/3)/iTwo',
    '((5/2)/(7/3))/iTwo', '-(((5/2)/(7/3))/iTwo)', 'iTwo/((7/3)/(5/2))'])
    row({kind:'runtime-mixture', expression});
for (const expression of ['5/0.5', '(1/3)/(1/3)', '1E-14/2', '9999999999/2.5'])
    row({kind:'retained-outside-scope', expression});
for (const r of rows) { a(!r.expression.includes("'")); a(r.expression.length < 240); }
const inventory = {requirement:'RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-NONWHOLE-001',
    baseline:'ef3ea326f78d640546363abe97bbe27b6208bf45', settings:[0,2,6,14,18], cores, rows,
    setup:{iTwo:'2',nTwo:'2.00'}, limitation:'Complete-expression fields. Separate literal controls are not child-result oracles for enclosing folded trees. Pure expressions replayed for independent BRS and TRANSFORM; no callbacks in this grid.'};
const prg = `* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-NONWHOLE-001: complete-expression native observations.
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
outputPath='Z:\\home\\rich\\.codex\\worktrees\\numeric-nonwhole-division-5611\\Project-Copperfin\\build\\nonwhole-native\\grid-results.txt'
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
if (process.argv[2] === 'source') process.stdout.write(prg);
else if (process.argv[2] === 'cases') process.stdout.write(JSON.stringify(inventory,null,2)+'\n');
else console.log(JSON.stringify({cores:cores.length,expressions:rows.length,rowsPerRun:rows.length*20,literals:literals.size}));
