* #5611/#6776 DEFINE BAR numeric identifier recovery; no popup activation.
LOCAL i, lcOperand, nCode, lAfter, loError, nId
DIMENSION aOperand[32]
aOperand[1]='-1'
aOperand[2]='-0.1'
aOperand[3]='0'
aOperand[4]='0.5'
aOperand[5]='0.9'
aOperand[6]='1'
aOperand[7]='1.1'
aOperand[8]='1.5'
aOperand[9]='1.9'
aOperand[10]='2'
aOperand[11]='32767'
aOperand[12]='32768'
aOperand[13]='65535'
aOperand[14]='65536'
aOperand[15]='2147483647'
aOperand[16]='2147483648'
aOperand[17]='4294967295'
aOperand[18]='4294967296'
aOperand[19]='4294967297'
aOperand[20]='-4294967295'
aOperand[21]='1E20'
aOperand[22]='1E300'
aOperand[23]='-1E300'
aOperand[24]='.T.'
aOperand[25]='.F.'
aOperand[26]='.NULL.'
aOperand[27]="'1'"
aOperand[28]='2147483647.9'
aOperand[29]='4294967295.9'
aOperand[30]='4294967296.1'
aOperand[31]='-4294967294'
aOperand[32]='-4294967296'
=STRTOFILE('VERSION|'+VERSION()+CHR(10),'probe.out',0)
=STRTOFILE('CONTROL|INT|'+TRANSFORM(INT(1.9))+'|'+TRANSFORM(SET('DATASESSION'))+CHR(10),'probe.out',1)
FOR i=1 TO ALEN(aOperand)
 lcOperand=aOperand[i]
 =STRTOFILE('BEGIN|'+lcOperand+CHR(10),'probe.out',1)
 DEFINE POPUP numguard RELATIVE
 DEFINE BAR 1 OF numguard PROMPT 'one'
 DEFINE BAR 2 OF numguard PROMPT 'two'
 DEFINE BAR 2147483647 OF numguard PROMPT 'cap'
 nCode=0
 lAfter=.F.
 TRY
  =EXECSCRIPT('DEFINE BAR '+lcOperand+" OF numguard PROMPT 'new'")
  lAfter=.T.
 CATCH TO loError
  nCode=loError.ErrorNo
 ENDTRY
 nId=0
 IF CNTBAR('numguard')=4
  nId=GETBAR('numguard',4)
 ENDIF
 =STRTOFILE('BAR|'+lcOperand+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(CNTBAR('numguard'))+'|'+PRMBAR('numguard',1)+'|'+PRMBAR('numguard',2)+'|'+PRMBAR('numguard',2147483647)+'|'+TRANSFORM(nId)+'|'+TRANSFORM(SET('DATASESSION'))+CHR(10),'probe.out',1)
 RELEASE POPUP numguard
ENDFOR
=STRTOFILE('CLEANUP|'+TRANSFORM(SET('DATASESSION'))+CHR(10),'probe.out',1)
RETURN
