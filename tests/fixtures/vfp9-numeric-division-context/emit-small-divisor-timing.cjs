// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Native source/inventory only; no product, candidate or expected-result import.
const a=require('node:assert/strict'),fs=require('node:fs'),path=require('node:path');
const settings=[0,2,6,14,18],rows=[];
const cores=['5/0.5','-5/0.5','5/-0.5','0.5/0.25','1/0.3','2.50/0.50',
    '1.234567890123456789/0.5','(1/3)/(1/3)','5/(1/3)','(5/2)/(1/3)',
    '0/0.5','-(0/0.5)'];
for(const core of cores)for(const changesTo of [0,14]){
    const e=`(${core})`,mark=`CFMark(${changesTo})`;
    for(const [expression,observesArgument] of [
        [`CFObserve(${e},${changesTo})`,true],
        [`-(CFObserve(${e},${changesTo}))`,true],
        [`CFObserve(-(${e}),${changesTo})`,true],
        [`${e}/${mark}`,false],
        [`-(${e})/${mark}`,false],
        [`-((${e})/${mark})`,false],
        [`${mark}/${e}`,false],
        [`${core}/${mark}`,false],
        [`IIF(.T.,${e},0)/${mark}`,false]])
        rows.push({index:rows.length+1,core,expression,changesTo,observesArgument,calls:1});
}
for(const expression of ['5/0.5','(1/3)/(1/3)','-(IIF(.T.,(0/0.5),0))'])
    rows.push({index:rows.length+1,expression,changesTo:null,observesArgument:false,calls:0,pureControl:true});
a.equal(rows.length,219);
const inventory={requirement:'RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-SMALL-DIVISOR-001',
    baseline:'240ac51cbbaa3114a08a687142e67fe4969ea212',settings,cores,rows,
    limitations:'Single evaluation through documented public-ABI observer. CFObserve sees AFTER LPARAMETERS normalization, not raw arguments. Retain zero/nonfinite controls; no checked-zero policy change or universal precision claim.'};
const old=fs.readFileSync(path.join(__dirname,'constant-effects.prg'),'utf8');
const body=old.slice(old.indexOf('LOCAL outputPath'),old.indexOf('\nFUNCTION CompiledObservation'));
const helpers=old.slice(old.indexOf('\nFUNCTION CFMark'));
const prg=`* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-CONSTANT-DIVISION-SMALL-DIVISOR-001: single-evaluation timing freeze.
LOCAL ARRAY expressions[${rows.length}],settings[5]
${rows.map(r=>`expressions[${r.index}]='${r.expression}'`).join('\n')}
${settings.map((d,i)=>`settings[${i+1}]=${d}`).join('\n')}
${body.replace('numeric-constant-division-5611','numeric-small-divisor-5611')
    .replace('build\\constant-division-native\\effects-results.txt','build\\small-divisor-native\\timing-results.txt')}

FUNCTION CompiledObservation
 LPARAMETERS index
 DO CASE
${rows.map(r=>` CASE index=${r.index}\n  RETURN CFNMETA(${r.expression})`).join('\n')}
 ENDCASE
 RETURN ''
${helpers}`;
if(process.argv[2]==='source')process.stdout.write(prg);
else if(process.argv[2]==='cases')process.stdout.write(JSON.stringify(inventory,null,2)+'\n');
else console.log(JSON.stringify({expressions:rows.length,rowsPerRun:rows.length*20,effectExpressions:216,pureControls:3}));
