// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Five source-audited conservative SERIAL subjects; no hostile registration.
const fs=require('fs'),path=require('path'),assert=require('assert');
const [file,commands='yes']=process.argv.slice(2),tests=JSON.parse(fs.readFileSync(file,'utf8')).tests;
const prefix='test_prg_engine_addlistitem_new_column_numeric';
const names=[prefix,...['copperfin','vfp9'].flatMap(mode=>[1,2].map(session=>prefix+'_'+mode+'_session'+session))];
const selected=tests.filter(t=>names.includes(t.name));assert.equal(selected.length,5);assert.equal(new Set(selected.map(t=>t.name)).size,5);
for(const t of selected){
 const p=new Map(t.properties.map(x=>[x.name,x.value])),labels=p.get('LABELS')??[];assert.equal(p.get('RUN_SERIAL'),true,t.name);
 for(const label of ['audit=complete','child-processes=none','environment=scoped-process','filesystem=test-owned-unique','network=none','platform=portable','resources=none','samples=none','schedule=serial'])assert(labels.includes('copperfin-isolation:'+label),t.name+' '+label);
 const parts=t.name.slice(prefix.length).match(/^_(copperfin|vfp9)_session([12])$/),args=parts?[parts[1].toUpperCase(),parts[2]]:[];
 if(commands==='yes'){assert(t.command,t.name);assert.equal(path.basename(t.command[0]),prefix);assert.deepEqual(t.command.slice(1),args);}
 console.log(JSON.stringify({name:t.name,serial:true,command:t.command??null,expectedArguments:args,labels}));
}
console.log(JSON.stringify({subjects:5,commands,complete:true,note:commands==='yes'?'Built commands/isolation only, not execution acceptance.':'Pre-build labels/serial only; unbuilt commands NOT qualified.'}));
