// Verification-only original/fixed preservation capture audit; no native oracle.
const fs=require('fs'),assert=require('assert'),crypto=require('crypto');
const [file]=process.argv.slice(2);assert(file&&process.argv.length===3,'Expected one complete304 capture');
const bytes=fs.readFileSync(file),lines=bytes.toString('utf8').trimEnd().split(/\r?\n/);
assert.strictEqual(lines.pop(),'DONE|calls=304|scaffold_failures=0');
assert.strictEqual(lines.length,304);
const expected={'caption-type':4,'column-type':4,'nonordinary-id':9,arity:3,'binding-rst2':1,'binding-rst5':1,'nonempty-binding':1,'nonexact-rst':6,'ordinary-shared-state':4,'sorted-shared-state':4,'missing-id':1};
const identities=new Set(),contexts=new Map();
for(const line of lines){
 const match=line.match(/^(ListBox|ComboBox)\|(COPPERFIN|VFP9)\|([12])\|([^|]+)\|([^|]+)\|completed=1\|cphase=invoke\|nbefore=3\|/);
 assert(match,'completed invocation/pre-existing three rows');
 assert(Object.hasOwn(expected,match[4]),'known bounded group');
 const identity=match.slice(1,6).join('|');assert(!identities.has(identity),'unique case');identities.add(identity);
 const context=match.slice(1,4).join('|');if(!contexts.has(context))contexts.set(context,{});
 const counts=contexts.get(context);counts[match[4]]=(counts[match[4]]??0)+1;
 for(const field of ['ncode','vresult','ncount','nfocus','nid','cvalue','cdisplay','nnewindex','nnewid','ccontext','cother','ids','primary','selected','rows','data'])assert(line.includes('|'+field+'='),'retained '+field);
}
assert.strictEqual(contexts.size,8);
for(const counts of contexts.values())assert.deepStrictEqual(counts,expected,'all38 per class/mode/session');
console.log(JSON.stringify({file,calls:lines.length,contexts:contexts.size,uniqueCases:identities.size,groupsPerContext:expected,
 rawSHA256:crypto.createHash('sha256').update(bytes).digest('hex'),
 note:'Non-native completed capture only; same-compiler fixed304 full-byte equality still required. Not a conversion requirement or native parity oracle.'}));
