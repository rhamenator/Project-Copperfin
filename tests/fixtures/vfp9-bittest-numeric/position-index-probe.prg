* Supplemental independent BITTEST position-index observation.
* Literal one-hot masks disambiguate every index; no other bit function supplies results.
LOCAL lcOut, lcPositions, lcMasks, lcPosition, lcMask, lcCommand, lcResult, lnCase, lnBit, lnError, loError, vResult
lcOut = 'VFP9=' + VERSION() + CHR(10)
lcPositions = '-1.9|-1|-0.9|-0.5|0|0.49|0.5|0.9|1.49|1.5|1.9|2.5|30.9|31|31.9|32|32.1'
lcPositions = lcPositions + '|2147483647|2147483648|-2147483648|-2147483649|4294967295|4294967296|4294967297|4294967327|-4294967295|-4294967296|-4294967297|-4294967265'
lcPositions = lcPositions + '|9007199254740992|1E20|-1E20|1E300|-1E300|EXP(1000)|-EXP(1000)'
lcMasks = '1|2|4|8|16|32|64|128|256|512|1024|2048|4096|8192|16384|32768|65536|131072|262144|524288|1048576|2097152|4194304|8388608|16777216|33554432|67108864|134217728|268435456|536870912|1073741824|-2147483648'
FOR lnCase = 1 TO GETWORDCOUNT(lcPositions, '|')
    lcPosition = GETWORDNUM(lcPositions, lnCase, '|')
    FOR lnBit = 0 TO 31
        lcMask = GETWORDNUM(lcMasks, lnBit + 1, '|')
        lcCommand = 'vResult = BITTEST(' + lcMask + ',' + lcPosition + ')'
        lnError = 0
        lcResult = '<not-returned>'
        TRY
            &lcCommand
            lcResult = IIF(vResult, '.T.', '.F.')
        CATCH TO loError
            lnError = loError.ErrorNo
        ENDTRY
        lcOut = lcOut + 'POSITION-INDEX|' + lcPosition + '|MASKBIT=' + TRANSFORM(lnBit) + '|ERR=' + TRANSFORM(lnError) + '|RESULT=' + lcResult + CHR(10)
    ENDFOR
ENDFOR
lcOut = lcOut + 'DONE' + CHR(10)
=STRTOFILE(lcOut, 'position-index.out')
RETURN
