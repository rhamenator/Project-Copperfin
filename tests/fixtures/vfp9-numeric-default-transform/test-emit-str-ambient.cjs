// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
'use strict';
const a=require('node:assert/strict'),cp=require('node:child_process'),path=require('node:path');
const emitter=path.join(__dirname,'emit-str-ambient.cjs');
const r=cp.spawnSync(process.execPath,[emitter,'source'],{encoding:'utf8'});a.equal(r.status,0);a.equal(r.stderr,'');
const source=r.stdout;let checks=0;const check=f=>{f();++checks;};
check(()=>a(source.includes('LOCAL ARRAY inputHex[6],targetDecimals[6],sourceIndices[6]')));
check(()=>a.equal((source.match(/^sourceIndices\[\d\]=/gm)||[]).length,6));
check(()=>a.equal((source.match(/^\s*FOR /gm)||[]).length,(source.match(/^\s*ENDFOR$/gm)||[]).length));
check(()=>a(source.includes('FOR ambientNo=1 TO 3')));
check(()=>a(source.includes('ambientD=IIF(ambientNo=1,0,IIF(ambientNo=2,13,18))')));
const enter=source.indexOf('SET DECIMALS TO ambientD'),reset=source.indexOf('SET DECIMALS TO 15',enter);
check(()=>a(enter>0&&reset>enter));
for(const field of ['ambientMeta','ambientBits','ambientAfterMeta','ambientAfterBits','ambientAfterDecimalHex','ambientAfterFixedHex'])
 check(()=>{const pos=source.indexOf(field+'=',enter);a(pos>enter&&pos<reset);});
for(const count of [15,16,17,18])check(()=>{const call=`HexBytes(STR(CFNDEC(CFNWIDTH(payload,40),18),40,${count}))`;const pos=source.indexOf(call,enter);a(pos>enter&&pos<reset);});
const line=source.split('\n').find(l=>l.trimStart().startsWith("line=fixedMode+'|'"));
check(()=>a.equal((line.match(/\+'\|'/g)||[]).length,43));
check(()=>a(line.endsWith("+'|'+ambientMeta+'|'+ambientBits+'|'+ambientAfterMeta+'|'+ambientAfterBits+'|'+ambientAfterDecimalHex+'|'+ambientAfterFixedHex")));
check(()=>a(!/\bargument\s*=/i.test(source)));
check(()=>a(source.includes('IF failed\n   SET LIBRARY TO\n   RETURN')));
console.log(JSON.stringify({checks,passed:true,fields:44,records:36,nativeInvoked:false}));
