// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: stimuli only, no expected new strings.
const fs=require('node:fs'),path=require('node:path'),z=require('node:zlib');
const a=require('node:assert/strict'),crypto=require('node:crypto');
const dir=path.join(__dirname,'round-inline-captures1');
const bytes=n=>z.gunzipSync(fs.readFileSync(path.join(dir,n+'.gz')));
const sha=b=>crypto.createHash('sha256').update(b).digest('hex');
const parentHash='254b46dadbb71fa51b10e8439d51785b2c2aa3eb871c8c948fd3b8bdd3da4873';
const sums=new Map(fs.readFileSync(path.join(dir,'SHA256SUMS'),'utf8').trim().split('\n').map(l=>l.split('  ').reverse()));
for(const n of ['audit.json','first-results.txt','repeat-results.txt'])a.equal(sha(fs.readFileSync(path.join(dir,n+'.gz'))),sums.get(n+'.gz'));
const parent=JSON.parse(bytes('audit.json'));
for(const k of ['documentedFieldsRepeatEqual','inlineResultAndStoredParentControls','exactRuntimeIdentity','rawByteEqual'])a.equal(parent[k],true);
for(const run of ['first','repeat']){a.equal(sha(bytes(run+'-results.txt')),parentHash);a.equal(parent[run+'Sha256'],parentHash);}
const inputs=[];
for(const parentIndex of [193,199,205,211,217,223]){
 const p=parent.observations.find(r=>r.index===parentIndex&&r.fixed==='ON');a(p);a([16,17,18].includes(p.setting));
 a.equal(p.inlineMeta.type,'4E');a.equal(p.inlineMeta.width,'0028');a.equal(parseInt(p.inlineMeta.decimals,16),p.setting);a.equal(p.inlineMeta.real,p.inlineBits);
 for(const kind of ['original','round'])for(const scale of [...new Set([18,p.setting])])inputs.push({index:inputs.length+1,parentIndex,originalBits:p.bits,bits:kind==='original'?p.bits:p.inlineBits,setting:p.setting,scale,kind});
}
a.equal(inputs.length,20);
const previous=JSON.parse(fs.readFileSync(path.join(__dirname,'str-middle-cases.json')));
const inv={requirement:previous.requirement,inputCount:20,recordsPerRun:40,fieldsPerRecord:42,parentNativeSha256:parentHash,inputs,
 inputRoute:'Inline CFNDEC(CFNWIDTH(targetPayload,40),inputScale[index]) at every target consumer; no stored numeric argument/result.',
 parentRoute:'Original N40/18 and INLINE ROUND(original,appliedD), before/after target observations.',
 width:40,controlCount:17,runtimeEnvironment:'WINEDLLOVERRIDES=msvcr71=n',observerSha256:'0b1fcd2ba81b95631cb52e6ecd0d4afa721b450a967402f504d1852a49e9e9ac',
 limitation:'D16/17 same-payload scale intervention plus D18 unchanged-scale controls, both signs/FIXED. Reconstructed ROUND payload is a new route: its strings are NOT assumed equal to the parent ROUND expression. No internal-call, universal conversion or formatter proof.'};
for(const k of ['modulePath','moduleSha256','moduleFileSize','modulePe'])inv[k]=previous[k];
const original='CFNDEC(CFNWIDTH(originalPayload,40),18)',round=`ROUND(${original},inputD[index])`,target='CFNDEC(CFNWIDTH(targetPayload,40),inputScale[index])';
const values=(prefix,expr,type=false,binding=false)=>[
 ...(type?[[prefix+'Type',`VARTYPE(${expr})`]]:[]),[prefix+'Meta',`CFNMETA(${expr})`],[prefix+'Bits',`HexBytes(BINTOC(${expr},'BRS'))`],
 [prefix+'Display',`HexBytes(TRANSFORM(${expr}))`],[prefix+'Str',`HexBytes(STR(${expr},40,inputD[index]))`],...(binding?[[prefix+'Binding',`CFNFCVT(${expr},17)`]]:[])];
const observations=[...values('original',original,false,true),...values('round',round),
 ['decType',"VARTYPE(SET('DECIMALS'))"],['decText',"HexBytes(TRANSFORM(SET('DECIMALS')))"],['fixedType',"VARTYPE(SET('FIXED'))"],['fixedText',"HexBytes(TRANSFORM(SET('FIXED')))"],
 ...values('target',target,true,true),...values('originalAfter',original,false,true),...values('roundAfter',round),...values('targetAfter',target,true,true)];
a.equal(observations.length,34);
let source=`* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: bounded scale interventions, no expected strings.
LOCAL ARRAY inputOriginal[20],inputHex[20],inputD[20],inputScale[20],inputParent[20],inputKind[20]
${inputs.map(r=>`inputOriginal[${r.index}]='${r.originalBits}'\ninputHex[${r.index}]='${r.bits}'\ninputD[${r.index}]=${r.setting}\ninputScale[${r.index}]=${r.scale}\ninputParent[${r.index}]=${r.parentIndex}\ninputKind[${r.index}]='${r.kind}'`).join('\n')}
LOCAL outputPath,libraryPath,crtPath,index,fixedNo,fixedMode,originalPayload,targetPayload,decimalCount,caught,line,total,failed
LOCAL ${observations.map(([n])=>n).join(',')}
outputPath='Z:\\home\\rich\\.codex\\worktrees\\numeric-literal-display-5611\\Project-Copperfin\\build\\default-transform-scale-intervention\\results.txt'
libraryPath='Z:\\home\\rich\\dev\\Project-Copperfin\\build\\pr7203-completed\\native-evidence\\diagnostic\\cause-probe.fll'
crtPath='Z:\\home\\rich\\.codex\\worktrees\\numeric-literal-display-5611\\Project-Copperfin\\build\\default-transform-gcvt-captures1\\gcvt-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
TRY
 SET LIBRARY TO (libraryPath) ADDITIVE
 SET LIBRARY TO (crtPath) ADDITIVE
CATCH TO caught
 =STRTOFILE('ERROR-LOAD|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)+CHR(13)+CHR(10),outputPath,1)
 SET LIBRARY TO
 RETURN
ENDTRY
total=0
FOR index=1 TO ALEN(inputHex)
 originalPayload=CTOBIN(Unhex(inputOriginal[index]),'BS')
 targetPayload=CTOBIN(Unhex(inputHex[index]),'BS')
 decimalCount=inputD[index]
 SET DECIMALS TO decimalCount
 FOR fixedNo=1 TO 2
  IF fixedNo=1
   SET FIXED OFF
   fixedMode='OFF'
  ELSE
   SET FIXED ON
   fixedMode='ON'
  ENDIF
  failed=.F.
  TRY
`;
for(const[n,e]of observations){source+=`   ${n}=${e}\n`;if(n.endsWith('Binding'))source+=`   IF LEFT(${n},6)=='ERROR:'
    =STRTOFILE('ERROR-${n}|'+ALLTRIM(STR(index,6,0))+'|'+HexBytes(${n})+CHR(13)+CHR(10),outputPath,1)
    SET LIBRARY TO
    RETURN
   ENDIF
`;}
source+=`   line=fixedMode+'|'+ALLTRIM(STR(index,6,0))+'|'+ALLTRIM(STR(inputParent[index],6,0))+'|'+inputOriginal[index]+'|'+inputHex[index]+'|'+ALLTRIM(STR(inputD[index],6,0))+'|'+ALLTRIM(STR(inputScale[index],6,0))+'|'+inputKind[index]${observations.map(([n])=>"+'|'+"+n).join('')}
  CATCH TO caught
   failed=.T.
   line='ERROR-'+fixedMode+'|'+ALLTRIM(STR(index,6,0))+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
  ENDTRY
  =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
  IF failed
   SET LIBRARY TO
   RETURN
  ENDIF
  total=total+1
 ENDFOR
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(total,6,0))+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO

`;
const helpers=fs.readFileSync(path.join(__dirname,'round-inline.prg'),'utf8').split('\nFUNCTION Unhex\n');a.equal(helpers.length,2);source+='FUNCTION Unhex\n'+helpers[1];
if(process.argv[2]==='source')process.stdout.write(source);
else if(process.argv[2]==='cases')process.stdout.write(JSON.stringify(inv,null,2)+'\n');
else console.log(JSON.stringify({sourceOnly:true,inputs:20,records:40,fields:42,nativeInvoked:false}));
