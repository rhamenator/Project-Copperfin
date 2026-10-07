$ErrorActionPreference = 'Stop'
# Low-risk read-only installed-VFP9 COM observation; no registry/calendar changes.
$vfp = New-Object -ComObject VisualFoxPro.Application
try {
    $vfp.Visible = $false
    Write-Output ('VERSION|' + $vfp.Eval('VERSION()'))
    Write-Output ('CLOCK|' + $vfp.Eval('YEAR(DATE())'))
    foreach ($expression in @("SET('CENTURY')", "SET('CENTURY',1)", "SET('CENTURY',2)", "SET('CENTURY',3)")) {
        Write-Output ('QUERY|' + $expression + '|' + $vfp.Eval($expression))
    }
    $vfp.DoCmd('SET CENTURY OFF')
    $vfp.DoCmd('SET CENTURY TO 20 ROLLOVER 25')
    Write-Output ('WINDOW|OFF|' + $vfp.Eval("YEAR(CTOD('01/02/24'))") + '|' + $vfp.Eval("YEAR(CTOD('01/02/25'))"))
    $vfp.DoCmd('SET CENTURY ON')
    Write-Output ('WINDOW|ON|' + $vfp.Eval("YEAR(CTOD('01/02/24'))") + '|' + $vfp.Eval("YEAR(CTOD('01/02/25'))"))
    $vfp.DoCmd('SET CENTURY TO')
    Write-Output ('RESET|' + $vfp.Eval("SET('CENTURY',1)") + '|' + $vfp.Eval("SET('CENTURY',2)") + '|' + $vfp.Eval("SET('CENTURY',3)"))
    $originalSession = [int]$vfp.Eval("SET('DATASESSION')")
    $vfp.DoCmd("loCenturyProbe3698 = CREATEOBJECT('Session')")
    $newSession = [int]$vfp.Eval('loCenturyProbe3698.DataSessionID')
    $vfp.DoCmd('SET DATASESSION TO ' + $newSession)
    Write-Output ('SESSION|FRESH|' + $vfp.Eval("SET('CENTURY')") + '|' + $vfp.Eval("SET('CENTURY',1)") + '|' + $vfp.Eval("SET('CENTURY',2)") + '|' + $vfp.Eval("SET('CENTURY',3)"))
    $vfp.DoCmd('SET CENTURY TO 18 ROLLOVER 25')
    Write-Output ('SESSION|CHANGED|' + $vfp.Eval("SET('CENTURY')") + '|' + $vfp.Eval("SET('CENTURY',1)") + '|' + $vfp.Eval("SET('CENTURY',2)"))
    $vfp.DoCmd('SET DATASESSION TO ' + $originalSession)
    Write-Output ('SESSION|RESTORED|' + $vfp.Eval("SET('CENTURY')") + '|' + $vfp.Eval("SET('CENTURY',1)") + '|' + $vfp.Eval("SET('CENTURY',2)"))
    $vfp.DoCmd('loCenturyProbe3698 = .NULL.')
} finally {
    $vfp.Quit()
    [void][Runtime.InteropServices.Marshal]::FinalReleaseComObject($vfp)
}
