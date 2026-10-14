// Fail-closed five-subject isolation/commands audit after original compilation.
const fs=require('fs'),path=require('path');
const [file,requireCommands='yes']=process.argv.slice(2);
const tests=JSON.parse(fs.readFileSync(file,'utf8')).tests;
const prefix='test_prg_engine_addlistitem_column_numeric',wanted=[prefix,...['copperfin','vfp9'].flatMap(m=>[1,2].map(s=>prefix+'_'+m+'_session'+s))],failures=[];
const selected=tests.filter(t=>wanted.includes(t.name));
if(selected.length!==5||new Set(selected.map(t=>t.name)).size!==5)failures.push('five unique subjects');
for(const t of selected){
 const p=new Map(t.properties.map(x=>[x.name,x.value])),labels=p.get('LABELS')??[];
 for(const label of ['audit=complete','child-processes=none','environment=scoped-process','filesystem=test-owned-unique','network=none','platform=portable','resources=none','samples=none','schedule=serial'])if(!labels.includes('copperfin-isolation:'+label))failures.push(t.name+' '+label);
 if(p.get('RUN_SERIAL')!==true)failures.push(t.name+' not serial');
 const suffix=t.name.slice(prefix.length),parts=suffix?suffix.match(/^_(copperfin|vfp9)_session([12])$/):null;
 const args=parts?[parts[1].toUpperCase(),parts[2]]:[];
 if(requireCommands==='yes'&&(!t.command||path.basename(t.command[0])!==prefix||JSON.stringify(t.command.slice(1))!==JSON.stringify(args)))failures.push(t.name+' safe command');
 console.log(JSON.stringify({name:t.name,serial:p.get('RUN_SERIAL'),command:t.command??null,staticExpectedArguments:args,labels}));
}
console.log(JSON.stringify({subjects:selected.length,requireCommands,failures,note:requireCommands==='yes'?'Built command/isolation audit, not execution acceptance.':'Pre-build labels/serial only; commands absent while executable is unbuilt, not a completed command qualification.'}));
process.exitCode=failures.length?1:0;
