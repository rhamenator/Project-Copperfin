// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// NON-NATIVE complete capture framing only; SAME-compiler whole-byte equality
// remains mandatory. Current behavior is NOT a requirement or native oracle.
const fs=require('fs'),assert=require('assert'),crypto=require('crypto');
assert.equal(process.argv.length,3,'Expected one complete328 capture');
const bytes=fs.readFileSync(process.argv[2]),lines=bytes.toString('utf8').trimEnd().split(/\r?\n/);
assert.equal(lines.pop(),'DONE|calls=328|scaffold_failures=0');assert.equal(lines.length,328);
const expected={'caption-type':4,'column-type':4,'nonordinary-id':9,arity:3,'binding-rst2':1,'binding-rst5':1,'nonempty-binding':1,'nonexact-rst':6,'ordinary-shared-state':4,'sorted-shared-state':4,'existing-id-control':4};
assert.equal(Object.values(expected).reduce((a,b)=>a+b,0),41);
const identities=new Set(),contexts=new Map();
for(const line of lines){
 const m=line.match(/^(ListBox|ComboBox)\|(COPPERFIN|VFP9)\|([12])\|([^|]+)\|([^|]+)\|completed=1\|cphase=invoke\|nbefore=3\|/);assert(m,'complete invocation and pre-existing three rows');
 assert(Object.hasOwn(expected,m[4]));const identity=m.slice(1,6).join('|');assert(!identities.has(identity));identities.add(identity);
 const context=m.slice(1,4).join('|');if(!contexts.has(context))contexts.set(context,{});const counts=contexts.get(context);counts[m[4]]=(counts[m[4]]??0)+1;
 for(const field of ['ncode','vresult','ncount','nfocus','nid','cvalue','cdisplay','nnewindex','nnewid','ccontext','cother','rowsourcetype','controlsource','ids','primary','selected','rows','data'])assert(line.includes('|'+field+'='),'retained '+field);
}
assert.equal(contexts.size,8);for(const counts of contexts.values())assert.deepEqual(counts,expected,'all41 per class/mode/session');
console.log(JSON.stringify({calls:328,contexts:8,uniqueCases:identities.size,groupsPerContext:expected,rawSHA256:crypto.createHash('sha256').update(bytes).digest('hex'),note:'NON-NATIVE capture framing ONLY; fresh baseline/fixed SAME-compiler full bytes must match. Not requirements recovery, native parity or executed evidence without actual status/raw.'}));
