// Capture-only consistency and observations, NOT a conversion helper/oracle.
const fs=require('fs'),crypto=require('crypto');
const [first,repeat]=process.argv.slice(2),raw=fs.readFileSync(first),again=fs.readFileSync(repeat);
const lines=raw.toString().trim().split(/\r?\n/);let failures=[];
if(!raw.equals(again))failures.push('independent repeats differ');
if(lines.length!==974||!lines[0].includes('09.00.0000.7423')||lines.at(-1)!=='DONE|972')failures.push('complete version/counter');
const groups={},errors={};let invoked=0,rejectedUnchanged=0,operandChanged=0;
for(const line of lines.slice(1,-1)){
 const a=line.split('|'),get=k=>a.find(f=>f.startsWith(k+'='))?.slice(k.length+1);
 const cls=a[0],source=a[4],phase=a[5],before=get('before'),after=get('after');
 const key=a.slice(0,4).join('|');groups[key]=(groups[key]||0)+1;
 const error=Number(get('error'));errors[error]=(errors[error]||0)+1;
 if(phase==='invoke')++invoked;else failures.push('not invoked '+key+' '+source+' '+phase);
 if(!before||!after||!/^count=\d+,columns=\d+,/.test(before)||!/^count=\d+,columns=\d+,/.test(after))failures.push('snapshot framing '+key+' '+source);
 const beforeCount=Number(before?.match(/^count=(\d+)/)?.[1]),afterCount=Number(after?.match(/^count=(\d+)/)?.[1]);
 if(beforeCount!==(get('shape')==='0'?0:3))failures.push('setup count '+key+' '+source);
 const operandSame=get('preType')===get('postType')&&get('preBRS')===get('postBRS');
 if(!operandSame)++operandChanged;
 if(!/^[0-9A-F]{16}$/.test(get('preBRS'))||!/^[0-9A-F]{16}$/.test(get('postBRS')))failures.push('operand BRS framing '+key+' '+source);
 if(error!==0&&before===after)++rejectedUnchanged;
 const candidateVisible=[...after.matchAll(/;row=(\d+),[^;]*?(?=;row=|$)/g)].flatMap(m=>[...m[0].matchAll(/,cell(\d+)=candidate(?:,|;|$)/g)].map(c=>({id:Number(m[1]),column:Number(c[1])})));
 console.log(JSON.stringify({cls,columns:+get('columns'),shape:+get('shape'),target:+get('target'),source,phase,error,
  beforeCount,afterCount,beforeAfterSame:before===after,candidateVisible,preType:get('preType'),postType:get('postType'),preBRS:get('preBRS'),postBRS:get('postBRS'),operandSame,returned:get('return')}));
}
if(Object.keys(groups).length!==18||Object.values(groups).some(n=>n!==54))failures.push('18 contexts x54 operands');
console.log(JSON.stringify({rows:lines.length-2,groups,errors,invoked,rejectedUnchanged,operandChanged,failures,
 rawSHA256:crypto.createHash('sha256').update(raw).digest('hex'),note:'Paired installed native observations only. No default/exact extension or native/global capacity, hidden-storage, shared-success-state correctness or implementation acceptance requirement inferred here.'}));
process.exitCode=failures.length?1:0;
