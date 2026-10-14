// Synthetic audit-tool tests, NOT compiler/runtime/native acceptance evidence.
const assert=require('assert'),fs=require('fs');
const {auditRaw,auditEvidence}=require('./audit-originals-complete.cjs');
const fixture='tests/fixtures/vfp9-addlistitem-column-numeric/';
const inventory=fs.readFileSync(fixture+'pre-freeze-original95-test-list.log','utf8').split(/\r?\n/)
 .map(l=>l.match(/^\s*Test\s+#\d+:\s+(\S+)$/)?.[1]).filter(Boolean).sort();
assert.strictEqual(inventory.length,95);
const prefix='test_prg_engine_addlistitem_column_numeric';
const shards=inventory.filter(n=>n.startsWith(prefix+'_'));
const blocks=inventory.map((name,i)=>{
 const id=i+1,selected=shards.includes(name);
 const fail=selected?`${id}: FAIL: AddListItem COLUMN admission synthetic got 0\n${id}: continuation including a second line\n`:'';
 const count=name===prefix?`${id}: AddListItem COLUMN frozen cases: direct=210 public=0\n`:
  selected?`${id}: AddListItem COLUMN frozen cases: direct=0 public=660\n`:'';
 return `      Start ${id}: ${name}\n${fail}${count}${id}/95 Test #${id}: ${name} ........ ${selected?'***Failed':'Passed'} 0.01 sec\n`;
});
const raw=blocks.join('')+'96% tests passed, 4 tests failed out of 95\n';
const good=auditRaw(raw,inventory);assert(good.complete);assert.strictEqual(good.failures.length,4);
assert(good.failures.every(f=>f.text.endsWith('continuation including a second line')),'whole multiline blocks');
const mutations=[
 ['partial execution',raw.replace(blocks[94],'')],
 ['missing summary',raw.replace('96% tests passed, 4 tests failed out of 95','')],
 ['wrong count',raw.replace('direct=210 public=0','direct=209 public=0')],
 ['missing public counter',raw.replace(/\d+: AddListItem COLUMN frozen cases: direct=0 public=660\n/,'')],
 ['wrong public counter',raw.replace('direct=0 public=660','direct=0 public=659')],
 ['unexpected direct failure',raw.replace(new RegExp('('+prefix+' \\.{3,} )Passed'),'$1***Failed')],
 ['unrelated selected failure',raw.replace('FAIL: AddListItem COLUMN admission','FAIL: AddListItem COLUMN context')],
 ['runtime sanitizer diagnostic',raw+'runtime error: unsafe cast\n'],
 ['duplicate result',raw+blocks[0]],
 ['interrupted result',raw.replace('***Failed 0.01 sec','***Timeout 700 sec')],
 ['ordinal omission',raw.replace('1/95 Test','0/95 Test')],
 ['wrong failure subject',raw.replace('1: '+inventory[0], '1: '+shards[0])]
];
for(const [name,mutated] of mutations){assert(!auditRaw(mutated,inventory).complete,name);console.log(JSON.stringify({syntheticMutation:name,rejected:true}));}
assert(!auditRaw(raw,inventory.slice(1)).complete,'incomplete inventory');
assert.notStrictEqual(auditRaw(raw.replace('continuation including a second line','changed full continuation'),inventory).failureSHA256,good.failureSHA256,'full failure comparison');
assert.throws(()=>auditEvidence('build/nonexistent-addlistitem-original-evidence'),'no partial/missing gate accepted');
console.log(JSON.stringify({syntheticPositive:1,rejectedMutations:mutations.length+1,wholeMultilineHashSensitive:true,
 note:'Audit-tool self-test only. Neither actual compiler originals nor caller wiring qualified.'}));
