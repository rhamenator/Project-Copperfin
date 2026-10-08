PUBLIC cLog
LOCAL oSource,oHandler,oError,lcValue,lcExpression,lnFlags,lcSimple,lcRaised
? "VERSION=" + VERSION()
oHandler=CREATEOBJECT('FlagSink')
oSource=CREATEOBJECT('FlagSource')
lcValue="0"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',0)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="1"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',1)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="2"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',2)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="3"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',3)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="4"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',4)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="5"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',5)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="7"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',7)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="-1"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',-1)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="-2"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',-2)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="-3"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',-3)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="-4"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',-4)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="0.49"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',0.49)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="0.5"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',0.5)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="0.9"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',0.9)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="1.49"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',1.49)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="1.5"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',1.5)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="1.9"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',1.9)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="2.9"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',2.9)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="3.9"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',3.9)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="-0.5"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',-0.5)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="-1.5"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',-1.5)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="2147483647"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',2147483647)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="2147483648"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',2147483648)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="-2147483648"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',-2147483648)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="-2147483649"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',-2147483649)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="4294967295"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',4294967295)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="4294967296"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',4294967296)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="4294967297"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',4294967297)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="4294967298"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',4294967298)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="4294967299"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',4294967299)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="-4294967295"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',-4294967295)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="9007199254740991"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',9007199254740991)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="9007199254740992"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',9007199254740992)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="1E20"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',1E20)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="-1E20"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',-1E20)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="1E300"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',1E300)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="-1E300"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',-1E300)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="1E300*1E300"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',1E300*1E300)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="-1E300*1E300"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',-1E300*1E300)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue=".NULL."
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',.NULL.)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue=".T."
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',.T.)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="'1'"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle','1')
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
lcValue="$1.5"
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',$1.5)
    cLog=''
    oSource.Ping()
    lcSimple=cLog
    cLog=''
    =RAISEEVENT(oSource,'Ping')
    lcRaised=cLog
    ? lcValue + "|" + lcSimple + "|" + lcRaised
CATCH TO oError
    ? lcValue + "|ERR" + TRANSFORM(oError.ErrorNo)
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
=BINDEVENT(oSource,'Ping',oHandler,'Handle',1)
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',-1)
    ? "rebind:-1|accepted"
CATCH TO oError
    cLog=''
    oSource.Ping()
    ? "rebind:-1|ERR"+TRANSFORM(oError.ErrorNo)+"|"+cLog
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
=BINDEVENT(oSource,'Ping',oHandler,'Handle',1)
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',2147483648)
    ? "rebind:2147483648|accepted"
CATCH TO oError
    cLog=''
    oSource.Ping()
    ? "rebind:2147483648|ERR"+TRANSFORM(oError.ErrorNo)+"|"+cLog
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
=BINDEVENT(oSource,'Ping',oHandler,'Handle',1)
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',4294967295)
    ? "rebind:4294967295|accepted"
CATCH TO oError
    cLog=''
    oSource.Ping()
    ? "rebind:4294967295|ERR"+TRANSFORM(oError.ErrorNo)+"|"+cLog
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
=BINDEVENT(oSource,'Ping',oHandler,'Handle',1)
TRY
    =BINDEVENT(oSource,'Ping',oHandler,'Handle',.NULL.)
    ? "rebind:.NULL.|accepted"
CATCH TO oError
    cLog=''
    oSource.Ping()
    ? "rebind:.NULL.|ERR"+TRANSFORM(oError.ErrorNo)+"|"+cLog
ENDTRY
=UNBINDEVENTS(oSource)
oSource=CREATEOBJECT('FlagSource')
TRY
    =BINDEVENT(oSource,'Ping','FlagRoutine',1)
    ? "routine|accepted"
CATCH TO oError
    ? "routine|ERR"+TRANSFORM(oError.ErrorNo)
ENDTRY
RETURN
PROCEDURE FlagRoutine
cLog=cLog+'H'
ENDPROC
DEFINE CLASS FlagSource AS Custom
    PROCEDURE Ping
        cLog=cLog+'B'
    ENDPROC
ENDDEFINE
DEFINE CLASS FlagSink AS Custom
    PROCEDURE Handle
        cLog=cLog+'H'
    ENDPROC
ENDDEFINE
