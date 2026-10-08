* RQ-CF-PRG-UNLOCK-RECORD-NUMERIC-001: isolated in-process record locks.
LOCAL loError, nCode, lAfter, nCase, nQualified, x, lSetup, cTable
? 'VERSION|'+VERSION()
SET MULTILOCKS ON
cTable='unlock-probe-owned.dbf'
CREATE TABLE (cTable) (label C(8))
USE
USE (cTable) SHARED ALIAS unlockguard
INSERT INTO unlockguard VALUES ('one')
INSERT INTO unlockguard VALUES ('two')
INSERT INTO unlockguard VALUES ('three')
SELECT 0
CREATE CURSOR observer (label C(8))
INSERT INTO observer VALUES ('sentinel')
DIMENSION aCases[36]
aCases[1]=[-1.9]
aCases[2]=[-0.9]
aCases[3]=[0]
aCases[4]=[0.5]
aCases[5]=[0.9]
aCases[6]=[1]
aCases[7]=[1.5]
aCases[8]=[1.9]
aCases[9]=[2.5]
aCases[10]=[3.9]
aCases[11]=[4]
aCases[12]=[2147483647]
aCases[13]=[2147483648]
aCases[14]=[4294967295]
aCases[15]=[4294967296]
aCases[16]=[4294967297]
aCases[17]=[4294967298]
aCases[18]=[-2147483648]
aCases[19]=[-2147483649]
aCases[20]=[-4294967295]
aCases[21]=[-4294967296]
aCases[22]=[-4294967297]
aCases[23]=[9007199254740992]
aCases[24]=[9223372036854774784]
aCases[25]=[9223372036854775808]
aCases[26]=[-9223372036854775808]
aCases[27]=[1E20]
aCases[28]=[-1E20]
aCases[29]=[1E300]
aCases[30]=[-1E300]
aCases[31]=[1E300*1E300]
aCases[32]=[-1E300*1E300]
aCases[33]=[$1.5]
aCases[34]=['1.5']
aCases[35]=[.T.]
aCases[36]=[.NULL.]
FOR nQualified=0 TO 1
 FOR nCase=1 TO ALEN(aCases)
  SELECT unlockguard
  UNLOCK
  GO 1
  lSetup=RLOCK()
  GO 2
  lSetup=RLOCK() AND lSetup
  GO 3
  lSetup=RLOCK() AND lSetup
  GO 2
  IF nQualified=1
   SELECT observer
  ENDIF
  lSetup=lSetup AND ISRLOCKED(1,'unlockguard') AND ISRLOCKED(2,'unlockguard') AND ISRLOCKED(3,'unlockguard')
  x=EVALUATE(aCases[nCase])
  nCode=0
  lAfter=.F.
  TRY
   IF nQualified=1
    UNLOCK RECORD (x) IN unlockguard
   ELSE
    UNLOCK RECORD (x)
   ENDIF
   lAfter=.T.
  CATCH TO loError
   nCode=loError.ErrorNo
  ENDTRY
  ? TRANSFORM(nQualified)+'|'+aCases[nCase]+'|'+TRANSFORM(nCode)+'|'+TRANSFORM(lAfter)+'|'+TRANSFORM(lSetup)+'|'+TRANSFORM(ISRLOCKED(1,'unlockguard'))+'|'+TRANSFORM(ISRLOCKED(2,'unlockguard'))+'|'+TRANSFORM(ISRLOCKED(3,'unlockguard'))+'|'+TRANSFORM(RECNO('unlockguard'))+'|'+UPPER(ALIAS())+'|'+TRANSFORM(SET('DATASESSION'))
 ENDFOR
ENDFOR
SELECT unlockguard
UNLOCK
USE IN unlockguard
USE IN observer
ERASE (cTable)
SET MULTILOCKS OFF
? 'CLEANUP|'+TRANSFORM(USED('unlockguard'))+'|'+TRANSFORM(USED('observer'))+'|'+SET('MULTILOCKS')+'|'+TRANSFORM(SET('DATASESSION'))
RETURN
