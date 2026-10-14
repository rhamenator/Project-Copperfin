// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Independent source enumeration; runtime counters must separately corroborate.
const fs=require('fs'),assert=require('assert'),crypto=require('crypto');
const p='tests/fixtures/vfp9-addlistitem-new-column-numeric/';
const s=fs.readFileSync('tests/test_prg_engine_addlistitem_new_column_numeric.cpp','utf8');
const old=fs.readFileSync(p+'counter-v1-interrupted/test-source.cpp','utf8');
assert.equal(s,old.replace('argc==4?112:624','argc==4?112:432'),'ONLY counter constant, no operand/expected behavior/state change');
const block=name=>s.split(' '+name+'{')[1].split('\n};')[0];
const rows=[...block('numbers').matchAll(/\{"[^"\n]+"[^\n]*?,(true|false)\}/g)],exacts=[...block('exacts').matchAll(/\{"[^"\n]+"[^\n]*?,(true|false)\}/g)];
assert.equal(rows.length,65);assert.equal(exacts.length,18);
const safe=rows.filter(m=>m[1]==='true').length+exacts.filter(m=>m[1]==='true').length;assert.equal(safe,56);
const guard=s.split('const std::vector<NumberCase> guards')[1].split('const std::vector<std::string> configs')[0];
const guards=[...guard.matchAll(/\{"/g)].length;assert.equal(guards,4);
const configs=s.split('const std::vector<std::string> configs{')[1].split('};')[0];
const configurations=[...configs.matchAll(/"(?:[^"\\]|\\.)*"/g)].length;assert.equal(configurations,6);
assert(s.includes('for(const std::string cls:{"ListBox","ComboBox"})for(const bool sparse:{false,true})'));
assert(s.includes('for(const int columns:{0,9})for(const auto& config:configs)for(const auto& c:guards)'));
const locales=s.split('for(const auto& locale:')[1].split('set_env_value("COPPERFIN_LOCALE",locale[0],true)')[0];
assert.equal([...locales.matchAll(/\{"/g)].length,4);
const core=2*2*safe,configuration=2*2*2*configurations*guards,locale=4*2*2;
assert.equal(core,224);assert.equal(configuration,192);assert.equal(locale,16);assert.equal(core+configuration+locale,432);
assert.equal(2*(rows.length+exacts.length+13),192);assert.equal(2*2*(rows.length+exacts.length-safe)+2*2,112);
assert(s.includes('direct_count==192&&public_count==0'));assert(s.includes('argc==4?112:432'));
console.log(JSON.stringify({counterConstantOnlyCorrection:true,numeric:65,exact:18,safe,guards,configurations,plannedDirect:192,core,configuration,locale,plannedPublicPerShard:432,plannedFixedOnlyHostilePerShard:112,sourceSHA256:crypto.createHash('sha256').update(s).digest('hex'),note:'SOURCE enumeration ONLY, not executed original/fixed qualification. V1 partial failures retained and BOTH full compiler suites must be rerun from the v2 freeze.'}));
