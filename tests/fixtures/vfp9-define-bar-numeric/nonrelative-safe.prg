* #5611/#6776: safe ordinal-versus-RELATIVE popup control; never activate.
LOCAL i
=STRTOFILE('VERSION|'+VERSION()+CHR(10),'nonrelative-safe.out',0)
FOR i=1 TO 2
 IF i=1
  DEFINE POPUP numguard
 ELSE
  DEFINE POPUP numguard RELATIVE
 ENDIF
 DEFINE BAR 1 OF numguard PROMPT 'one'
 DEFINE BAR 2 OF numguard PROMPT 'two'
 DEFINE BAR 20 OF numguard PROMPT 'twenty'
 =STRTOFILE(TRANSFORM(i)+'|'+TRANSFORM(CNTBAR('numguard'))+'|'+PRMBAR('numguard',1)+'|'+PRMBAR('numguard',2)+'|'+PRMBAR('numguard',20)+CHR(10),'nonrelative-safe.out',1)
 RELEASE POPUP numguard
ENDFOR
=STRTOFILE('CLEANUP|'+TRANSFORM(SET('DATASESSION'))+CHR(10),'nonrelative-safe.out',1)
RETURN
