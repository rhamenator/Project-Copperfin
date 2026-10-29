// Copyright © 2026 Richard M. Hamilton.
// SPDX-License-Identifier: GPL-3.0-only
// Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
// Native feedback selects inputs; no candidate formatter supplies an answer.
const fs=require('node:fs'),path=require('node:path'),zlib=require('node:zlib');
const a=require('node:assert/strict');
const prior=JSON.parse(fs.readFileSync(path.join(__dirname,'rounding-ladder-cases.json')));
const observed=JSON.parse(zlib.gunzipSync(fs.readFileSync(path.join(__dirname,'rounding-ladder-captures/audit.json.gz'))));
a.equal(observed.firstSha256,'e7cbfd0a45549b69bbfcebc2d924dc20bb7f895e1405ad95136f4359b4eb134f');
a.equal(observed.repeatSha256,observed.firstSha256);a.equal(observed.rawByteEqual,true);
const groups=[];
for(const center of prior.centers.slice(0,5))for(const negative of [false,true])
    groups.push({id:groups.length+1,...center,center:center.id,negative});
for(let i=0;i<groups.length;i++)groups[i].id=i+1;
const inv={requirement:prior.requirement,parentNativeSha256:observed.firstSha256,groups,
    lowOffset:'-4503599627370496',highOffset:'0',width:40,scale:18,maxQueriesPerGroup:54,
    limitation:'Native feedback bisection of five near-half centers, both signs, exact finite payloads. Initial endpoints need different observed ON text. Each midpoint is retained in both FIXED modes. Reader independently reconstructs every query and requires adjacent final payloads with different ON text. This proves a local witnessed boundary only, not monotonicity, uniqueness, a tolerance or a conversion algorithm. No Copperfin/model/expected-output input, production changes, VM or new FLL.'};
const helpers=fs.readFileSync(path.join(__dirname,'rounding-ladder.prg'),'utf8').split('\nFUNCTION Unhex')[1];a(helpers);
const decl=groups.map(g=>`baseHex[${g.id}]='${g.bits}'\nnegative[${g.id}]=${g.negative?'.T.':'.F.'}\nsetting[${g.id}]=${g.setting}`).join('\n');
const source=`* Copyright © 2026 Richard M. Hamilton.
* SPDX-License-Identifier: GPL-3.0-only
* Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.
* RQ-CF-PRG-NUMERIC-DEFAULT-TRANSFORM-001: native-feedback local boundary recovery.
LOCAL ARRAY baseHex[10],negative[10],setting[10]
${decl}
LOCAL outputPath,libraryPath,groupNo,queryNo,lowOffset,highOffset,midOffset,lowText,highText,midText,lowHex,highHex,midHex,total,decimalCount
outputPath='Z:\\home\\rich\\.codex\\worktrees\\numeric-literal-display-5611\\Project-Copperfin\\build\\default-transform-rounding-edge\\results.txt'
libraryPath='Z:\\home\\rich\\dev\\Project-Copperfin\\build\\pr7203-completed\\native-evidence\\diagnostic\\cause-probe.fll'
=STRTOFILE('VERSION|'+VERSION()+CHR(13)+CHR(10),outputPath,0)
SET LIBRARY TO (libraryPath) ADDITIVE
total=0
FOR groupNo=1 TO 10
 decimalCount=setting[groupNo]
 SET DECIMALS TO decimalCount
 lowOffset=-4503599627370496
 highOffset=0
 queryNo=1
 highHex=OffsetHex(baseHex[groupNo],negative[groupNo],highOffset)
 highText=Observe(highHex,groupNo,queryNo,setting[groupNo],outputPath)
 queryNo=2
 lowHex=OffsetHex(baseHex[groupNo],negative[groupNo],lowOffset)
 lowText=Observe(lowHex,groupNo,queryNo,setting[groupNo],outputPath)
 IF lowText==highText
  =STRTOFILE('UNBRACKETED|'+ALLTRIM(STR(groupNo,4,0))+CHR(13)+CHR(10),outputPath,1)
 ELSE
  DO WHILE highOffset-lowOffset>1 AND queryNo<54
   midOffset=INT((lowOffset+highOffset)/2)
   midHex=OffsetHex(baseHex[groupNo],negative[groupNo],midOffset)
   queryNo=queryNo+1
   midText=Observe(midHex,groupNo,queryNo,setting[groupNo],outputPath)
   IF midText==highText
    highOffset=midOffset
    highHex=midHex
   ELSE
    lowOffset=midOffset
    lowHex=midHex
    lowText=midText
   ENDIF
  ENDDO
  =STRTOFILE('EDGE|'+ALLTRIM(STR(groupNo,4,0))+'|'+lowHex+'|'+highHex+'|'+lowText+'|'+highText+CHR(13)+CHR(10),outputPath,1)
 ENDIF
 total=total+queryNo*2
ENDFOR
=STRTOFILE('DONE|'+ALLTRIM(STR(total,6,0))+CHR(13)+CHR(10),outputPath,1)
SET LIBRARY TO

FUNCTION Observe
 LPARAMETERS inputHex,groupNo,queryNo,decimalCount,outputPath
 LOCAL payload,fixedNo,fixedMode,line,result,caught
 payload=CTOBIN(Unhex(inputHex),'BS')
 result=''
 FOR fixedNo=1 TO 2
  IF fixedNo=1
   SET FIXED OFF
   fixedMode='OFF'
  ELSE
   SET FIXED ON
   fixedMode='ON'
  ENDIF
  TRY
   result=HexBytes(TRANSFORM(CFNDEC(CFNWIDTH(payload,40),18)))
   line=ALLTRIM(STR(groupNo,4,0))+'|'+ALLTRIM(STR(queryNo,4,0))+'|'+fixedMode+'|'+inputHex+'|'+ALLTRIM(STR(decimalCount,4,0))+'|'+CFNMETA(CFNDEC(CFNWIDTH(payload,40),18))+'|'+HexBytes(BINTOC(CFNDEC(CFNWIDTH(payload,40),18),'BRS'))+'|'+result+'|'+HexBytes(STR(CFNDEC(CFNWIDTH(payload,40),18),40,decimalCount))+'|'+VARTYPE(TRANSFORM(CFNDEC(CFNWIDTH(payload,40),18)))
  CATCH TO caught
   line='ERROR|'+ALLTRIM(STR(groupNo,4,0))+'|'+ALLTRIM(STR(queryNo,4,0))+'|'+ALLTRIM(STR(caught.ErrorNo,6,0))+'|'+HexBytes(caught.Message)
  ENDTRY
  =STRTOFILE(line+CHR(13)+CHR(10),outputPath,1)
 ENDFOR
 RETURN result

FUNCTION OffsetHex
 LPARAMETERS base,negative,offset
 LOCAL high,low,carry
 high=HexNumber(LEFT(base,8))
 low=HexNumber(RIGHT(base,8))+offset
 carry=FLOOR(low/4294967296)
 high=high+carry
 low=low-carry*4294967296
 IF negative
  high=high+2147483648
 ENDIF
 RETURN Hex8(high)+Hex8(low)

FUNCTION HexNumber
 LPARAMETERS value
 LOCAL result,position
 result=0
 FOR position=1 TO LEN(value)
  result=result*16+AT(SUBSTR(value,position,1),'0123456789ABCDEF')-1
 ENDFOR
 RETURN result

FUNCTION Hex8
 LPARAMETERS value
 LOCAL result,position,digit
 result=''
 FOR position=1 TO 8
  digit=MOD(value,16)
  result=SUBSTR('0123456789ABCDEF',digit+1,1)+result
  value=INT(value/16)
 ENDFOR
 RETURN result

FUNCTION Unhex${helpers}`;
if(process.argv[2]==='source')process.stdout.write(source);
else if(process.argv[2]==='cases')process.stdout.write(JSON.stringify(inv,null,2)+'\n');
else console.log(JSON.stringify({groups:groups.length,maxQueriesPerGroup:54,maxRecords:1080}));
