// Synthetic tooling self-test only; NOT fixed runtime evidence.
const fs=require('fs'),assert=require('assert'),{auditRaw,auditEvidence}=require('./audit-fixed-complete.cjs');
const inventory=fs.readFileSync('tests/fixtures/vfp9-addlistitem-column-numeric/pre-freeze-original95-test-list.log','utf8').split(/\r?\n/).map(l=>l.match(/^\s*Test\s+#\d+:\s+(\S+)$/)?.[1]).filter(Boolean).sort();
const prefix='test_prg_engine_addlistitem_column_numeric';
const blocks=inventory.map((name,i)=>`${i+1}: starting\n Start ${i+1}: ${name}\n`+
 (name===prefix?`${i+1}: AddListItem COLUMN frozen cases: direct=210 public=0\n`:name.startsWith(prefix+'_')?`${i+1}: AddListItem COLUMN frozen cases: direct=0 public=660\n`:'')+
 `${i+1}/95 Test #${i+1}: ${name} ......... Passed 0.01 sec\n`);
const raw=blocks.join('')+'100% tests passed, 0 tests failed out of 95\n';assert(auditRaw(raw,inventory).complete);
const mutations=[['omitted result',raw.replace(blocks[94],'')],['missing summary',raw.replace('100% tests passed, 0 tests failed out of 95','')],
 ['failed test',raw.replace('Passed 0.01','***Failed 0.01')],['timeout',raw.replace('Passed 0.01','***Timeout 700')],
 ['duplicate result',raw+blocks[0]],['wrong start identity',raw.replace('1: '+inventory[0],'1: '+inventory[1])],
 ['missing direct counter',raw.replace(/\d+: AddListItem COLUMN frozen cases: direct=210 public=0\n/,'')],
 ['wrong public counter',raw.replace('public=660','public=659')],['hidden assertion',raw+'1: FAIL: unacceptable\n'],
 ['runtime diagnostic',raw+'runtime error: out of range\n'],['missing ordinal',raw.replace('1/95 Test','0/95 Test')]];
for(const [name,mutated] of mutations){assert(!auditRaw(mutated,inventory).complete,name);console.log(JSON.stringify({syntheticMutation:name,rejected:true}));}
assert(!auditRaw(raw,inventory.slice(1)).complete,'bad inventory');
assert.throws(()=>auditEvidence('build/nonexistent-addlistitem-fixed-evidence'),'missing completed original gate');
console.log(JSON.stringify({syntheticPositive:1,rejectedMutations:12,note:'Future fixed audit-tool self-test only; real fixed execution and caller wiring remain unqualified.'}));
