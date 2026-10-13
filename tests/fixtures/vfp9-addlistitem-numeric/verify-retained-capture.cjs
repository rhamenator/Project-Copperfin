// Retained pre-code observation integrity; not Copperfin acceptance assertions.
const fs=require('fs'),path=require('path'),crypto=require('crypto');
const root=process.argv[2];
const pairs=[['native',440],['boundary',256],['configuration',96],['precision',6],
 ['precision-bits',6],['binary-neighbor',70],['no-touch-neighbor',70],
 ['context',24],['concat-context',30],['seed-context',30],['boundary-postbits',256]];
let failures=0;
const read=n=>fs.readFileSync(path.join(root,n));
const requireCheck=(ok,name)=>{console.log(JSON.stringify({check:name,passed:ok}));if(!ok)++failures;};
for(const [name,calls] of pairs){
 const first=read(name+'-first.out'),repeat=read(name+'-repeat.out');
 requireCheck(first.equals(repeat),name+': full raw pair');
 for(const arm of ['first','repeat']){
  requireCheck(read(name+'-'+arm+'.status').toString().trim()==='0',name+': '+arm+' runner0');
  requireCheck(read(name+'-'+arm+'.stderr').length===0,name+': '+arm+' stderr empty');
 }
 requireCheck(read(name+'-repeat.cmp.status').toString().trim()==='0',name+': cmp0');
 requireCheck(read(name+'-repeat.cmp').length===0,name+': cmp output empty');
 const lines=first.toString().split(/\r?\n/).filter(Boolean);
 requireCheck(lines.length===calls+2&&lines[0].includes('09.00.0000.7423')&&
   lines.at(-1)==='DONE|'+calls,name+': version/count/DONE');
 console.log(JSON.stringify({pair:name,calls,sha256:crypto.createHash('sha256').update(first).digest('hex')}));
}
const original=read('boundary-first.out').toString(),post=read('boundary-postbits-first.out').toString();
requireCheck(original===post.replace(/,postcallBRS=[0-9A-F]+/g,''),'post-call BRS stripped raw exactly original');
const near=original.split(/\r?\n/).filter(l=>/1-2\^\(-5[23]\)/.test(l));
requireCheck(near.length===16&&near.every(l=>/\|error=11\|/.test(l)),'all sixteen original arithmetic near-one calls reject11');
requireCheck(read('audit-native-capture.jsonl').equals(read('audit-native-capture-recheck.jsonl')),'932-call audit exact recheck');
console.log(JSON.stringify({pairs:pairs.length,failures,note:'Native evidence integrity only; no implementation acceptance or root-cause claim.'}));
process.exitCode=failures?1:0;
