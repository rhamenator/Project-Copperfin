// Pre-code native capture audit only; no Copperfin/helper-produced oracle.
const fs=require('fs'),path=require('path');
const root=process.argv[2];
let failures=0,total=0,rejected=0,appended=0,updated=0;
for(const [name,planned] of [['native',440],['boundary',256],['configuration',96],['binary-neighbor',70],['no-touch-neighbor',70]]) {
 const raw=fs.readFileSync(path.join(root,name+'-first.out'),'utf8');
 const lines=raw.split(/\r?\n/).filter(Boolean);
 const calls=lines.filter(l=>l.startsWith('ListBox|')||l.startsWith('ComboBox|'));
 if(calls.length!==planned||lines[lines.length-1]!=='DONE|'+planned) ++failures;
 for(const line of calls) {
  ++total;
  const [prefix,states]=line.split('|before=');
  const [before,after]=states.split('|after=');
  const count=s=>Number(s.match(/^count=(\d+)/)[1]);
  const fields=prefix.split('|');
  const shape=Number(fields[1].split('=')[1]);
  const error=Number(prefix.match(/\|error=(\d+)/)[1]);
  const expectedCount=name==='configuration'?3:shape===0?0:shape===3?1:3;
  if(count(before)!==expectedCount||fields[3]!=='invoke') ++failures;
  let outcome;
  if(error!==0) {
   ++rejected;outcome='rejected';
   if(error!==11||before!==after) ++failures;
  } else {
   const delta=count(after)-count(before);
   outcome=delta===1?'append':delta===0?'existing-ID-update':'unexpected-count';
   if(delta===1) ++appended;else if(delta===0) ++updated;else ++failures;
   const rows=s=>s.split(';').slice(1).map(r=>r.replace(/^row=/,'').split(','));
   const oldRows=rows(before),newRows=rows(after);
   const candidate=newRows.filter(r=>r[1]==='candidate');
   if(candidate.length!==1||Number(candidate[0][0])<1||Number(candidate[0][0])>32767) ++failures;
   for(const oldRow of oldRows) {
    const next=newRows.find(r=>r[0]===oldRow[0]);
    if(!next||next.slice(2).join(',')!==oldRow.slice(2).join(',')||
      (next[1]!=='candidate'&&next[1]!==oldRow[1])) ++failures;
   }
  }
  process.stdout.write(JSON.stringify({pair:name,class:fields[0],shape,source:fields[2],phase:fields[3],error,outcome,beforeCount:count(before),afterCount:count(after)})+'\n');
 }
}
process.stdout.write(JSON.stringify({consumerCalls:total,fullyPreservedRejections:rejected,appendSuccesses:appended,existingIDUpdates:updated,failures,note:'Native successful focus/text/top/return are captured but excluded; existing-ID updates are separate unadmitted7173. No helper output used.'})+'\n');
process.exitCode=failures?1:0;
